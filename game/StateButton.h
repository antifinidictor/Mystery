/*
 * StateButton
 * Defines a type of button listener that implements a state machine
 */

#ifndef STATE_BUTTON_H
#define STATE_BUTTON_H

#include <map>
#include "mge/Event.h"

class StateDevice {
public:
    virtual int getState() = 0;
    virtual void setState(int iState, void *data) = 0;
};

class StateButton : public Listener {
public:
    StateButton();
    StateButton(StateButton *btn);
    virtual ~StateButton();

    //Listener
    virtual void callBack(uint cID, void *data, EventID id);

    //Mapping
    inline void mapStates(int iTriggerState, int iResultState) {
        m_mStateMap.insert(std::pair<int,int>(iTriggerState,iResultState));
    }

    inline void setListener(StateDevice *pListener) { m_pListener = pListener; }

protected:  //Necessary for the sake of the copy constructor
    std::map<int, int>  m_mStateMap;
    StateDevice *m_pListener;
};

#endif
