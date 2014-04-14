#include "WanderAction.h"
#include "Character.h"
#define WAIT_LENGTH 300
WanderAction::WanderAction(Character *pActor)
  : Action(pActor)
{
    m_ptDestDir = Point();
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
            m_pActor->moveTowards(m_ptDestDir + m_pActor->getPhysicsModel()->getPosition(), 0.5f);
        }
        break;
    case WANDER_WAITING:
        if(m_uiTimer > WAIT_LENGTH) {
            m_eState = WANDER_WALKING;
            m_uiTimer = 0;

            //Use ints to avoid floating point rounding errors
            int xdir, zdir;
            do {    //Should rarely execute more than once
                xdir = (rand() % 3) - 1;
                zdir = (rand() % 3) - 1;
            } while(xdir == zdir && xdir == 0);

            m_ptDestDir.x = xdir;
            m_ptDestDir.z = zdir;

        } else {
            m_uiTimer++;
        }
        break;
    }
}

int
WanderAction::callBack(uint cID, void *data, uint uiEventId) {
    switch(uiEventId) {
    case TPE_ON_COLLISION:
        handleCollision((HandleCollisionData*)data);
        return EVENT_CAUGHT;
    }
    return EVENT_DROPPED;
}
void
WanderAction::handleCollision(HandleCollisionData *data) {
    //You can collide and bounce with a wall in any of the three local cardinal directions
    uint uiMyDirection = m_pActor->getDirection();
    uint uiMyLeft = (uiMyDirection + 1) % NUM_CARDINAL_DIRECTIONS;
    uint uiMyRight = (uiMyDirection + NUM_CARDINAL_DIRECTIONS - 1) % NUM_CARDINAL_DIRECTIONS;
    uint uiMyCollisionDirs = BIT(uiMyDirection) | BIT(uiMyLeft) | BIT(uiMyRight);
    uint uiEquivCollisionDirs = data->iDirection & uiMyCollisionDirs;

    //Did we collide in any of those directions?
    if(m_eState == WANDER_WALKING && (uiEquivCollisionDirs != 0)) {
        //Unbit the direction: Find the first nonzero bit
        int dirBit;
        for(dirBit = 0; dirBit < 32; ++dirBit) {
            if(BIT(dirBit) & uiEquivCollisionDirs) {
                break;
            }
        }
        //"Bounce" away from collision object
        switch(dirBit) {//m_pActor->getDirection()) {
        case SOUTH:
        case NORTH:
            m_ptDestDir.z *= -1.f;
            break;
        case EAST:
        case WEST:
            m_ptDestDir.x *= -1.f;
            break;
        default:
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
