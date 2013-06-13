#include "tpe/TimePhysicsEngine.h"
#include "mge/GameObject.h"
#include "mge/RenderModel.h"
#include "tpe/TimePhysicsModel.h"

using namespace std;


TimePhysicsEngine *TimePhysicsEngine::tpe;

TimePhysicsEngine::TimePhysicsEngine() {
    //ctor
}

TimePhysicsEngine::~TimePhysicsEngine() {
    //dtor
}



void TimePhysicsEngine::update(uint time) {
    m_uiDeltaTime = time - m_uiLastUpdated;
    m_uiLastUpdated = time;
}

bool TimePhysicsEngine::applyPhysics(GameObject *obj) {
    AbstractTimePhysicsModel *tmdl = dynamic_cast<AbstractTimePhysicsModel*>(obj->getPhysicsModel());
    if(tmdl == NULL) return false;
    bool hasMoved = tmdl->wasPushed();
    /*
    if(obj->getFlag(TPE_STATIC)) {
        return false;
    }
    */

    //Apply gravity
    if(obj->getFlag(TPE_FALLING) && !obj->getFlag(TPE_FLOATING) && !obj->getFlag(TPE_STATIC)) {
        tmdl->applyForce(Point(0,0,-3));
    }

    //Apply general physics updates to object
    tmdl->update(m_uiDeltaTime);
    obj->getRenderModel()->moveBy(tmdl->getLastVelocity());
    tmdl->setWasPushed(false);

    hasMoved = hasMoved || tmdl->getLastVelocity() != Point();
    //Object automatically falls if it doesn't collide with a surface next turn
    if(!obj->getFlag(TPE_PASSABLE) && hasMoved && !obj->getFlag(TPE_STATIC)) {
        obj->setFlag(TPE_FALLING, true);
    }

    return (hasMoved);
}

void TimePhysicsEngine::applyPhysics(GameObject *obj1, GameObject *obj2) {
    //We need to find the point from the two last velocities and current
    // position that the two objects do not collide and have not passed
    AbstractTimePhysicsModel *tpm1 = dynamic_cast<AbstractTimePhysicsModel*>(obj1->getPhysicsModel()),
                      *tpm2 = dynamic_cast<AbstractTimePhysicsModel*>(obj2->getPhysicsModel());
    if(tpm1 == NULL || tpm2 == NULL || !bxIntersects(tpm1->getCollisionVolume(), tpm2->getCollisionVolume())) {
        return;    //no physics model or no collision
    }

    //Find the shortest distance we need to move these objects so they don't
    // intersect.  They should be signed such that if you add the value to
    // bx1, it will move obj1 out of range of obj2.
    Box bx1 = tpm1->getCollisionVolume(),
        bx2 = tpm2->getCollisionVolume();
    float fXShift1 = bx2.x - (bx1.x + bx1.w),
          fXShift2 = (bx2.x + bx2.w) - bx1.x,
          fXShift  = fabs(fXShift1) < fabs(fXShift2) ? fXShift1 : fXShift2;
    float fYShift1 = bx2.y - (bx1.y + bx1.l),
          fYShift2 = (bx2.y + bx2.l) - bx1.y,
          fYShift  = fabs(fYShift1) < fabs(fYShift2) ? fYShift1 : fYShift2;
    float fZShift1 = bx2.z - (bx1.z + bx1.h),
          fZShift2 = (bx2.z + bx2.h) - bx1.z,
          fZShift  = fabs(fZShift1) < fabs(fZShift2) ? fZShift1 : fZShift2;
    bool bApplyForce = true;

    //Now determine the smallest magnitude, and calculate the shifts.
    Point ptObj1Shift, ptObj2Shift;
    int iDir1, iDir2;
    if(fabs(fXShift) < fabs(fYShift) && fabs(fXShift) < fabs(fZShift)) {
        //Shift by X
        if(obj1->getFlag(TPE_STATIC)) {
            ptObj1Shift = Point();
            ptObj2Shift = Point(-fXShift, 0, 0);
        } else if(obj2->getFlag(TPE_STATIC)) {
            ptObj1Shift = Point(fXShift, 0, 0);
            ptObj2Shift = Point();

        } else {    //Split evenly
            ptObj1Shift = Point(fXShift / 2, 0, 0);
            ptObj2Shift = Point(-fXShift / 2, 0, 0);
        }

        if(fXShift < 0) {
            iDir1 = EAST;
            iDir2 = WEST;
        } else {
            iDir1 = WEST;
            iDir2 = EAST;
        }
    } else if(fabs(fYShift) < fabs(fXShift) && fabs(fYShift) < fabs(fZShift)) {
        //Shift by Y
        if(obj1->getFlag(TPE_STATIC)) {
            ptObj1Shift = Point();
            ptObj2Shift = Point(0, -fYShift, 0);
        } else if(obj2->getFlag(TPE_STATIC)) {
            ptObj1Shift = Point(0, fYShift, 0);
            ptObj2Shift = Point();
        } else {    //Split evenly
            ptObj1Shift = Point(0, fYShift / 2, 0);
            ptObj2Shift = Point(0, -fYShift / 2, 0);
        }

        if(fYShift < 0) {
            iDir1 = NORTH;
            iDir2 = SOUTH;
        } else {
            iDir1 = SOUTH;
            iDir2 = NORTH;
        }
    } else {
        //Shift by Z
        if(obj1->getFlag(TPE_STATIC)) {
            ptObj1Shift = Point();
            ptObj2Shift = Point(0, 0, -fZShift);
        } else if(obj2->getFlag(TPE_STATIC)) {
            ptObj1Shift = Point(0, 0, fZShift);
            ptObj2Shift = Point();

        } else {    //Split evenly
            ptObj1Shift = Point(0, 0, fZShift / 2);
            ptObj2Shift = Point(0, 0, -fZShift / 2);
        }

        if(bx2.z > bx1.z/*fZShift < 0*/) {
            iDir1 = UP;
            iDir2 = DOWN;
            if(!obj1->getFlag(TPE_PASSABLE) && !obj2->getFlag(TPE_PASSABLE)) {
                obj2->setFlag(TPE_FALLING, false);
                tpm2->clearVerticalVelocity();
            }
        } else {
            iDir1 = DOWN;
            iDir2 = UP;
            if(!obj1->getFlag(TPE_PASSABLE) && !obj2->getFlag(TPE_PASSABLE)) {
                obj1->setFlag(TPE_FALLING, false);
                tpm1->clearVerticalVelocity();
            }
        }
        bApplyForce = false;
    }

    //Only move the objects if they aren't passable.  We needed some of
    // the earlier calculations to handle collision events
    if(!obj1->getFlag(TPE_PASSABLE) && !obj2->getFlag(TPE_PASSABLE) && !(obj1->getFlag(TPE_STATIC) && obj2->getFlag(TPE_STATIC))) {
        //Move the objects
        obj1->getRenderModel()->moveBy(ptObj1Shift);
        obj2->getRenderModel()->moveBy(ptObj2Shift);
        tpm1->moveBy(ptObj1Shift);
        tpm2->moveBy(ptObj2Shift);
        tpm1->setWasPushed(!obj1->getFlag(TPE_STATIC));
        tpm2->setWasPushed(!obj2->getFlag(TPE_STATIC));

        //Also apply a force for use on the next cycle
        if(bApplyForce) {
            tpm1->applyForce(ptObj1Shift);
            tpm2->applyForce(ptObj2Shift);
        }
    }

    //Handle collision events
    HandleCollisionData sData1(obj2, iDir1),
                        sData2(obj1, iDir2);
    tpm1->handleCollisionEvent(&sData1);
    tpm2->handleCollisionEvent(&sData2);
}

#if 0
void TimePhysicsEngine::applyPhysics(GameObject *obj1, GameObject *obj2) {
    //We need to find the point from the two last velocities and current
    // position that the two objects do not collide and have not passed
    TimePhysicsModel *tpm1 = dynamic_cast<TimePhysicsModel*>(obj1->getPhysicsModel()),
                      *tpm2 = dynamic_cast<TimePhysicsModel*>(obj2->getPhysicsModel());
    if(tpm1 == NULL || tpm2 == NULL) return;    //no collision for NULL physics

    if(!bxIntersects(tpm1->m_bxVolume, tpm2->m_bxVolume)) return;   //no collision

    Point v1 = tpm1->getLastVelocity(),
          v2 = tpm2->getLastVelocity();
    Rect rc1 = tpm1->getCollisionVolume(),
         rc2 = tpm2->getCollisionVolume();

    //Calculate time of travel.  The correct value will be the smallest
    // nonnegative option
    float time_x1 = (rc1.x - (rc2.x + rc2.w)) / (v1.x - v2.x),
          time_x2 = ((rc1.x + rc1.w) - rc2.x) / (v1.x - v2.x),
          time_x  = (time_x1 < 0) ? time_x2 : time_x1;
    float time_y1 = (rc1.y - (rc2.y + rc2.l)) / (v1.y - v2.y),
          time_y2 = ((rc1.y + rc1.l) - rc2.y) / (v1.y - v2.y),
          time_y  = (time_y1 < 0) ? time_y2 : time_y1;

    //Get the correct velocity.  We only want to move in the collision
    // direction so objects can slide past each other
    Point vb1, vb2;
    if(time_x < time_y) {
        vb1 = Point(v1.x * -time_x, 0, 0);
        vb2 = Point(v2.x * -time_x, 0, 0);
        //tpm1->m_ptVelocity = Point(tpm1->m_ptVelocity.x / 2, tpm1->m_ptVelocity.y, tpm1->m_ptVelocity.z);
        //tpm2->m_ptVelocity = Point(tpm2->m_ptVelocity.x / 2, tpm2->m_ptVelocity.y, tpm2->m_ptVelocity.z);
    } else {
        vb1 = Point(0, v1.y * -time_y, 0);
        vb2 = Point(0, v2.y * -time_y, 0);
        //tpm1->m_ptVelocity = Point(tpm1->m_ptVelocity.x, tpm1->m_ptVelocity.y / 2, tpm1->m_ptVelocity.z);
        //tpm2->m_ptVelocity = Point(tpm2->m_ptVelocity.x, tpm2->m_ptVelocity.y / 2, tpm2->m_ptVelocity.z);
    }

    //Move the objects
    obj1->getRenderModel()->moveBy(vb1);
    obj2->getRenderModel()->moveBy(vb2);
    tpm1->moveBy(vb1);
    tpm2->moveBy(vb2);

    //Handle collision events
    HandleCollisionData sData1(bm2, iDir1),
                        sData2(bm1, iDir2);
    tpm1->handleCollisionEvent(&sData1);
    tpm2->handleCollisionEvent(&sData2);
}
#endif
