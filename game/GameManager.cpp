#include "GameManager.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/TimePhysicsEngine.h"
#include "game/spells/ElementalVolume.h"
#include "game/game_defs.h"
#include "game/gui/TextDisplay.h"
#include "game/gui/DraggableHud.h"
#include "game/items/Item.h"

using namespace std;

GameManager *GameManager::m_pInstance;

GameManager::GameManager(uint uiId) {
    m_uiId = uiId;
    m_uiFlags = 0;
    m_skState.push(GM_START);
    m_fFadeTimer = 0.f;
    m_uiNextArea = 0;
    m_crWorld = Color(0xFF,0xFF,0xFF);
    m_crBackground = Color(0x9a,0xd7,0xfb);

    D3RE::get()->setWorldColor(m_crWorld);
    D3RE::get()->setBackgroundColor(m_crBackground);
    D3RE::get()->setColorWeight(DEFAULT_WEIGHT);
    TextDisplay::init();
    m_pHud = new DraggableHud(0);
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
GameManager::update(uint time) {
    TextDisplay::get()->update(time);
    m_pHud->updateItemAnimations(&m_inv);
    switch(m_skState.top()) {
    case GM_START:
        //Any initialization here
        initBasicHud();
        initPlayerHud();

        m_skState.push(GM_NORMAL);
        break;
    case GM_FADE_OUT:
        if(m_fFadeTimer < 1.f) {
            fadeArea();
            m_fFadeTimer += FADE_TIME_STEP;
        } else {
            m_fFadeTimer = 1.f;
            m_skState.pop();
            m_skState.push(GM_FADE_IN);
            PWE::get()->setCurrentArea(m_uiNextArea);
        }
        break;
    case GM_FADE_IN:
        if(m_fFadeTimer > 0.f) {
            fadeArea();
            m_fFadeTimer -= FADE_TIME_STEP;
        } else {
            m_fFadeTimer = 0.f;
            m_skState.pop();
            //PWE::get()->setState(PWE_RUNNING);
        }
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
            m_skState.pop();
        case GM_NORMAL:
            m_skState.push(GM_FADE_OUT);
            break;
        default:
            status = EVENT_DROPPED;
            break;
        }
        //PWE::get()->setState(PWE_PAUSED);
        break;
      }
    default:
        status = EVENT_DROPPED;
        break;
    }
    return status;
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
GameManager::initBasicHud() {
    ContainerRenderModel *panel = D3RE::get()->getHudContainer();

    //ContainerRenderModel *rpanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);

    //ContainerRenderModel *topBar = new ContainerRenderModel(Rect(0.f, 0.f, SCREEN_WIDTH, TEXTURE_TILE_SIZE));
    uint uiHudBackdropImgId = D3RE::get()->getImageId("guiBackdrop");
    Rect rcHudBackdropArea = Rect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT);
    D3HudRenderModel *pBackground = new D3HudRenderModel(uiHudBackdropImgId, rcHudBackdropArea);
    m_pHud->getHudContainer()->add(MGHUD_BACKDROP, pBackground);

    ContainerRenderModel *bottomBar = new ContainerRenderModel(Rect(0.f, SCREEN_HEIGHT - 2 * TEXTURE_TILE_SIZE, SCREEN_WIDTH, TEXTURE_TILE_SIZE * 2));
    panel->add(HUD_TOPBAR, m_pHud->getHudContainer());
    panel->add(HUD_BOTTOMBAR, bottomBar);
}

void
GameManager::cleanBasicHud() {
    D3RE::get()->getHudContainer()->clear();
    delete m_pHud;
}

void
GameManager::initPlayerHud() {
    //const uint hudBackdropId = D3RE::get()->getImageId("hudBackdrop");
    ContainerRenderModel *panel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(HUD_TOPBAR);

    //Backdrop
    //D3HudRenderModel *leftEdge = new D3HudRenderModel(hudBackdropId, Rect(0,0,TEXTURE_TILE_SIZE,TEXTURE_TILE_SIZE));
    //D3HudRenderModel *middle = new D3HudRenderModel(hudBackdropId, Rect(TEXTURE_TILE_SIZE,0,SCREEN_WIDTH - TEXTURE_TILE_SIZE * 2,TEXTURE_TILE_SIZE));
    //D3HudRenderModel *rightEdge = new D3HudRenderModel(hudBackdropId, Rect(SCREEN_WIDTH - TEXTURE_TILE_SIZE,0,TEXTURE_TILE_SIZE,TEXTURE_TILE_SIZE));
    Rect rcHealthPanel = Rect(
        TEXTURE_TILE_SIZE,
        SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        SCREEN_WIDTH - TEXTURE_TILE_SIZE * 2,
        TEXTURE_TILE_SIZE
    );
    Rect rcInventoryPanel = Rect(
        TEXTURE_TILE_SIZE * 8,
        SCREEN_HEIGHT,
        TEXTURE_TILE_SIZE * 10,
        TEXTURE_TILE_SIZE
    );
    ContainerRenderModel *healthPanel = new ContainerRenderModel(rcHealthPanel);
    ContainerRenderModel *inventoryPanel = new ContainerRenderModel(rcInventoryPanel);
    //rightEdge->setFrameH(2);
    //middle->setFrameH(1);
    //middle->setRepsW((SCREEN_WIDTH) / TEXTURE_TILE_SIZE - 2);

    //panel->add(MGHUD_LEFT_EDGE, leftEdge);
    //panel->add(MGHUD_MIDDLE, middle);
    //panel->add(MGHUD_RIGHT_EDGE, rightEdge);
    panel->add(MGHUD_HEALTH_CONTAINER, healthPanel);
    panel->add(MGHUD_INVENTORY_CONTAINER, inventoryPanel);

    //Area name
    Rect rcAreaLabel = Rect(
        TEXTURE_TILE_SIZE * 4.f,
        5.F + SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 4,
        TEXTURE_TILE_SIZE
    );
    D3HudRenderModel *label = new D3HudRenderModel("Area0", rcAreaLabel,1.0f);
    panel->add(MGHUD_AREA_NAME, label);

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
}

void
GameManager::cleanPlayerHud() {
    D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(HUD_TOPBAR)->clear();
}

void
GameManager::addToInventory(Item *item) {
    uint invIndex = m_inv.add(item);
    uint itemId = item->getItemId();

    float x, y;
    uint hudIndex = itemId + MGHUD_ELEMENT_THUMBNAIL_START;
    if(itemId < ITEM_NUM_ELEMENTS) {
        x = (2 + 2 * (invIndex % 2)) * TEXTURE_TILE_SIZE;
        y = (2 + 2 * (invIndex / 2)) * TEXTURE_TILE_SIZE;
    } else if(itemId < ITEM_NUM_SPELLS) {
        x = (10) * TEXTURE_TILE_SIZE;
        y = (2 + 2 * (invIndex % 2)) * TEXTURE_TILE_SIZE;
    } else {
        x = (2 + 2 * (invIndex % 6)) * TEXTURE_TILE_SIZE;
        y = (7 + 2 * (invIndex / 6)) * TEXTURE_TILE_SIZE;
        hudIndex = invIndex + ITEM_NUM_SPELLS + MGHUD_ELEMENT_THUMBNAIL_START;
    }

    Rect rcArea = Rect(x, y, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    D3HudRenderModel *thumbnail = new D3HudRenderModel(D3RE::get()->getImageId("items"), rcArea);
    thumbnail->setFrameH(item->getItemId());

    m_pHud->getHudContainer()->add(hudIndex, thumbnail);
}
