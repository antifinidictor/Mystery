#ifndef ACTION_H
#define ACTION_H

#include "Character.h"

class Action : public Listener
{
public:
    Action(Character *pActor) { m_pActor = pActor; }
    virtual ~Action() {}

    virtual void update(unsigned int time) = 0;
    virtual uint getId() { return m_pActor->getId(); }

protected:
    Character *m_pActor;
};

class NoAction : public Action {
public:
    NoAction(Character *pActor) : Action(pActor) {}
    virtual void update(unsigned int time) {}
    virtual int callBack(uint cID, void *data, uint uiEventId) { return EVENT_DROPPED; }
};

#endif // ACTION_H
