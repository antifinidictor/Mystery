/*
 * EditorManager.cpp
 */
#include "EditorManager.h"
#include "EditorObject.h"
#include "EditorHudButton.h"
#include "game/world/Wall.h"
#include "pwe/PartitionedWorldEngine.h"

using namespace std;

EditorManager *EditorManager::m_pInstance;

EditorManager::EditorManager(uint uiId) {
    m_uiId = uiId;
    m_uiFlags = 0;
    PWE::get()->setState(PWE_PAUSED);
    m_pEditorObject = NULL;
    m_eState = m_eNewState = ED_STATE_NORMAL;
}

EditorManager::~EditorManager() {
}

void
EditorManager::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
}

bool
EditorManager::update(uint time) {
    switch(m_eNewState) {
    case ED_STATE_NORMAL:
        m_eState = m_eNewState;
        D3RE::get()->clearHud();
        if(m_pEditorObject) {
            m_pEditorObject->prepState(m_eState);
        }
        initMainHud();
        break;
    case ED_STATE_SELECT:
        m_eState = m_eNewState;
        D3RE::get()->clearHud();
        if(m_pEditorObject) {
            m_pEditorObject->prepState(m_eState);
        }
        initCreateObjectHud();
        break;
    case ED_STATE_LOAD_FILE:
        m_eState = m_eNewState;
        D3RE::get()->clearHud();
        if(m_pEditorObject) {
            m_pEditorObject->prepState(m_eState);
        }
        initLoadFileHud();
        break;
    case ED_STATE_SAVE_FILE:
        m_eState = m_eNewState;
        D3RE::get()->clearHud();
        if(m_pEditorObject) {
            m_pEditorObject->prepState(m_eState);
        }
        initSaveFileHud();
        break;
    default:    //do nothing
        break;
    }

    if(m_pEditorObject != NULL) {
        m_pEditorObject->update(time);
    }
    m_eNewState = ED_NUM_STATES;
    return false;
}

void
EditorManager::callBack(uint cID, void *data, uint eventId) {
    switch(eventId) {
    case ED_HUD_NEW:
        m_eNewState = ED_STATE_SELECT;
        break;
    case ED_HUD_CREATE: {
        m_eNewState = ED_STATE_NORMAL;
        Wall *wall = new Wall(PWE::get()->genID(), IMG_WALL_TOP, IMG_WALL_BOTTOM, IMG_WALL_SIDE, m_pEditorObject->getVolume());
        PWE::get()->add(wall);
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
        PWE::get()->writeArea(ED_AREA_0, pt, "all");
        write_info(((string*)data)->c_str(), pt);
        m_eNewState = ED_STATE_NORMAL;
        break;
    }
    case ED_LOAD_FILE:
        m_eNewState = ED_STATE_NORMAL;
        break;

    default:
        break;
    }
}


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
