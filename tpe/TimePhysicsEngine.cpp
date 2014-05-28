
#include "mge/GameObject.h"
#include "mge/RenderModel.h"
#include "tpe/tpe.h"

#define GRAV_ACCEL 0.1f

using namespace std;


TimePhysicsEngine *TimePhysicsEngine::tpe;

TimePhysicsEngine::TimePhysicsEngine() {
    assert(TPE_NUM_FLAGS <= PHYSICS_FLAGS_END);

    printf("Physics engine initialized\n");
    //ctor
}

TimePhysicsEngine::~TimePhysicsEngine() {
    printf("Physics engine cleaned\n");
    //dtor
}



void TimePhysicsEngine::update(float fDeltaTime) {
    //m_uiDeltaTime = time - m_uiLastUpdated;
    m_uiDeltaTime = 12;
    //m_uiLastUpdated = time;
}

#define PRINT_IF_ID(obj, id) if(obj->getId() == id) { printf(__FILE__" %d (obj %d)\n",__LINE__, id); }

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
    if(/*obj->getFlag(TPE_FALLING) && */!obj->getFlag(TPE_FLOATING) && !obj->getFlag(TPE_STATIC)) {
        tmdl->applyForce(Point(0,-GRAV_ACCEL * tmdl->getMass(),0));
    }

    //Apply general physics updates to object
    tmdl->update(m_uiDeltaTime);
    tmdl->setWasPushed(false);

    hasChanged = hasChanged || tmdl->getLastVelocity() != Point();
    bool bCanFall = !obj->getFlag(TPE_PASSABLE) && !obj->getFlag(TPE_FLOATING) && !obj->getFlag(TPE_STATIC);
    bool bIsOnSurface = tmdl->getSurface() != NULL;
    bool bHasLeftSurface = hasChanged && (!bIsOnSurface || isNotInArea(tmdl->getCollisionVolume(), tmdl->getSurface()->getCollisionVolume()));
    if(bCanFall) {
        //Ensures normal force is applied
        obj->setFlag(TPE_FALLING, true);
    }
    if(bIsOnSurface) {
        GameObject *pObjSurface = tmdl->getSurface()->getParent();
        applyPhysics(tmdl->getParent(), pObjSurface);
        if(bHasLeftSurface) {
            tmdl->setSurface(NULL);
        }
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
        case CM_VORTON:
            vortonOnUnknownCollision(obj1, obj2, uiMdl1);
            break;
        default:
            break;
        }
    }
}


void
TimePhysicsEngine::applyCollisionPhysics(list<GameObject*> &ls1, list<GameObject*> &ls2) {
    for(list<GameObject*>::iterator it1 = ls1.begin(); it1 != ls1.end(); ++it1) {
        for(list<GameObject*>::iterator it2 = ls2.begin(); it2 != ls2.end(); ++it2) {
            if((*it1)->getId() != (*it2)->getId()) {
                applyPhysics(*it1, *it2);
            }
        }
    }
}

void
TimePhysicsEngine::boxOnUnknownCollision(GameObject *objBox, GameObject *obj2, uint uiMdlBox) {
    AbstractTimePhysicsModel *tpm2 = (AbstractTimePhysicsModel*)(obj2->getPhysicsModel());

    //Otherwise, iterate through collision models.  Yes, I know it's n^2
    for(uint uiMdl2 = 0; uiMdl2 < tpm2->getNumModels(); ++uiMdl2) {
        CollisionModel *mdl2 = tpm2->getCollisionModel(uiMdl2);
        switch(mdl2->getType()) {
        case CM_BOX:
            boxOnBoxCollision(objBox, obj2, uiMdlBox, uiMdl2);
            break;
        case CM_Y_HEIGHTMAP:
            boxOnHmapCollision(objBox, obj2, uiMdlBox, uiMdl2);
            break;
        case CM_VORTON:
            vortonOnBoxCollision(obj2, objBox, uiMdl2, uiMdlBox);
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
        case CM_VORTON:
            vortonOnHmapCollision(obj2, objHmap, uiMdl2, uiMdlHmap);
            break;
        case CM_Y_HEIGHTMAP:
            //We don't want to handle this case yet
        default:
            break;
        }
    }
}


void
TimePhysicsEngine::vortonOnUnknownCollision(GameObject *objVorton, GameObject *obj2, uint uiMdlVorton) {
    AbstractTimePhysicsModel *tpm2 = (AbstractTimePhysicsModel*)(obj2->getPhysicsModel());

    //Otherwise, iterate through collision models.  Yes, I know it's n^2
    for(uint uiMdl2 = 0; uiMdl2 < tpm2->getNumModels(); ++uiMdl2) {
        CollisionModel *mdl2 = tpm2->getCollisionModel(uiMdl2);
        switch(mdl2->getType()) {
        case CM_BOX:
            vortonOnBoxCollision(objVorton, obj2, uiMdlVorton, uiMdl2);
            break;
        case CM_Y_HEIGHTMAP:
            vortonOnHmapCollision(objVorton, obj2, uiMdlVorton, uiMdl2);
            break;
        case CM_VORTON:
            vortonOnVortonCollision(objVorton, obj2, uiMdlVorton, uiMdl2);
            break;
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
                tpm1->clearVerticalVelocity();
                bool bApplyNormalForce = !obj1->getFlag(TPE_STATIC) &&
                    !obj1->getFlag(TPE_FLOATING) &&
                    obj1->getFlag(TPE_FALLING);
                if(bApplyNormalForce) {
                    tpm1->setSurface(tpm2);

                    //Apply normal force
                    tpm1->applyForce(Point(0,GRAV_ACCEL * tpm1->getMass(),0));
                    obj1->setFlag(TPE_FALLING, false);  //Apply normal force only once
                }
            } else {
                tpm2->clearVerticalVelocity();
                bool bApplyNormalForce = !obj2->getFlag(TPE_STATIC) &&
                    !obj2->getFlag(TPE_FLOATING) &&
                    obj2->getFlag(TPE_FALLING);
                if(bApplyNormalForce) {
                    tpm2->setSurface(tpm1);

                    //Apply normal force
                    tpm2->applyForce(Point(0,GRAV_ACCEL * tpm2->getMass(),0));
                    obj2->setFlag(TPE_FALLING, false);  //Apply normal force only once
                }
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
    HandleCollisionData sData1(obj2, iDir1, uiMdl2, ptObj1Shift),
                        sData2(obj1, iDir2, uiMdl1, ptObj2Shift);
    tpm1->handleCollisionEvent(&sData1);
    tpm2->handleCollisionEvent(&sData2);
}


void
TimePhysicsEngine::boxOnHmapCollision(GameObject *objBox, GameObject *objHmap, uint uiMdlBox, uint uiMdlHmap) {
    AbstractTimePhysicsModel *tpmBox = (AbstractTimePhysicsModel*)(objBox->getPhysicsModel());
    AbstractTimePhysicsModel *tpmHmap = (AbstractTimePhysicsModel*)(objHmap->getPhysicsModel());
    BoxCollisionModel *bmdl = (BoxCollisionModel*)tpmBox->getCollisionModel(uiMdlBox);
    PixelMapCollisionModel *hmdl = (PixelMapCollisionModel*)tpmHmap->getCollisionModel(uiMdlHmap);
    Box bxBox = bmdl->getBounds() + tpmBox->getPosition(),
        bxHmap = hmdl->getBounds() + tpmHmap->getPosition();
    if(!bxIntersectsEq(bxBox, bxHmap)) {
        return;    //no physics model or no collision
    }

    //Now, get the position and the y-shift
    Point ptQueryPos = tpmBox->getPosition() - tpmHmap->getPosition();
    float y = hmdl->getHeightAtPoint(ptQueryPos) + tpmHmap->getPosition().y;
    float fYShift = (bxBox.y + bxBox.h > bxHmap.y) ? y - bxBox.y : 0.f;

    //Get the surface normal
    Vec3f v3Norm = hmdl->getNormalAtPoint(ptQueryPos);

    //If the box is above the height of the hmap at this point
    if(y < bxBox.y) {
        return;
    }

    //Get the box shifts for comparison
    float fXShift1 = bxHmap.x - (bxBox.x + bxBox.w),
          fXShift2 = (bxHmap.x + bxHmap.w) - bxBox.x,
          fXShift  = fabs(fXShift1) < fabs(fXShift2) ? fXShift1 : fXShift2;
    float fZShift1 = bxHmap.z - (bxBox.z + bxBox.l),
          fZShift2 = (bxHmap.z + bxHmap.l) - bxBox.z,
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
        //Shift in the Y direction: Calculate the actual shift to move the box
        // out along the normal
        //float delta = fYShift / (2 * v3Norm.y);

        //Set up the shifts
        //ptBoxShift = Point(v3Norm.x * delta, v3Norm.y * delta, v3Norm.z * delta);
        ptBoxShift = Point(0.f, fYShift, 0.f);
        ptHmapShift = Point();

        //Somehow this prevents objects from falling through the heightmap
        bool bBoxInHmap = ptBoxShift.y >= 0.f;
        if(!bBoxInHmap) {
            if(tpmBox->getSurface() == tpmHmap) {
                tpmBox->setSurface(NULL);
                //objBox->setFlag(TPE_FALLING, true);
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
        //objBox->setFlag(TPE_FALLING, true);
        applyBuoyantForce(tpmBox, tpmHmap, bxBox, y, bxHmap.y);
    } else if(!bHmapIsLiquid) {
        tpmBox->clearVerticalVelocity();

        bool bApplyNormalForce = !objBox->getFlag(TPE_STATIC) &&
            !objBox->getFlag(TPE_FLOATING) &&
            objBox->getFlag(TPE_FALLING);

        if(bApplyNormalForce) {
            tpmBox->setSurface(tpmHmap);

#if 1
            //Do you slide down the slope?
            Vec3f v3VertNorm = Vec3f(0.f, 1.f, 0.f);
            float fCosAngle = dot(v3Norm, v3VertNorm);
            Vec3f v3NormForce;
            #define MIN_ANGLE 0.9
            //When off by pi/2, value is 0
            //When pointing in the same direction, value is 1
            //So, smaller cos means more slip
            if(fCosAngle > MIN_ANGLE) {
                v3NormForce = v3VertNorm * GRAV_ACCEL * tpmBox->getMass();
            } else {
                float fBias = fCosAngle / MIN_ANGLE;
                v3NormForce = (v3VertNorm * fBias + v3Norm * (1.f - fBias)) * GRAV_ACCEL * tpmBox->getMass();
            }

            //We want to keep gravity vertical accel so the object gets pushed into the hmap
            //but only if we aren't near the edge of the hmap.  This allows us to walk over edges just outside the hmap
            //if(fabs(fXShift) > bxBox.w + 0.1 && fabs(fZShift) > bxBox.l + 0.1) {
                v3NormForce.y = 0.f;
            //}

            //Apply normal force.  This makes stepping over objects easier,
            // but since it at least partially cancels gravity it makes moving downhill jerky
            tpmBox->applyForce(v3NormForce);
            objBox->setFlag(TPE_FALLING, false);    //Apply normal force only once
#endif
        }
    }

    extractCollisionDirections(tpmHmap->getPosition() - tpmBox->getPosition(),
                               fXShift, fYShift, fZShift,
                               &iDirBox, &iDirHmap);

    //Handle collision events
    HandleCollisionData sDataBox(objHmap, iDirBox, uiMdlHmap, ptBoxShift),
                        sDataHmap(objBox, iDirHmap, uiMdlBox, ptHmapShift);
    tpmBox->handleCollisionEvent(&sDataBox);
    tpmHmap->handleCollisionEvent(&sDataHmap);
}

void
TimePhysicsEngine::vortonOnVortonCollision(GameObject *obj1, GameObject *obj2, uint uiMdl1, uint uiMdl2) {
    TimePhysicsModel *tpm1 = (TimePhysicsModel*)obj1->getPhysicsModel();
    TimePhysicsModel *tpm2 = (TimePhysicsModel*)obj2->getPhysicsModel();
    VortonCollisionModel *vcm1 = (VortonCollisionModel*)tpm1->getCollisionModel(uiMdl1);
    VortonCollisionModel *vcm2 = (VortonCollisionModel*)tpm2->getCollisionModel(uiMdl2);

    //TODO: Check fluid ids first before interacting!
    vcm1->exchangeVorticityWith(0.1f, vcm2);
}

void
TimePhysicsEngine::vortonOnBoxCollision(GameObject *objVorton, GameObject *objBox, uint uiMdlVorton, uint uiMdlBox) {
}

void
TimePhysicsEngine::vortonOnHmapCollision(GameObject *objVorton, GameObject *objHmap, uint uiMdlVorton, uint uiMdlHmap) {
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
    return ((bxObj.x + bxObj.w) < (bxBounds.x)) ||
            ((bxObj.x) > (bxBounds.x + bxBounds.w)) ||
            ((bxObj.z + bxObj.l) < (bxBounds.z)) ||
            ((bxObj.z) > (bxBounds.z + bxBounds.l));
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
