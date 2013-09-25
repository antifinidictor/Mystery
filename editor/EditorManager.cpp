/*
 * EditorManager.cpp
 */
#include "EditorManager.h"
#include "EditorObject.h"
#include "EditorHudButton.h"

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
        initMainHud();
        break;
    case ED_STATE_SELECT:
        m_eState = m_eNewState;
        D3RE::get()->clearHud();
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
    case ED_HUD_CREATE_OBJECT:
        m_eNewState = ED_STATE_SELECT;
        break;
    }
}

void
EditorManager::initMainHud() {
    //Create HUD objects
    D3RE *re = D3RE::get();
    uint w = re->getScreenWidth();

    D3HudRenderModel *posText = new D3HudRenderModel("(?,?,?)", Rect(0,0,BUTTON_WIDTH,BUTTON_HEIGHT));
    re->addHudElement(ED_HUD_CURSOR_POS, posText);
    EditorHudButton *createObjectButton = new EditorHudButton(ED_HUD_CREATE_OBJECT, "New...", Point(w - BUTTON_WIDTH,0,0));
    re->addHudElement(ED_HUD_CREATE_OBJECT, createObjectButton);
}
