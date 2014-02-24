
#include "mge/GameObject.h"
#include "mge/RenderModel.h"
#include "tpe/tpe.h"

#define GRAV_ACCEL 0.1f

using namespace std;


TimePhysicsEngine *TimePhysicsEngine::tpe;

TimePhysicsEngine::TimePhysicsEngine() {
    //ctor
}

TimePhysicsEngine::~TimePhysicsEngine() {
    //dtor
}



void TimePhysicsEngine::update(uint time) {
    m_uiDeltaTime = 7;//time - m_uiLastUpdated;
    m_uiLastUpdated = time;
}

bool TimePhysicsEngine::applyPhysics(GameObject *obj) {
    AbstractTimePhysicsModel *tmdl = dynamic_cast<AbstractTimePhysicsModel*>(obj->getPhysicsModel());
    if(tmdl == NULL) return false;
    bool hasChanged = tmdl->wasPushed() || tmdl->getPhysicsChanged();
    /*
    if(obj->getFlag(TPE_STATIC)) {
        return false;
    }
    */

    //Apply gravity
    if(obj->getFlag(TPE_FALLING) && !obj->getFlag(TPE_FLOATING) && !obj->getFlag(TPE_STATIC)) {
        tmdl->applyForce(Point(0,-GRAV_ACCEL * tmdl->getMass(),0));
    }

    //Apply general physics updates to object
    tmdl->update(m_uiDeltaTime);
    obj->getRenderModel()->moveBy(tmdl->getLastVelocity());
    tmdl->setWasPushed(false);

    hasChanged = hasChanged || tmdl->getLastVelocity() != Point();
    bool bCanFall = !obj->getFlag(TPE_PASSABLE) && !obj->getFlag(TPE_FLOATING) && !obj->getFlag(TPE_STATIC);
    bool bIsOnSurface = tmdl->getSurface() != NULL;
    bool bHasLeftSurface = hasChanged && (!bIsOnSurface || isNotInArea(tmdl->getCollisionVolume(), tmdl->getSurface()->getCollisionVolume()));
    if(bIsOnSurface && bHasLeftSurface) {
        tmdl->setSurface(NULL);
    }
    if(bCanFall && bHasLeftSurface) {
        obj->setFlag(TPE_FALLING, true);
    }
    return (hasChanged);
}

void
TimePhysicsEngine::applyPhysics(GameObject *obj1, GameObject *obj2) {
    //We need to find the point from the two last velocities and current
    // position that the two objects do not collide and have not passed
    AbstractTimePhysicsModel *tpm1 = dynamic_cast<AbstractTimePhysicsModel*>(obj1->getPhysicsModel()),
                      *tpm2 = dynamic_cast<AbstractTimePhysicsModel*>(obj2->getPhysicsModel());
    if(tpm1 == NULL || tpm2 == NULL || !bxIntersectsEq(tpm1->getCollisionVolume(), tpm2->getCollisionVolume())) {
        return;    //no physics model or no collision
    }

    //Otherwise, iterate through collision models.  Most objects only have one
    for(uint uiMdl1 = 0; uiMdl1 < tpm1->getNumModels(); ++uiMdl1) {
        CollisionModel *mdl1 = tpm1->getCollisionModel(uiMdl1);
        switch(mdl1->getType()) {
        case CM_BOX:
            boxOnUnknownCollision(obj1, obj2, uiMdl1);
            break;
        case CM_Y_HEIGHTMAP:
            hmapOnUnknownCollision(obj1, obj2, uiMdl1);
            break;
        default:
            break;
        }
    }
}

void
TimePhysicsEngine::boxOnUnknownCollision(GameObject *obj1, GameObject *obj2, uint uiMdl1) {
    AbstractTimePhysicsModel *tpm2 = (AbstractTimePhysicsModel*)(obj2->getPhysicsModel());

    //Otherwise, iterate through collision models.  Yes, I know it's n^2
    for(uint uiMdl2 = 0; uiMdl2 < tpm2->getNumModels(); ++uiMdl2) {
        CollisionModel *mdl2 = tpm2->getCollisionModel(uiMdl2);
        switch(mdl2->getType()) {
        case CM_BOX:
            boxOnBoxCollision(obj1, obj2, uiMdl1, uiMdl2);
            break;
        case CM_Y_HEIGHTMAP:
            boxOnHmapCollision(obj1, obj2, uiMdl1, uiMdl2);
            break;
        default:
            break;
        }
    }
}

void
TimePhysicsEngine::hmapOnUnknownCollision(GameObject *objHmap, GameObject *obj2, uint uiMdlHmap) {
    AbstractTimePhysicsModel *tpm2 = (AbstractTimePhysicsModel*)(obj2->getPhysicsModel());

    //Otherwise, iterate through collision models.  Yes, I know it's n^2
    for(uint uiMdl2 = 0; uiMdl2 < tpm2->getNumModels(); ++uiMdl2) {
        CollisionModel *mdl2 = tpm2->getCollisionModel(uiMdl2);
        switch(mdl2->getType()) {
        case CM_BOX:
            boxOnHmapCollision(obj2, objHmap, uiMdl2, uiMdlHmap);
            break;
        case CM_Y_HEIGHTMAP:
            //We don't want to handle this case yet
        default:
            break;
        }
    }
}

void
TimePhysicsEngine::boxOnBoxCollision(GameObject *obj1, GameObject *obj2, uint uiMdl1, uint uiMdl2) {
    AbstractTimePhysicsModel *tpm1 = (AbstractTimePhysicsModel*)(obj1->getPhysicsModel());
    AbstractTimePhysicsModel *tpm2 = (AbstractTimePhysicsModel*)(obj2->getPhysicsModel());
    BoxCollisionModel *bmdl1 = (BoxCollisionModel*)tpm1->getCollisionModel(uiMdl1);
    BoxCollisionModel *bmdl2 = (BoxCollisionModel*)tpm2->getCollisionModel(uiMdl2);
    Box bx1 = bmdl1->getBounds() + tpm1->getPosition(),
        bx2 = bmdl2->getBounds() + tpm2->getPosition();
    if(!bxIntersectsEq(bx1, bx2)) {
        return;    //no physics model or no collision
    }

    //Find the shortest distance we need to move these objects so they don't
    // intersect.  They should be signed such that if you add the value to
    // bx1, it will move obj1 out of range of obj2.
    float fXShift1 = bx2.x - (bx1.x + bx1.w),
          fXShift2 = (bx2.x + bx2.w) - bx1.x,
          fXShift  = fabs(fXShift1) < fabs(fXShift2) ? fXShift1 : fXShift2;
    float fYShift1 = bx2.y - (bx1.y + bx1.h),
          fYShift2 = (bx2.y + bx2.h) - bx1.y,
          fYShift  = fabs(fYShift1) < fabs(fYShift2) ? fYShift1 : fYShift2;
    float fZShift1 = bx2.z - (bx1.z + bx1.l),
          fZShift2 = (bx2.z + bx2.l) - bx1.z,
          fZShift  = fabs(fZShift1) < fabs(fZShift2) ? fZShift1 : fZShift2;
    bool bApplyForce = true;

    //Now determine the smallest magnitude, and calculate the shifts.
    Point ptObj1Shift, ptObj2Shift;
    int iDir1, iDir2;

    bool bNoCollide = obj1->getFlag(TPE_PASSABLE) || obj1->getFlag(TPE_LIQUID) ||
                      obj2->getFlag(TPE_PASSABLE) || obj2->getFlag(TPE_LIQUID);

    if(fabs(fXShift) < fabs(fYShift) && fabs(fXShift) < fabs(fZShift)) {
        //Shift by X
        ptObj1Shift = Point(1,0,0);
        ptObj2Shift = Point(-1,0,0);
        splitShift(obj1, obj2, fXShift, &ptObj1Shift, &ptObj2Shift);
    } else if(fabs(fZShift) < fabs(fXShift) && fabs(fZShift) < fabs(fYShift)) {
        //Shift by Z
        ptObj1Shift = Point(0,0,1);
        ptObj2Shift = Point(0,0,-1);
        splitShift(obj1, obj2, fZShift, &ptObj1Shift, &ptObj2Shift);
    } else {
        //Shift by Y
        ptObj1Shift = Point(0,1,0);
        ptObj2Shift = Point(0,-1,0);
        splitShift(obj1, obj2, fYShift, &ptObj1Shift, &ptObj2Shift);

        if(!bNoCollide) {
            if(bx2.y < bx1.y) {
                obj1->setFlag(TPE_FALLING, false);
                tpm1->clearVerticalVelocity();
                tpm1->setSurface(tpm2);
            } else {
                obj2->setFlag(TPE_FALLING, false);
                tpm2->clearVerticalVelocity();
                tpm2->setSurface(tpm1);
            }
        }
        bApplyForce = false;
    }
    if(obj1->getFlag(TPE_LIQUID) && !obj2->getFlag(TPE_LIQUID) && !obj2->getFlag(TPE_STATIC)) {
        applyBuoyantForce(tpm2, tpm1, bx2, bx1.y + bx1.h, bx1.y);
    } else if(obj2->getFlag(TPE_LIQUID) && !obj1->getFlag(TPE_LIQUID) && !obj1->getFlag(TPE_STATIC)) {
        applyBuoyantForce(tpm1, tpm2, bx1, bx2.y + bx2.h, bx2.y);
    }

    //Only move the objects if they aren't passable.  We needed some of
    // the earlier calculations to handle collision events
    if(!bNoCollide && !(obj1->getFlag(TPE_STATIC) && obj2->getFlag(TPE_STATIC))) {
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

    extractCollisionDirections(tpm2->getPosition() - tpm1->getPosition(),
                               fXShift, fYShift, fZShift,
                               &iDir1, &iDir2);

    //Handle collision events
    HandleCollisionData sData1(obj2, iDir1, ptObj1Shift),
                        sData2(obj1, iDir2, ptObj2Shift);
    tpm1->handleCollisionEvent(&sData1);
    tpm2->handleCollisionEvent(&sData2);
}


void
TimePhysicsEngine::boxOnHmapCollision(GameObject *objBox, GameObject *objHmap, uint uiMdlBox, uint uiMdlHmap) {
    AbstractTimePhysicsModel *tpmBox = (AbstractTimePhysicsModel*)(objBox->getPhysicsModel());
    AbstractTimePhysicsModel *tpmHmap = (AbstractTimePhysicsModel*)(objHmap->getPhysicsModel());
    BoxCollisionModel *bmdl = (BoxCollisionModel*)tpmBox->getCollisionModel(uiMdlBox);
    PixelMapCollisionModel *hmdl = (PixelMapCollisionModel*)tpmHmap->getCollisionModel(uiMdlHmap);
    Box bx1 = bmdl->getBounds() + tpmBox->getPosition(),
        bx2 = hmdl->getBounds() + tpmHmap->getPosition();
    if(!bxIntersectsEq(bx1, bx2)) {
        return;    //no physics model or no collision
    }

    //Now, get the position and the y-shift
    float y = hmdl->getHeightAtPoint(tpmBox->getPosition() - tpmHmap->getPosition()) + tpmHmap->getPosition().y;
    float fYShift = (bx1.y + bx1.h > bx2.y) ? y - bx1.y : 0.f;

    //Get the box shifts for comparison
    float fXShift1 = bx2.x - (bx1.x + bx1.w),
          fXShift2 = (bx2.x + bx2.w) - bx1.x,
          fXShift  = fabs(fXShift1) < fabs(fXShift2) ? fXShift1 : fXShift2;
    float fZShift1 = bx2.z - (bx1.z + bx1.l),
          fZShift2 = (bx2.z + bx2.l) - bx1.z,
          fZShift  = fabs(fZShift1) < fabs(fZShift2) ? fZShift1 : fZShift2;

    Point ptBoxShift, ptHmapShift;
    int iDirBox, iDirHmap;

    if(fabs(fXShift) < fabs(fYShift) && fabs(fXShift) < fabs(fZShift)) {
        //Shift by X
        ptBoxShift = Point(1,0,0);
        ptHmapShift = Point(-1,0,0);
        splitShift(objBox, objHmap, fXShift, &ptBoxShift, &ptHmapShift);
    } else if(fabs(fZShift) < fabs(fXShift) && fabs(fZShift) < fabs(fYShift)) {
        //Shift by Z
        ptBoxShift = Point(0,0,1);
        ptHmapShift = Point(0,0,-1);
        splitShift(objBox, objHmap, fZShift, &ptBoxShift, &ptHmapShift);
    } else {
        //Shift in the Y direction
        ptBoxShift = Point(0.f, fYShift, 0.f);
        ptHmapShift = Point();
        bool bBoxInHmap = ptBoxShift.y > 0.f;
        if(!bBoxInHmap) {
            if(tpmBox->getSurface() == tpmHmap) {
                tpmBox->setSurface(NULL);
                objBox->setFlag(TPE_FALLING, true);
            }
            return;
        }
    }

    bool bCollidable = !(objBox->getFlag(TPE_PASSABLE) || objBox->getFlag(TPE_LIQUID) ||
                        objHmap->getFlag(TPE_PASSABLE) || objHmap->getFlag(TPE_LIQUID));
    bool bBothStatic = objBox->getFlag(TPE_STATIC) && objHmap->getFlag(TPE_STATIC);
    if(bCollidable && !bBothStatic) {
        //Move the objects
        tpmBox->moveBy(ptBoxShift);
        tpmHmap->moveBy(ptHmapShift);
        tpmBox->setWasPushed(!objBox->getFlag(TPE_STATIC));
        tpmHmap->setWasPushed(!objHmap->getFlag(TPE_STATIC));
    }

    bool bHmapIsLiquid = objHmap->getFlag(TPE_LIQUID);
    bool bBoxIsFloatable = !objBox->getFlag(TPE_LIQUID) && !objBox->getFlag(TPE_STATIC);
    bool bBoxHasNotSunk = objBox->getFlag(TPE_FALLING) || ptBoxShift.y > 0.f;
    if(bHmapIsLiquid && bBoxIsFloatable && bBoxHasNotSunk) {
        tpmBox->setSurface(NULL);
        objBox->setFlag(TPE_FALLING, true);
        applyBuoyantForce(tpmBox, tpmHmap, bx1, y, bx2.y);
    } else if(!bHmapIsLiquid) {
        tpmBox->setSurface(tpmHmap);
        objBox->setFlag(TPE_FALLING, false);
    }

    extractCollisionDirections(tpmHmap->getPosition() - tpmBox->getPosition(),
                               fXShift, fYShift, fZShift,
                               &iDirBox, &iDirHmap);

    //Handle collision events
    HandleCollisionData sDataBox(objHmap, iDirBox, ptBoxShift),
                        sDataHmap(objBox, iDirHmap, ptHmapShift);
    tpmBox->handleCollisionEvent(&sDataBox);
    tpmHmap->handleCollisionEvent(&sDataHmap);
}

#define MAX_B_FORCE 1.5f
#define MIN_B_FORCE (2.f - MAX_B_FORCE) //ensures objects always are half-immersed


//Splis the collision shift between the two colliding objects
void
TimePhysicsEngine::splitShift(GameObject *obj1, GameObject *obj2, float fShift, Point *ptShift1, Point *ptShift2) {
    if(obj1->getFlag(TPE_STATIC) && !obj2->getFlag(TPE_STATIC)) {
        (*ptShift1) *= 0.f;
        (*ptShift2) *= fShift;
    } else if(!obj1->getFlag(TPE_STATIC) && obj2->getFlag(TPE_STATIC)) {
        (*ptShift1) *= fShift;
        (*ptShift2) *= 0.f;
    } else {    //Split by ratio
        AbstractTimePhysicsModel *tpm1 = (AbstractTimePhysicsModel*)(obj1->getPhysicsModel());
        AbstractTimePhysicsModel *tpm2 = (AbstractTimePhysicsModel*)(obj2->getPhysicsModel());
        float fMassRatio1 = tpm2->getMass() / (tpm1->getMass() + tpm2->getMass());
        float fMassRatio2 = tpm1->getMass() / (tpm1->getMass() + tpm2->getMass());
        (*ptShift1) *= fShift * fMassRatio1;
        (*ptShift2) *= fShift * fMassRatio2;
    }
}

void
TimePhysicsEngine::applyBuoyantForce(AbstractTimePhysicsModel *tpmObj, AbstractTimePhysicsModel *tpmLiquid, const Box &bxObj, float liquidTop, float liquidBottom) {
    float fGravForce = GRAV_ACCEL * tpmObj->getMass();
    if(tpmLiquid->getDensity() > tpmObj->getDensity()) {    //Float
        //Percent of object underwater
        float percentImmersed = ((bxObj.y + bxObj.h) < liquidBottom) ? (1.f) : (liquidTop - bxObj.y) / bxObj.h;
        float bforce = (MAX_B_FORCE * percentImmersed + MIN_B_FORCE * (1.f - percentImmersed)) * fGravForce;
        tpmObj->applyForce(Point(0.f, bforce, 0.f));
    } else {    //Sink slowly
        //tpmObj->applyForce(Point(0.f, fGravForce / 2.f, 0.f));
    }
}

bool
TimePhysicsEngine::isNotInArea(const Box &bxObj, const Box &bxBounds) {
    return  ((bxObj.x) < (bxBounds.x)) ||
            ((bxObj.x + bxObj.w) > (bxBounds.x + bxBounds.w)) ||
            ((bxObj.z) < (bxBounds.z)) ||
            ((bxObj.z + bxObj.l) > (bxBounds.z + bxBounds.l));
}

#define EQUALITY_WIDTH 0.01f

bool
TimePhysicsEngine::isOnSurface(const Box &bxObj, const Box &bxSurface) {
    return !isNotInArea(bxObj, bxSurface) &&
        bxObj.y >= (bxSurface.y + bxSurface.h - EQUALITY_WIDTH) &&
        bxObj.y <= (bxSurface.y + bxSurface.h + EQUALITY_WIDTH);
}

void
TimePhysicsEngine::extractCollisionDirections(const Point &ptCenterDiff, float fXShift, float fYShift, float fZShift, int *iDir1, int *iDir2) {
    *iDir1 = 0;
    *iDir2 = 0;
    float fXMag = fabs(fXShift);
    float fYMag = fabs(fYShift);
    float fZMag = fabs(fZShift);

    if(fXMag == 0.f) {
        if(ptCenterDiff.x < 0.f) {
            *iDir1 |= BIT_EAST;
            *iDir2 |= BIT_WEST;
        } else {
            *iDir1 |= BIT_WEST;
            *iDir2 |= BIT_EAST;
        }
    }
    if(fYMag == 0.f) {
        if(ptCenterDiff.y < 0.f) {
            *iDir1 |= BIT_UP;
            *iDir2 |= BIT_DOWN;
        } else {
            *iDir1 |= BIT_DOWN;
            *iDir2 |= BIT_UP;
        }
    }
    if(fZMag == 0.f) {
        if(ptCenterDiff.z < 0) {
            *iDir1 |= BIT_SOUTH;
            *iDir2 |= BIT_NORTH;
        } else {
            *iDir1 |= BIT_NORTH;
            *iDir2 |= BIT_SOUTH;
        }
    }

    //If none of the shifts are 0
    if(*iDir1 == 0) {
        if(fXMag < fZMag && fXMag < fYMag) {
            if(fXShift < 0.f) {
                *iDir1 |= BIT_EAST;
                *iDir2 |= BIT_WEST;
            } else {
                *iDir1 |= BIT_WEST;
                *iDir2 |= BIT_EAST;
            }
        } else if(fZMag < fXMag && fZMag < fYMag) {
            if(fZShift < 0) {
                *iDir1 = BIT_SOUTH;
                *iDir2 = BIT_NORTH;
            } else {
                *iDir1 = BIT_NORTH;
                *iDir2 = BIT_SOUTH;
            }
        } else {    // if(fYMag < fXMag && fYMag < fZMag) {
            if(fYShift < 0.f) {
                *iDir1 |= BIT_UP;
                *iDir2 |= BIT_DOWN;
            } else {
                *iDir1 |= BIT_DOWN;
                *iDir2 |= BIT_UP;
            }
        }
    }
}
