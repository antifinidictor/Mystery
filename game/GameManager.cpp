#include "GameManager.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/TimePhysicsEngine.h"
#include "game/spells/ElementalVolume.h"
#include "game/game_defs.h"
#include "game/gui/TextDisplay.h"
#include "game/gui/GuiButton.h"
#include "game/items/Item.h"
#include "mge/ConfigManager.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

#define FADE_TIME_STEP 0.1f
#define DEFAULT_WEIGHT 0.0f //Used to be 0.5f.  Now let's only change it if the world color changes
#define FADE_WEIGHT 1.f

//Static variables
uint GuiButton::s_uiHudId = 0;

using namespace std;

GameManager *GameManager::m_pInstance;

GameManager::GameManager(uint uiId) {
    m_uiId = uiId;
    m_uiFlags = 0;
    pushState(GM_START);
    m_fFadeTimer = 0.f;
    m_uiNextArea = 0;
    m_crWorld = Color(0xFF,0xFF,0xFF);
    m_crBackground = Color(0x9a,0xd7,0xfb);

    D3RE::get()->setWorldColor(m_crWorld);
    D3RE::get()->setBackgroundColor(m_crBackground);
    D3RE::get()->setColorWeight(DEFAULT_WEIGHT);
    TextDisplay::init();

    //Init draggable item location information
    //TODO: Fix so it is relative to the draggable hud and not so hardcoded?
    //Add the draw-item location

}

GameManager::~GameManager() {
    PWE::get()->freeId(getId());
    TextDisplay::clean();

    cleanPlayerHud();
    cleanBasicHud();
}

GameObject*
GameManager::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint id = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));

    //Put state information here
    return new GameManager(id);
}

void
GameManager::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    pt.put(keyBase + ".id", getId());
    //Read state information here
}

bool
GameManager::update(float fDeltaTime) {
    TextDisplay::get()->update(fDeltaTime);

    m_pPlayerListener->callBack(getId(), &fDeltaTime, ON_UPDATE_HUD);

    switch(m_skState.top()) {
    case GM_START:
        //Any initialization here
        initBasicHud();
        initPlayerHud();

        pushState(GM_NORMAL);
        break;

    case GM_FADE_OUT:
        if(m_fFadeTimer < 1.f) {
            fadeArea();
            m_fFadeTimer += FADE_TIME_STEP;
        } else {
            m_fFadeTimer = 1.f;
            popState();
            //pushState(GM_FADE_IN);
            //PWE::get()->setCurrentArea(m_uiNextArea);
        }
        break;

    case GM_FADE_IN:
        if(m_fFadeTimer > 0.f) {
            fadeArea();
            m_fFadeTimer -= FADE_TIME_STEP;
        } else {
            m_fFadeTimer = 0.f;
            popState();
            //PWE::get()->setState(PWE_RUNNING);
        }
        break;

    case GM_FADE_AREA:
        swapState(GM_FADE_IN);
        PWE::get()->setCurrentArea(m_uiNextArea);
        break;

    case GM_CLEAN_GAME:
        //Prepare to wait for the PWE to finish cleaning the world
        swapState(GM_WAIT_FOR_WORLD_CLEAN);

        //Clean the game
        cleanGame();
        break;
    case GM_NEW_GAME:
        popState();
        break;
    case GM_LOAD_GAME:
        popState();
        break;

    default:
        break;
    }
    return false;
}

void
GameManager::fadeArea() {
    Color black = Color(0x0,0x0,0x0);
    Color world = Color(
        m_crWorld.r * (1 - m_fFadeTimer) + black.r * m_fFadeTimer,
        m_crWorld.g * (1 - m_fFadeTimer) + black.r * m_fFadeTimer,
        m_crWorld.b * (1 - m_fFadeTimer) + black.r * m_fFadeTimer
    );
    Color background = Color(
        m_crBackground.r * (1 - m_fFadeTimer) + black.r * m_fFadeTimer,
        m_crBackground.g * (1 - m_fFadeTimer) + black.r * m_fFadeTimer,
        m_crBackground.b * (1 - m_fFadeTimer) + black.r * m_fFadeTimer
    );
    float fWeight = DEFAULT_WEIGHT * (1 - m_fFadeTimer) + FADE_WEIGHT * m_fFadeTimer;
    D3RE::get()->setWorldColor(world);
    D3RE::get()->setBackgroundColor(background);
    D3RE::get()->setColorWeight(fWeight);
}

int
GameManager::callBack(uint uiId, void *data, uint eventId) {
    int status = EVENT_CAUGHT;
    switch(eventId) {
    case ON_AREA_FADE_IN: {
        m_uiNextArea = *((uint*)data);
        switch(m_skState.top()) {
        case GM_FADE_IN:
            popState();
        case GM_NORMAL:
            pushState(GM_FADE_AREA);
            pushState(GM_FADE_OUT);
            break;
        default:
            status = EVENT_DROPPED;
            break;
        }
        //PWE::get()->setState(PWE_PAUSED);
        break;
      }
    case PWE_ON_WORLD_CLEANED:
        //Read in basic info
        readWorldFile();

        //Read in save-game info
        readSaveFile();

        //Make the world dirty
        m_bWorldIsClean = false;

        //Pop off the WAIT_CLEAN state and push on a FADE_IN state
        if(m_skState.top() == GM_WAIT_FOR_WORLD_CLEAN) {
            swapState(GM_FADE_IN);
        } else {
            printf("ERROR: Invalid state! Expected GM_WAIT_FOR_WORLD_CLEAN but was %d\n", m_skState.top());
        }
        break;
    default:
        status = EVENT_DROPPED;
        break;
    }
    return status;
}


void
GameManager::readWorldFile() {
    using boost::property_tree::ptree;
    ptree pt;
    const string filename = "res/world.info";

    //Read appropriate file format
    uint fileExtIndex = filename.find_last_of(".");
    if(filename.substr(fileExtIndex) == ".info") {
        read_info(filename, pt);
    } else {
        read_xml(filename, pt);
    }

    //Read resources
    D3RE::get()->read(pt, "resources");

    //Read areas
    PWE::get()->read(pt, "areas");
}

void
GameManager::readSaveFile() {
    using boost::property_tree::ptree;
    ptree pt;

    string file = m_fsGameFile.string();

    //Read appropriate file format
    uint fileExtIndex = file.find_last_of(".");
    if(file.substr(fileExtIndex) == ".info") {
        read_info(file, pt);
    } else {
        read_xml(file, pt);
    }

    //Read areas
    PWE::get()->read(pt, "areas");
}

void
GameManager::addActiveVolume(ElementalVolume *ev) {
    m_mActiveVolumes[ev->getId()] = ev;
}

void
GameManager::removeActiveVolume(uint id) {
    m_mActiveVolumes.erase(id);
}

ElementalVolume*
GameManager::getTopVolume() {
    if(m_mActiveVolumes.size() < 1) {
        return NULL;
    }

    map<uint, ElementalVolume*>::iterator iter;
    ElementalVolume *ev = m_mActiveVolumes.begin()->second;
    Box bxVol = ev->getPhysicsModel()->getCollisionVolume();
    float maxY = bxVol.y + bxVol.h;
    for(iter = m_mActiveVolumes.begin(); iter != m_mActiveVolumes.end(); ++iter) {
        bxVol = iter->second->getPhysicsModel()->getCollisionVolume();
        if(bxVol.y + bxVol.h > maxY) {
            maxY = bxVol.y + bxVol.h;
            ev = iter->second;
        }
    }
    return ev;
}

void
GameManager::setDefaultInputMapping() {
    MGE *mge = MGE::get();
    mge->mapInput(SDLK_w,     IN_NORTH);
    mge->mapInput(SDLK_d,     IN_EAST);
    mge->mapInput(SDLK_s,     IN_SOUTH);
    mge->mapInput(SDLK_a,     IN_WEST);
    mge->mapInput(SDLK_q,     IN_ROTATE_LEFT);
    mge->mapInput(SDLK_e,     IN_ROTATE_RIGHT);

    mge->mapInput(SDLK_UP,    IN_NORTH);
    mge->mapInput(SDLK_RIGHT, IN_EAST);
    mge->mapInput(SDLK_DOWN,  IN_SOUTH);
    mge->mapInput(SDLK_LEFT,  IN_WEST);
    mge->mapInput(SDLK_RSHIFT,IN_ROTATE_LEFT);
    mge->mapInput(SDLK_END,   IN_ROTATE_RIGHT);

    mge->mapInput(SDLK_SPACE, IN_CAST);
    mge->mapInput(SDLK_LSHIFT, IN_SHIFT);
    mge->mapInput(SDLK_LCTRL, IN_CTRL);
    mge->mapInput(SDL_BUTTON_LEFT, IN_SELECT);
    mge->mapInput(SDL_BUTTON_RIGHT, IN_RCLICK);
    mge->mapInput(SDLK_h,     IN_BREAK);
    mge->mapInput(SDLK_F1, IN_TOGGLE_DEBUG_MODE);
}

void
GameManager::setTypingInputMapping() {
    MGE *mge = MGE::get();
    mge->mapInput(SDLK_PERIOD, TP_IN_PERIOD);
    mge->mapInput(SDLK_MINUS, TP_IN_UNDERSCORE);
    mge->mapInput(SDLK_SPACE, TP_IN_SPACE);
    mge->mapInput(SDLK_BACKSPACE, TP_IN_BACKSPACE);
    mge->mapInput(SDLK_RETURN, TP_IN_ENTER);
    mge->mapInput(SDLK_SLASH, TP_IN_SLASH);
    mge->mapInput(SDLK_COLON, TP_IN_COLON);
    mge->mapInput(SDLK_LSHIFT, IN_SHIFT);
    mge->mapInput(SDLK_LCTRL, IN_CTRL);
}

void
GameManager::resetInputMapping() {
    //Start with default mapping as a base
    setDefaultInputMapping();

    //Use config mapping
    ConfigManager::get()->setKeyMapping();
}


bool
GameManager::newGame(const std::string &filename) {
    if(!validateSaveFileName(filename, false)) {
        printf("ERROR: Bad save file %s\n", filename.c_str());
        return false;
    }

    //Copy the save-game template to the new game file name
    namespace fs = boost::filesystem;
    if(m_fsGameFile.extension().compare((string)".info") == 0) {
        fs::copy_file(fs::path(SAVE_TEMPLATE_FILE_INFO), m_fsGameFile);
    } else {
        fs::copy_file(fs::path(SAVE_TEMPLATE_FILE_XML), m_fsGameFile);
    }

    pushState(GM_NEW_GAME);
    pushState(GM_CLEAN_GAME);
    pushState(GM_FADE_OUT);
    //cleanGame();

    //Error-free exit
    return true;
}

bool
GameManager::loadGame(const std::string &filename) {
    if(!validateSaveFileName(filename, true)) {
        printf("ERROR: Bad save file %s\n", filename.c_str());
        return false;
    }

    pushState(GM_LOAD_GAME);
    pushState(GM_CLEAN_GAME);
    pushState(GM_FADE_OUT);
    //cleanGame();

    //Error-free exit
    return true;
}


bool
GameManager::saveGame(const std::string &filename) {
    if(!validateSaveFileName(filename, false)) {
        printf("ERROR: Bad save file %s\n", filename.c_str());
        return false;
    }

    //Write to a property tree
    using boost::property_tree::ptree;
    ptree pt;

    PWE::get()->write(pt, "areas", true);   //'true' indicates this is a save file

    //Write the prop tree to the specified file
    string ext = m_fsGameFile.extension().string();
    if(ext.compare(".info") == 0) {
        write_info(m_fsGameFile.string(), pt);
    } else if(ext.compare(".xml") == 0) {
        write_xml(m_fsGameFile.string(), pt);
    }

    //Error-free exit
    return true;
}

void
GameManager::cleanGame() {
    PWE::get()->cleanWorld(this);
    D3RE::get()->setLookAngle(M_PI / 2.f);
    D3RE::get()->moveScreenTo(Point());
    m_bWorldIsClean = true;
}

void
GameManager::initBasicHud() {
    ContainerRenderModel *panel = D3RE::get()->getHudContainer();

    ContainerRenderModel *bottomBar = new ContainerRenderModel(Rect(0.f, SCREEN_HEIGHT - 2 * TEXTURE_TILE_SIZE, SCREEN_WIDTH, TEXTURE_TILE_SIZE * 2));
    panel->add(HUD_BOTTOMBAR, bottomBar);
}

void
GameManager::cleanBasicHud() {
    D3RE::get()->getHudContainer()->clear();
}

void
GameManager::initPlayerHud() {
#if 0
    //const uint hudBackdropId = D3RE::get()->getImageId("hudBackdrop");
    ContainerRenderModel *panel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(HUD_TOPBAR);

    //Backdrop
    //D3HudRenderModel *leftEdge = new D3HudRenderModel(hudBackdropId, Rect(0,0,TEXTURE_TILE_SIZE,TEXTURE_TILE_SIZE));
    //D3HudRenderModel *middle = new D3HudRenderModel(hudBackdropId, Rect(TEXTURE_TILE_SIZE,0,SCREEN_WIDTH - TEXTURE_TILE_SIZE * 2,TEXTURE_TILE_SIZE));
    //D3HudRenderModel *rightEdge = new D3HudRenderModel(hudBackdropId, Rect(SCREEN_WIDTH - TEXTURE_TILE_SIZE,0,TEXTURE_TILE_SIZE,TEXTURE_TILE_SIZE));
    Rect rcHealthPanel = Rect(
        TEXTURE_TILE_SIZE,
        SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 3,
        TEXTURE_TILE_SIZE
    );
    Rect rcInventoryPanel = Rect(
        TEXTURE_TILE_SIZE * 8,
        SCREEN_HEIGHT,
        TEXTURE_TILE_SIZE * 10,
        TEXTURE_TILE_SIZE
    );
    Rect rcItembarPanel = Rect(
        TEXTURE_TILE_SIZE * 16,
        SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 3,
        TEXTURE_TILE_SIZE
    );
    ContainerRenderModel *healthPanel = new ContainerRenderModel(rcHealthPanel);
    ContainerRenderModel *inventoryPanel = new ContainerRenderModel(rcInventoryPanel);
    ContainerRenderModel *itembarPanel = new ContainerRenderModel(rcItembarPanel);
    //rightEdge->setFrameH(2);
    //middle->setFrameH(1);
    //middle->setRepsW((SCREEN_WIDTH) / TEXTURE_TILE_SIZE - 2);

    //panel->add(MGHUD_LEFT_EDGE, leftEdge);
    //panel->add(MGHUD_MIDDLE, middle);
    //panel->add(MGHUD_RIGHT_EDGE, rightEdge);
    panel->add(MGHUD_HEALTH_CONTAINER, healthPanel);
    panel->add(MGHUD_INVENTORY_CONTAINER, inventoryPanel);
    panel->add(MGHUD_ITEMBAR_CONTAINER, itembarPanel);

    //Area name
#define TEXT_BOX_WIDTH (TEXTURE_TILE_SIZE * 9)
    Rect rcAreaLabel = Rect(
        rcHealthPanel.x + rcHealthPanel.w,
        5.F + SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXT_BOX_WIDTH,
        TEXTURE_TILE_SIZE
    );
    Rect rcActionLabel = Rect(
        rcAreaLabel.x + rcAreaLabel.w,
        7.F + SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 3,
        TEXTURE_TILE_SIZE - 12
    );
    D3HudRenderModel *label = new D3HudRenderModel("area", rcAreaLabel,1.0f);
    label->centerHorizontally(true);
    panel->add(MGHUD_CUR_AREA, label);

    label = new D3HudRenderModel("action", rcActionLabel,0.8f);
    label->centerHorizontally(true);
    label->centerVertically(true);
    panel->add(MGHUD_CUR_ACTION, label);

    //Health bar
    #define BAR_SIZE (TEXTURE_TILE_SIZE / 2.F)
    #define BAR_WIDTH  (TEXTURE_TILE_SIZE * 3.F)
    #define BAR_X (0.f)
    #define BAR_Y (BAR_SIZE / 2.F)
    uint barId = D3RE::get()->getImageId("hudbar");
    panel = healthPanel;
    D3HudRenderModel *leftEdge = new D3HudRenderModel(barId, Rect(BAR_X,BAR_Y,BAR_SIZE,BAR_SIZE));
    D3HudRenderModel *middle = new D3HudRenderModel(barId, Rect(BAR_X + BAR_SIZE,BAR_Y,BAR_WIDTH - BAR_SIZE * 2,BAR_SIZE));
    D3HudRenderModel *rightEdge = new D3HudRenderModel(barId, Rect(BAR_X + BAR_WIDTH - BAR_SIZE,BAR_Y,BAR_SIZE,BAR_SIZE));

    label = new D3HudRenderModel("99", Rect(BAR_X+BAR_WIDTH/2.f - 10.f,BAR_Y,20.f,BAR_SIZE),0.8f);
    D3HudRenderModel *bar = new D3HudRenderModel(barId, Rect(BAR_X+1,BAR_Y,BAR_WIDTH-2,BAR_SIZE));

    rightEdge->setFrameH(2);
    middle->setFrameH(1);
    middle->setRepsW((BAR_WIDTH) / BAR_SIZE - 2);
    bar->setFrameH(3);
    bar->setImageColor(Color(0xFF,0x0,0x0));

    panel->add(MGHUD_HEALTH_BACKDROP_LEFT_EDGE, leftEdge);
    panel->add(MGHUD_HEALTH_BACKDROP_MIDDLE, middle);
    panel->add(MGHUD_HEALTH_BACKDROP_RIGHT_EDGE, rightEdge);
    panel->add(MGHUD_HEALTH_BAR, bar);
    panel->add(MGHUD_HEALTH_VALUE, label);

    //Add item elements
    panel = itembarPanel;
    Rect rcElementArea = Rect(0, 0, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    D3HudRenderModel *curElementThumbnail = new D3HudRenderModel(D3RE::get()->getImageId("items"), rcElementArea);

    Rect rcSpellArea = Rect(0, 0, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    D3HudRenderModel *curSpellThumbnail = new D3HudRenderModel(D3RE::get()->getImageId("items"), rcSpellArea);

    Rect rcItemArea = Rect(TEXTURE_TILE_SIZE * 2, 0, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    D3HudRenderModel *curItemThumbnail = new D3HudRenderModel(D3RE::get()->getImageId("items"), rcItemArea);

    panel->add(MGHUD_ELEMENT_ITEMBAR_CUR_ELEMENT, curElementThumbnail);
    panel->add(MGHUD_ELEMENT_ITEMBAR_CUR_SPELL, curSpellThumbnail);
    panel->add(MGHUD_ELEMENT_ITEMBAR_CUR_ITEM, curItemThumbnail);
#endif
}

void
GameManager::cleanPlayerHud() {
    D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(HUD_TOPBAR)->clear();
}

void
GameManager::registerPlayer(Listener *pPlayer) {
    //The GameManager needs to know about the player so it can tell it to update the HUD
    m_pPlayerListener = pPlayer;
}


void
GameManager::pushState(GameManagerState eNewState) {
    if(m_skState.size() > 0) {
        //Clean the old state, if there was one
        cleanCurState();
    }

    //Push the new state
    m_skState.push(eNewState);

    //Init the new state
    initCurState();
}

void
GameManager::popState() {
    if(m_skState.size() > 1) {
        //Clean the current state if there is at least one remaining state
        cleanCurState();

        //Pop the old state
        m_skState.pop();

        //Init the new current state
        initCurState();
    }
}

void
GameManager::swapState(GameManagerState eNewState) {
    if(m_skState.size() > 0) {
        //Clean the old state, if there was one
        cleanCurState();
    }

    //Pop the old state
    m_skState.pop();

    //Push the new state
    m_skState.push(eNewState);

    //Init the new state
    initCurState();
}

void
GameManager::initCurState() {
    switch(m_skState.top()) {
    default:
        break;
    }
}

void
GameManager::cleanCurState() {
    switch(m_skState.top()) {
    case GM_LOAD_GAME:
    case GM_NEW_GAME:
    default:
        break;
    }
}


bool
GameManager::validateSaveFileName(const std::string &filename, bool bMustExist) {
    namespace fs = boost::filesystem;
    fs::path path0(filename);
    fs::path path1(filename);

    if(path0.empty()) {
        return false;
    }

    //Attach appropriate path front/back
    if(!path0.has_parent_path()) {
        path0 = fs::path("res/saves") / path0;
        path1 = fs::path("res/saves") / path1;
    }
    if(!path0.has_extension()) {
        path0 += ".info";
        path1 += ".xml";
    }

    //Verify that the save files do not override important game files
    bool bBad = (path0.compare((string)WORLD_FILE_INFO) == 0) ||
        (path1.compare((string)WORLD_FILE_XML) == 0) ||
        (path0.compare((string)SAVE_TEMPLATE_FILE_INFO) == 0) ||
        (path1.compare((string)SAVE_TEMPLATE_FILE_XML) == 0);
    if(bBad) {
        return false;
    }

    //Always use the file that already exists.
    // This code is a little redundant; if path1 and path0 are both valid,
    // path1 will get selected first but then will be overridden by path0
    bool bCouldUse = false;
    if(exists(path1)) {
        if(!is_directory(path1)) {
            m_fsGameFile = path1;
            bCouldUse = true;
        }
    } else if(!bMustExist) {
        m_fsGameFile = path0;
        bCouldUse = true;
    }

    if(exists(path0)) {
        if(!is_directory(path0)) {
            m_fsGameFile = path0;
            bCouldUse = true;
        }
    } else if(!bMustExist) {
        m_fsGameFile = path0;
        bCouldUse = true;
    }

    return bCouldUse;
}

void
GameManager::getCurGameFileRoot(std::string &result) {
    namespace fs = boost::filesystem;
    fs::path stem = m_fsGameFile.stem();
    if(!stem.empty()) {
        result = stem.string();
    }
}
