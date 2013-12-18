#include "WanderAction.h"
#include "Character.h"
#define WAIT_LENGTH 300
WanderAction::WanderAction(Character *pActor)
  : Action(pActor)
{
    m_ptDest = Point();
    m_uiTimer = rand() % WAIT_LENGTH;
    m_eState = WANDER_WAITING;
}

WanderAction::~WanderAction() {
    //dtor
}

void
WanderAction::update(unsigned int time) {
    switch(m_eState) {
    case WANDER_WALKING:
        if(m_uiTimer > WAIT_LENGTH ) {//|| isNear(m_pActor->getPhysicsModel()->getPosition(), m_ptDest)) {
            m_eState = WANDER_WAITING;
            m_pActor->standStill();
            m_uiTimer = 0;
        } else {
            m_uiTimer++;
            m_pActor->moveTowards(m_ptDest, 0.5f);
        }
        break;
    case WANDER_WAITING:
        if(m_uiTimer > WAIT_LENGTH) {
            m_eState = WANDER_WALKING;
            m_uiTimer = 0;
            int xdir, zdir;
            do {    //Should rarely execute more than once
                xdir = (rand() % 3) - 1;
                zdir = (rand() % 3) - 1;
            } while(xdir == zdir && xdir == 0);
            m_ptDest = m_pActor->getPhysicsModel()->getPosition()
                + Point(xdir * 3.f, 0.f, zdir * 3.f);
        } else {
            m_uiTimer++;
        }
        break;
    }
}

void
WanderAction::callBack(uint cID, void *data, uint uiEventId) {
    switch(uiEventId) {
    case TPE_ON_COLLISION:
        handleCollision((HandleCollisionData*)data);
        break;
    }
}
void
WanderAction::handleCollision(HandleCollisionData *data) {
    if(m_eState == WANDER_WALKING && data->iDirection == m_pActor->getDirection()) {
        //"Bounce" away from collision object
        switch(data->iDirection) {
        case NORTH:
            m_ptDest.z += 6.f;
            break;
        case SOUTH:
            m_ptDest.z -= 6.f;
            break;
        case WEST:
            m_ptDest.x += 6.f;
            break;
        case EAST:
            m_ptDest.x += -6.f;
            break;
        }
    }
}

bool
WanderAction::isNear(const Point &pos, const Point &dest, float discr) {
    return ((pos.x > dest.x - discr) || (pos.x < dest.x + discr)) &&
        //((pos.y > dest.y - discr) || (pos.y < dest.y + discr)) &&
        ((pos.z > dest.z - discr) || (pos.z < dest.z + discr));
}
