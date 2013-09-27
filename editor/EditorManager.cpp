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
    case ED_STATE_LOADING_FILE:
        m_eNewState = ED_STATE_NORMAL;
        PWE::get()->cleanArea(ED_AREA_0);
        ObjectFactory::get()->read(m_pEditorCursor->getText());
        PWE::get()->setCurrentArea(ED_AREA_0);
        break;
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
    return false;
}

void
EditorManager::callBack(uint cID, void *data, uint eventId) {
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

