/*
 */

#include "StateButton.h"
#include "mge/Image.h"
using namespace std;

StateButton::StateButton() {
    m_pListener = NULL;
}

StateButton::StateButton(StateButton *btn) {
    //Copy the mapping
    for(map<int,int>::iterator itr = btn->m_mStateMap.begin();
            itr != btn->m_mStateMap.end(); ++itr) {
        mapStates(itr->first, itr->second);
    }

    //Copy the listener reference
    m_pListener = btn->m_pListener;
}

StateButton::~StateButton() {
    m_mStateMap.clear();
}

void StateButton::callBack(uint cID, void *data, EventID id) {
    switch(id) {
    case ON_ACTIVATE:
        map<int,int>::iterator itr = m_mStateMap.find(m_pListener->getState());
        if(itr != m_mStateMap.end()) {
            m_pListener->setState(itr->second, data);
        }
        break;
    }
}
