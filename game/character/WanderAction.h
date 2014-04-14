#ifndef WANDERACTION_H
#define WANDERACTION_H

#include "Action.h"
#include "game/game_defs.h"

class WanderAction : public Action
{
public:
    WanderAction(Character *pActor);
    virtual ~WanderAction();

    virtual void update(unsigned int time);
    virtual int callBack(uint cID, void *data, uint uiEventId);

private:
    enum WanderState {
        WANDER_WAITING,
        WANDER_WALKING
    };

    bool isNear(const Point &pos, const Point &dest, float discr = 0.1f);
    void handleCollision(HandleCollisionData *data);

    Point m_ptDestDir;
    uint m_uiTimer;
    WanderState m_eState;
};

#endif // WANDERACTION_H
