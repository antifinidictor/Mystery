/*
 * Clickable.cpp
 * Defines the button class
 */

#include "Clickable.h"
#include "game/gameDefs.h"
#include "ore/OrderedRenderEngine.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/CompositeRenderModel.h"
#include "game/TextRenderModel.h"
#include "game/TextRenderer.h"

using namespace std;

Clickable::Clickable(uint uiID, bool bFreeListener) {
    m_uiID = uiID;
    m_uiFlags = 0;
    m_pListener = NULL;
    m_fpCallBack = NULL;
    m_eState = CLICK_INACTIVE;
    m_bFreeListener = bFreeListener;

    PWE *we = PWE::get();
    we->addListener(this, ON_MOUSE_MOVE);
    we->addListener(this, ON_BUTTON_INPUT);
}

Clickable::~Clickable() {
    //Remove this as listener from the objects reporting to it
    PWE *we = PWE::get();
    we->removeListener(m_uiID, ON_MOUSE_MOVE);
    we->removeListener(m_uiID, ON_BUTTON_INPUT);
    if(m_bFreeListener) {
        delete m_pListener;
    }
}

//Listener
void Clickable::callBack(uint cID, void *data, EventID id) {
    switch(id) {
    case ON_MOUSE_MOVE:
        handleMouseMove((InputData*)data);
        break;
    case ON_BUTTON_INPUT:
        handleInput((InputData*)data);
        break;
    default:
        break;
    }
}

void Clickable::handleInput(InputData* data) {
    //Handle special keys, such as the escape character
    switch(m_eState) {
    case CLICK_SELECTED:
        if(data->getInputState(IN_SELECT)) {
            m_eState = CLICK_PRESSED;
        }
        break;
    case CLICK_PRESSED:
        if(!data->getInputState(IN_SELECT)) {
            m_eState = CLICK_SELECTED;
            informListeners(ON_ACTIVATE);   //Activated briefly before becoming inactive again
        }
        break;
    default:
        break;
    }
}


void Clickable::handleMouseMove(InputData* data) {
    int iMX = data->getInputState(MIN_MOUSE_X),
        iMY = data->getInputState(MIN_MOUSE_Y);
    Point ptMouse = ORE::get()->getRenderOffset() + Point(iMX, iMY, 0);

    switch(m_eState) {
    case CLICK_INACTIVE:  //Clickable may be selected
        if(ptInRect(ptMouse, getResponseArea())) {
            if(data->getInputState(IN_SELECT)) {
                m_eState = CLICK_PRESSED;     //Mouseover while pressed
            } else {
                m_eState = CLICK_SELECTED;    //Mouseover
            }
            informListeners(ON_SELECT);
        }
        break;
    case CLICK_SELECTED:  //Clickable may deactivate
        if(!ptInRect(ptMouse, getResponseArea())) {
            m_eState = CLICK_INACTIVE;    //Mouse leave
            informListeners(ON_DESELECT);
        }
        break;
    case CLICK_PRESSED:   //Clickable may not be pressed
        if(!ptInRect(ptMouse, getResponseArea())) {
            m_eState = CLICK_INACTIVE;    //Mouse leave
            informListeners(ON_DESELECT);
        }
        break;
    }
}


//Event handler
//object listener
void Clickable::addListener(Listener *pListener, EventID id, char* triggerData) {
    m_pListener = pListener;
}

//simple listener (ON_ACTIVATE only)
void Clickable::addListener(void (*fpCallBack)(uint,uint)) {
    m_fpCallBack = fpCallBack;
}

bool Clickable::removeListener(uint uiListenerID, EventID eventID) {
    if(uiListenerID == m_pListener->getID()) {
        m_pListener = NULL;
        return true;
    }

    return false;
}

void Clickable::informListeners(EventID id) {
    if(m_pListener != NULL)
        m_pListener->callBack(getID(), NULL, id);
    if(m_fpCallBack != NULL)
        (*m_fpCallBack)(getID(),id);
}
