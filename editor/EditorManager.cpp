/*
 * EditorManager.cpp
 */
#include "EditorManager.h"
#include "EditorCursor.h"
#include "EditorHudButton.h"
#include "game/world/Wall.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/ObjectFactory.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

using namespace std;

EditorManager *EditorManager::m_pInstance;

EditorManager::EditorManager(uint uiId) {
    m_uiId = uiId;
    m_uiFlags = 0;
    PWE::get()->setState(PWE_PAUSED);
    m_pEditorCursor = NULL;
    m_skState.push(ED_STATE_INIT);
    m_uiCurAreaId = 1;
    m_uiAreaFirst = 0;
    m_uiSwitchedAreaId = 0;
    m_uiObjFirst = 0;
    m_uiHudObjButtonIdStart = 0;
    m_uiHudAreaButtonIdStart = 0;
}

EditorManager::~EditorManager() {
}

void
EditorManager::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
}

bool
EditorManager::update(uint time) {
    //Handle events
    while(!m_qEvents.empty()) {
        switch(m_qEvents.front()) {
        case ED_HUD_OP_FINALIZE: {
            //Do some additional stuff here before popping the state
            switch(m_skState.top()) {
            case ED_STATE_LOAD_FILE:
                ObjectFactory::get()->read(m_pEditorCursor->getText());
                initAreaListPanel(m_uiAreaFirst);
                break;
            case ED_STATE_SAVE_FILE:
                ObjectFactory::get()->write(m_pEditorCursor->getText());
                break;
            case ED_STATE_NAME_AREA:
                PWE::get()->setAreaName(PWE::get()->getCurrentArea(), m_pEditorCursor->getText());
                initAreaListPanel(m_uiAreaFirst);
                m_uiCurAreaId = m_vAreas.size();
                break;
            default:
                break;
            }
          }
            //Continue without breaking
        case ED_HUD_OP_CANCEL:
            popState();
            break;
        case ED_HUD_OP_LOAD_WORLD:
            pushState(ED_STATE_LOAD_FILE);
            break;
        case ED_HUD_OP_SAVE_WORLD:
            pushState(ED_STATE_SAVE_FILE);
            break;
        case ED_HUD_OP_NEW_AREA:
            PWE::get()->generateArea(m_uiCurAreaId);
            PWE::get()->setCurrentArea(m_uiCurAreaId);
            m_uiSwitchedAreaId = m_uiCurAreaId;
            m_pEditorCursor->moveToArea(m_uiCurAreaId);
            m_pEditorCursor->setText(PWE::get()->getAreaName(m_uiCurAreaId));
            m_uiCurAreaId++;
            pushState(ED_STATE_NAME_AREA);
            break;
        case ED_HUD_OP_RENAME_AREA:
            m_pEditorCursor->setText(PWE::get()->getAreaName(PWE::get()->getCurrentArea()));
            pushState(ED_STATE_NAME_AREA);
            break;
        case ED_HUD_OP_UP_AREA:
        case ED_HUD_OP_DOWN_AREA:
        case ED_HUD_OP_GO_TO_AREA:
            initAreaListPanel(m_uiAreaFirst);
            break;
        default:
            break;
        }
        m_qEvents.pop();
    }


    switch(m_skState.top()) {
    case ED_STATE_INIT:
        //Add basic hud containers
        initHud();
        pushState(ED_STATE_MAIN);   //Push on main state

        //Continue into the main state
    case ED_STATE_MAIN:
        break;
    default:
        break;
    }

    if(m_pEditorCursor != NULL) {
        m_pEditorCursor->update(time);
    }

#if 0
    switch(m_eNewState) {
    case ED_STATE_NORMAL:
        prepState();
        initMainHud();
        break;
    case ED_STATE_SELECT:
        prepState();
        initCreateObjectHud();
        break;
    case ED_STATE_LOAD_FILE:
        prepState();
        initLoadFileHud();
        break;
    case ED_STATE_LOADING_FILE: {
        m_eNewState = ED_STATE_NORMAL;
        PWE *we = PWE::get();
        we->cleanArea(ED_AREA_0);
        ObjectFactory::get()->read(m_pEditorCursor->getText());
        list<uint> lsAreas;
        we->getAreas(lsAreas);
        we->setCurrentArea(ED_AREA_0);
        m_uiSwitchedAreaId = ED_AREA_0;
        break;
      }
    case ED_STATE_SAVE_FILE:
        prepState();
        initSaveFileHud();
        break;
    case ED_STATE_LIST_OBJECTS:
        prepState();
        initListObjectHud();
        break;
    default:    //do nothing
        break;
    }

    if(m_pEditorCursor != NULL) {
        m_pEditorCursor->update(time);
    }
    m_eNewState = ED_NUM_STATES;
#endif
    return false;
}

void
EditorManager::callBack(uint cID, void *data, uint eventId) {
    //If the data needs to be handled, handle it here.  Otherwise push an event.
    switch(eventId) {
    case ED_HUD_OP_FINALIZE:
        //Do some additional stuff here before popping the state
        break;
    case ED_HUD_OP_GO_TO_AREA: {
        //Get the actual area in question
        uint uiAreaId = m_uiAreaFirst + cID - m_uiHudAreaButtonIdStart;
        PWE::get()->setCurrentArea(uiAreaId);
        m_uiSwitchedAreaId = uiAreaId;
        m_pEditorCursor->moveToArea(uiAreaId);
        break;
      }
    case ED_HUD_OP_UP_AREA:
        if(m_uiAreaFirst > 0) {
            m_uiAreaFirst--;
        }
        break;
    case ED_HUD_OP_DOWN_AREA:
        if(m_uiAreaFirst < m_vAreas.size() - 1) {
            m_uiAreaFirst++;
        }
        break;
    default:
        break;
    }
    m_qEvents.push(eventId);
#if 0
    switch(eventId) {
    case ED_HUD_NEW:
        m_eNewState = ED_STATE_SELECT;//ED_STATE_LIST_OBJECTS;
        break;
    case ED_HUD_LIST_OBJECTS:
        m_eNewState = ED_STATE_SELECT;
        break;
    case ED_HUD_CREATE: {
        m_eNewState = ED_STATE_NORMAL;
        uint id = PWE::get()->genID();
        ostringstream s;
        s << Wall::getClassName() << id;
        ObjectFactory::get()->initObject(Wall::getClassName(), s.str())
            .setAttribute("id", id)
            .setAttribute("tex.north", IMG_NONE)
            .setAttribute("tex.south", IMG_WALL_SIDE)
            .setAttribute("tex.east", IMG_WALL_SIDE)
            .setAttribute("tex.west", IMG_WALL_SIDE)
            .setAttribute("tex.up", IMG_WALL_TOP)
            .setAttribute("tex.down", IMG_NONE)
            //.setAttribute("cr", Color(0x0, 0x0, 0xFF))
            .setAttribute("vol", m_pEditorCursor->getVolume())
        ;
        PWE::get()->add(ObjectFactory::get()->createFromAttributes());
        //Wall *wall = new Wall(PWE::get()->genID(), IMG_WALL_TOP, IMG_WALL_BOTTOM, IMG_WALL_SIDE, m_pEditorCursor->getVolume());
        //PWE::get()->add(wall);
        break;
    }
    case ED_HUD_CANCEL:
        m_eNewState = ED_STATE_NORMAL;
        break;
    case ED_LOAD:
        m_eNewState = ED_STATE_LOAD_FILE;
        break;
    case ED_SAVE:
        m_eNewState = ED_STATE_SAVE_FILE;
        break;
    case ED_SAVE_FILE: {
        boost::property_tree::ptree pt;
        PWE::get()->write(pt, "areas");
        write_info(m_pEditorCursor->getText(), pt);
        m_eNewState = ED_STATE_NORMAL;
        break;
    }
    case ED_LOAD_FILE:
        m_eNewState = ED_STATE_LOADING_FILE;
        /*
        ObjectFactory::get()->read(((string*)data)->c_str());
        PWE::get()->setCurrentArea(ED_AREA_0);
        */
        break;

    default:
        break;
    }
#endif
}


void
EditorManager::pushState(EditorState eState) {
    cleanState(m_skState.top());
    m_skState.push(eState);
    initState(m_skState.top());
}

void
EditorManager::popState() {
    cleanState(m_skState.top());
    m_skState.pop();
    initState(m_skState.top());
}

void
EditorManager::cleanState(EditorState eState) {
    switch(eState) {
    case ED_STATE_NAME_AREA:
    case ED_STATE_SAVE_FILE:
    case ED_STATE_LOAD_FILE:
        D3RE::get()->getHudContainer()
            ->get<ContainerRenderModel*>(ED_HUD_MIDDLE_PANE)
            ->clear();
        //Also clear the right pane
    case ED_STATE_MAIN:
        D3RE::get()->getHudContainer()
            ->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE)
            ->clear();
        break;
    default:
        break;
    }
}

void
EditorManager::initState(EditorState eState) {
    switch(eState) {
    case ED_STATE_MAIN:
        //Restore main hud
        initMainHud();
        break;
    case ED_STATE_SAVE_FILE:
        initSaveHud();
        break;
    case ED_STATE_LOAD_FILE:
        initLoadHud();
        break;
    case ED_STATE_NAME_AREA:
        initRenameAreaHud();
        break;
    default:
        break;
    }
}

void
EditorManager::initHud() {
    ContainerRenderModel *leftPane = new ContainerRenderModel(Rect(0, BUTTON_HEIGHT, BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT)),
                       *middlePane = new ContainerRenderModel(Rect(BUTTON_WIDTH, 0, SCREEN_WIDTH - BUTTON_WIDTH * 2, SCREEN_HEIGHT)),
                        *rightPane = new ContainerRenderModel(Rect(SCREEN_WIDTH - BUTTON_WIDTH, 0, BUTTON_WIDTH, SCREEN_HEIGHT))
    ;
    ContainerRenderModel *hud = D3RE::get()->getHudContainer();
    hud->add(ED_HUD_LEFT_PANE,   leftPane);
    hud->add(ED_HUD_MIDDLE_PANE, middlePane);
    hud->add(ED_HUD_RIGHT_PANE,  rightPane);

    initAreaPanel();
}


void
EditorManager::initMainHud() {
    ContainerRenderModel *rpanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);

    int i = 0;
    EditorHudButton *loadWorld = new EditorHudButton(rpanel, ED_HUD_OP_LOAD_WORLD, "Load World", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *saveWorld  = new EditorHudButton(rpanel, ED_HUD_OP_SAVE_WORLD, "Save World", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *newObj     = new EditorHudButton(rpanel, ED_HUD_OP_NEW_OBJ, "New Object", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *newTex     = new EditorHudButton(rpanel, ED_HUD_OP_NEW_TEXTURE, "New Texture", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);

    rpanel->add(ED_HUD_MAIN_LOAD_WORLD, loadWorld);
    rpanel->add(ED_HUD_MAIN_SAVE_WORLD, saveWorld);
    rpanel->add(ED_HUD_MAIN_NEW_OBJ, newObj);
    rpanel->add(ED_HUD_MAIN_NEW_TEXTURE, newTex);

    if(m_pEditorCursor) {
        m_pEditorCursor->setState(EDC_STATE_MOVE);
    }
}

void
EditorManager::initLoadHud() {
    ContainerRenderModel *rpanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);
    ContainerRenderModel *mpanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_MIDDLE_PANE);

    int i = 0;
    EditorHudButton *cancel = new EditorHudButton(rpanel, ED_HUD_OP_CANCEL, "Cancel", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *finalize  = new EditorHudButton(rpanel, ED_HUD_OP_FINALIZE, "Load it!", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    D3HudRenderModel *label = new D3HudRenderModel("Enter filename:", Rect(0,0,SCREEN_WIDTH - BUTTON_WIDTH*2,BUTTON_HEIGHT));
    D3HudRenderModel *text = new D3HudRenderModel("", Rect(0,BUTTON_HEIGHT,SCREEN_WIDTH - BUTTON_WIDTH*2,SCREEN_HEIGHT));

    rpanel->add(ED_HUD_LOAD_CANCEL, cancel);
    rpanel->add(ED_HUD_LOAD_LOAD, finalize);
    mpanel->add(ED_HUD_FIELD_LABEL, label);
    mpanel->add(ED_HUD_FIELD_TEXT, text);

    if(m_pEditorCursor) {
        m_pEditorCursor->setState(EDC_STATE_TYPE_FIELD);
    }
}

void
EditorManager::initSaveHud() {
    ContainerRenderModel *rpanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);
    ContainerRenderModel *mpanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_MIDDLE_PANE);

    int i = 0;
    EditorHudButton *cancel = new EditorHudButton(rpanel, ED_HUD_OP_CANCEL, "Cancel", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *finalize  = new EditorHudButton(rpanel, ED_HUD_OP_FINALIZE, "Save it!", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    D3HudRenderModel *label = new D3HudRenderModel("Enter filename:", Rect(0,0,SCREEN_WIDTH - BUTTON_WIDTH*2,BUTTON_HEIGHT));
    D3HudRenderModel *text = new D3HudRenderModel("", Rect(0,BUTTON_HEIGHT,SCREEN_WIDTH - BUTTON_WIDTH*2,SCREEN_HEIGHT));

    rpanel->add(ED_HUD_SAVE_CANCEL, cancel);
    rpanel->add(ED_HUD_SAVE_SAVE, finalize);
    mpanel->add(ED_HUD_FIELD_LABEL, label);
    mpanel->add(ED_HUD_FIELD_TEXT, text);

    if(m_pEditorCursor) {
        m_pEditorCursor->setState(EDC_STATE_TYPE_FIELD);
    }
}


void
EditorManager::initRenameAreaHud() {
    ContainerRenderModel *rpanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);
    ContainerRenderModel *mpanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_MIDDLE_PANE);

    int i = 0;
    EditorHudButton *cancel = new EditorHudButton(rpanel, ED_HUD_OP_CANCEL, "Cancel", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *finalize  = new EditorHudButton(rpanel, ED_HUD_OP_FINALIZE, "Set name!", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    D3HudRenderModel *label = new D3HudRenderModel("Enter area name:", Rect(0,0,SCREEN_WIDTH - BUTTON_WIDTH*2,BUTTON_HEIGHT));
    D3HudRenderModel *text = new D3HudRenderModel("", Rect(0,BUTTON_HEIGHT,SCREEN_WIDTH - BUTTON_WIDTH*2,SCREEN_HEIGHT));

    rpanel->add(ED_HUD_SAVE_CANCEL, cancel);
    rpanel->add(ED_HUD_SAVE_SAVE, finalize);
    mpanel->add(ED_HUD_FIELD_LABEL, label);
    mpanel->add(ED_HUD_FIELD_TEXT, text);

    if(m_pEditorCursor) {
        m_pEditorCursor->setState(EDC_STATE_TYPE_FIELD);
    }
}

void
EditorManager::initAreaPanel() {
    ContainerRenderModel *lpanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_LEFT_PANE);

    int i = 0;
    EditorHudButton *newArea    = new EditorHudButton(lpanel, ED_HUD_OP_NEW_AREA, "New area", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    EditorHudButton *renameArea = new EditorHudButton(lpanel, ED_HUD_OP_RENAME_AREA, "Rename area", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    EditorHudButton *upArea     = new EditorHudButton(lpanel, ED_HUD_OP_UP_AREA, "UP", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    EditorHudButton *downArea   = new EditorHudButton(lpanel, ED_HUD_OP_DOWN_AREA, "DOWN", Point(0.f ,SCREEN_HEIGHT - BUTTON_HEIGHT * 2, 0.f), BUTTON_TEXT_SIZE);
    ContainerRenderModel *lspanel = new ContainerRenderModel(Rect(0.f, BUTTON_HEIGHT * i, BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT * (i + 1)), Point(0.f, BUTTON_HEIGHT, 0.f));

    lpanel->add(ED_HUD_AREA_NEW, newArea);
    lpanel->add(ED_HUD_AREA_RENAME, renameArea);
    lpanel->add(ED_HUD_AREA_UP, upArea);
    lpanel->add(ED_HUD_AREA_DOWN, downArea);
    lpanel->add(ED_HUD_AREA_LIST_PANE, lspanel);

    initAreaListPanel(0);
}

void
EditorManager::initAreaListPanel(uint uiAreaFirst) {
    ContainerRenderModel *lspanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_LEFT_PANE)->get<ContainerRenderModel*>(ED_HUD_AREA_LIST_PANE);
    lspanel->clear();

    m_vAreas.clear();
    PWE::get()->getAreas(m_vAreas);

    uint i = 0;
    for(vector<uint>::iterator iter = m_vAreas.begin() + uiAreaFirst; iter < m_vAreas.end(); ++iter) {
        string name;
        if((*iter) == m_uiSwitchedAreaId) {
            name = "#00FF00#" + PWE::get()->getAreaName(*iter);
        } else {
            name = PWE::get()->getAreaName(*iter);
        }
        EditorHudButton *area = new EditorHudButton(lspanel, ED_HUD_OP_GO_TO_AREA, name, Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
        lspanel->add(i, area);
        if(i == 0) {
            m_uiHudAreaButtonIdStart = area->getID();
        }
        ++i;
    }
}


void
EditorManager::initClassListPanel(uint uiStart) {
    ContainerRenderModel *lspanel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE)->get<ContainerRenderModel*>(ED_HUD_NEW_OBJ_LIST_PANE);

    //Get available classes
    vector<const string *> vClasses;
    vector<const string *>::iterator iter;

    ObjectFactory::get()->getClassList(vClasses);
    
    //Display available classes
    uint i = 0;
    for(iter = vClasses.begin() + uiStart; iter != vClasses.end(); ++iter) {
        EditorHudButton *classButton = new EditorHudButton(lspanel, ED_HUD_OP_NEW_OBJ, **iter, Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
        lspanel->add(i, classButton);
        if(i == 0) {
            m_uiHudObjButtonIdStart = area->getID();
        }
        i++;
        printf("Found class: %s\n", (*iter)->c_str());
    }
}

#if 0

void
EditorManager::initConstHud() {
    //These hud elements are always there.
    D3HudRenderModel *posText = new D3HudRenderModel("(?,?,?)", Rect(0,0,BUTTON_WIDTH,BUTTON_HEIGHT));
    D3RE::get()->addHudElement(ED_HUD_CURSOR_POS, posText);
}
void
EditorManager::initMainHud() {
    initConstHud();

    //Create HUD objects
    D3RE *re = D3RE::get();
    uint w = re->getScreenWidth();

    EditorHudButton *btn;
    btn = new EditorHudButton(ED_HUD_NEW, "#000000#New...", Point(w - BUTTON_WIDTH,0,0));
    re->addHudElement(ED_HUD_NEW, btn);

    btn = new EditorHudButton(ED_LOAD, "#000000#Load file", Point(w - BUTTON_WIDTH,BUTTON_HEIGHT,0));
    re->addHudElement(ED_LOAD, btn);

    btn = new EditorHudButton(ED_SAVE, "#000000#Save file", Point(w - BUTTON_WIDTH,BUTTON_HEIGHT*2,0));
    re->addHudElement(ED_SAVE, btn);
}


void
EditorManager::initCreateObjectHud() {
    initConstHud();


    //Create HUD objects
    D3RE *re = D3RE::get();
    uint w = re->getScreenWidth();
    EditorHudButton *btn;
    btn = new EditorHudButton(ED_HUD_CREATE, "#000000#Create it!", Point(w - BUTTON_WIDTH,0,0));
    re->addHudElement(ED_HUD_CREATE, btn);

    btn = new EditorHudButton(ED_HUD_CANCEL, "#000000#Cancel", Point(w - BUTTON_WIDTH,BUTTON_HEIGHT,0));
    re->addHudElement(ED_HUD_CANCEL, btn);
}


void
EditorManager::initLoadFileHud() {
    initConstHud();

    //Create HUD objects
    D3RE *re = D3RE::get();
    uint w = re->getScreenWidth();
    EditorHudButton *btn;
    btn = new EditorHudButton(ED_LOAD_FILE, "#000000#Load", Point(w - BUTTON_WIDTH,0,0));
    re->addHudElement(ED_LOAD_FILE, btn);

    btn = new EditorHudButton(ED_HUD_CANCEL, "#000000#Cancel", Point(w - BUTTON_WIDTH,BUTTON_HEIGHT,0));
    re->addHudElement(ED_HUD_CANCEL, btn);

    D3HudRenderModel *msg = new D3HudRenderModel("Enter filename:", Rect(0,SCREEN_HEIGHT / 2 - BUTTON_HEIGHT,BUTTON_WIDTH,BUTTON_HEIGHT));
    D3RE::get()->addHudElement(ED_MESSAGE, msg);
    D3HudRenderModel *label = new D3HudRenderModel(".info", Rect(0,SCREEN_HEIGHT / 2,BUTTON_WIDTH,BUTTON_HEIGHT));
    D3RE::get()->addHudElement(ED_TEXT, label);
}

void
EditorManager::initSaveFileHud() {
    initConstHud();

    //Create HUD objects
    D3RE *re = D3RE::get();
    uint w = re->getScreenWidth();
    EditorHudButton *btn;
    btn = new EditorHudButton(ED_SAVE_FILE, "#000000#Save", Point(w - BUTTON_WIDTH,0,0));
    re->addHudElement(ED_SAVE_FILE, btn);

    btn = new EditorHudButton(ED_HUD_CANCEL, "#000000#Cancel", Point(w - BUTTON_WIDTH,BUTTON_HEIGHT,0));
    re->addHudElement(ED_HUD_CANCEL, btn);

    D3HudRenderModel *msg = new D3HudRenderModel("Enter filename:", Rect(0,SCREEN_HEIGHT / 2 - BUTTON_HEIGHT,BUTTON_WIDTH,BUTTON_HEIGHT));
    D3RE::get()->addHudElement(ED_MESSAGE, msg);
    D3HudRenderModel *label = new D3HudRenderModel(".info", Rect(0,SCREEN_HEIGHT / 2,BUTTON_WIDTH,BUTTON_HEIGHT));
    D3RE::get()->addHudElement(ED_TEXT, label);
}

void
EditorManager::initListObjectHud() {
    initConstHud();

    //Create HUD objects
    D3RE *re = D3RE::get();
    uint w = re->getScreenWidth();
    EditorHudButton *btn;
    list<const string *> lsClasses;
    list<const string *>::iterator iter;

    ObjectFactory::get()->getClassList(lsClasses);
    uint i = 1;
    for(iter = lsClasses.begin(); iter != lsClasses.end(); ++iter) {
        btn = new EditorHudButton(ED_HUD_LIST_OBJECTS + i, **iter, Point(w - BUTTON_WIDTH, i * BUTTON_HEIGHT,0));
        re->addHudElement(ED_HUD_LIST_OBJECTS + i, btn);
        i++;
        printf("Found class: %s\n", (*iter)->c_str());
    }

    btn = new EditorHudButton(ED_HUD_CANCEL, "#000000#Cancel", Point(w - BUTTON_WIDTH,0,0));
    re->addHudElement(ED_HUD_CANCEL, btn);

}

void
EditorManager::prepState() {
    m_eState = m_eNewState;
    D3RE::get()->clearHud();
    if(m_pEditorCursor) {
        m_pEditorCursor->prepState(m_eState);
    }
}
#endif

