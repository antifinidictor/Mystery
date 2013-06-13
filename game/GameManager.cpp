/*
 * GameManager.cpp
 */

#include "GameManager.h"
#include "mge/defs.h"
#include "mge/ModularEngine.h"
#include "ore/OrderedRenderEngine.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/gameDefs.h"


GameManager *GameManager::gm;


GameManager::GameManager() {
    //Add this object as a listener for button inputs
    ModularEngine::get()->addListener(this, ON_BUTTON_INPUT);
    
    //Create the areas needed for each screen
    PartitionedWorldEngine *we = PWE::get();
    uint uiAreaID;
    for(int pn = 0; pn < GM_NUM_SCREENS; ++pn) {
        uiAreaID = we->generateArea();
        m_mPageMap[pn] = uiAreaID;
    }
}

GameManager::~GameManager() {
    m_mPageMap.clear();
}

void GameManager::callBack(uint cID, void *data, EventID id) {
    switch(id) {
    case ON_BUTTON_INPUT:
        handleButton((InputData*)data);
        break;
    default:
        break;
    }
}

void GameManager::handleButton(InputData* data) {
    //Handle special keys, such as the escape character
    if(!data->getInputState(IN_BREAK) && data->hasChanged(IN_BREAK)) {
        switch(m_eCurScreen) {
        case GM_START_PAGE:
            ModularEngine::get()->stop();   //Kill the game
            break;
        case GM_PAUSE_SCREEN:
            setScreen(GM_START_PAGE);
            break;
        default:
            setScreen(GM_PAUSE_SCREEN);
            break;
        }
    }
}

void GameManager::setScreen(GMPageName pn) {
    m_eCurScreen = pn;
    uint uiScreenID = m_mPageMap.find(pn)->second;
    PWE::get()->setCurrentArea(uiScreenID);
}

