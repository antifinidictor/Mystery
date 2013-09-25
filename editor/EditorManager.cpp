/*
 * EditorManager.cpp
 */
#include "EditorManager.h"
#include "EditorObject.h"

EditorManager *EditorManager::m_pInstance;

EditorManager::EditorManager(uint uiId) {
    m_uiId = uiId;
    m_uiFlags = 0;
    PWE::get()->setState(PWE_PAUSED);
    m_pEditorObject = NULL;
    m_eState = ED_STATE_NORMAL;
}

EditorManager::~EditorManager() {
}

void
EditorManager::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
}

bool
EditorManager::update(uint time) {
    if(m_pEditorObject != NULL) {
        m_pEditorObject->update(time);
    }
    return false;
}

void
EditorManager::callBack(uint cID, void *data, uint id) {
}

