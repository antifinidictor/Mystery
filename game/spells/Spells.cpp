/*
 * Spells.cpp
 * Defines all spells
 */
#include "Spells.h"
#include "game/gameDefs.h"
#include "tpe/TimePhysicsEngine.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderEngine.h"
#include "ore/OrderedRenderModel.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/FXSprite.h"
#include <stdlib.h>

using namespace std;


#define DIST_FROM_PLAYER 16

/* * * * * * * * * *\
 * Base spell      *
\* * * * * * * * * */
BaseSpell::BaseSpell(uint uiID, OrderedRenderModel *rm, TimePhysicsModel *pm) {
    m_pRenderModel = rm;
    m_pPhysicsModel = pm;
    m_uiID = uiID;
    m_uiFlags = 0;
}

BaseSpell::BaseSpell(uint uiID, Point ptPlayerPos, Point ptClickPos, int iLayer, AreaOfEffect aoe) {
    Box bxArea;
    float angle;
    m_uiFlags = 0;
    m_uiID = uiID;

    switch(aoe) {
    case AOE_DIRECTED:
        angle = atan2(ptClickPos.y - ptPlayerPos.y, ptClickPos.x - ptPlayerPos.x);
        bxArea = Box(ptPlayerPos.x + DIST_FROM_PLAYER * cos(angle) - 8, ptPlayerPos.y + DIST_FROM_PLAYER * sin(angle) - 8, ptPlayerPos.z,
                     16, 16, 16);
        m_pRenderModel = new OrderedRenderModel(ORE::get()->getMappedImage(IMG_SPELLS),bxArea,ptPlayerPos.z,iLayer);
        m_pPhysicsModel = new TimePhysicsModel(bxArea);
        this->setFlag(TPE_PASSABLE, true);
        m_pPhysicsModel->setListener(this);
        break;
    default:
        m_pRenderModel = NULL;
        m_pPhysicsModel = NULL;
        break;
    }
}

BaseSpell::~BaseSpell() {
    if(m_pRenderModel != NULL) {
        delete m_pRenderModel;
    }
    if(m_pPhysicsModel != NULL) {
        delete m_pPhysicsModel;
    }
}


void BaseSpell::setFrameW(uint f) {
    m_pRenderModel->setFrameW(f);
}

void BaseSpell::setFrameH(uint f) {
    m_pRenderModel->setFrameH(f);
}

/* * * * * * * * * *\
 * Level 1 Spells  *
\* * * * * * * * * */
//Air: Lift
AirSpell::AirSpell(uint uiID, Point ptPlayerPos, Point ptClickPos, int power)
    : BaseSpell(uiID, ptPlayerPos, ptClickPos, ORE_LAYER_OBJECTS, AOE_DIRECTED) {
    timer = 100*power;    //Decremented once per cycle
    direction = ptClickPos - ptPlayerPos;
    direction.normalize();
    direction *= 5; //speed
    m_fBaseHeight = 0;
    target = NULL;
    m_pShadow = NULL;
    m_uiFrame = 0;
    m_bWasStatic = false;
    m_fPrevFriction = 0;
}

AirSpell::~AirSpell() {
}

bool AirSpell::update(uint time) {
    if(target == NULL && timer > 0) {
        this->getPhysicsModel()->applyForce(direction);

        //Animate spell
        if((timer & 15) == 0) {
            m_uiFrame = (m_uiFrame + 1) & 3;
            this->setFrameH(m_uiFrame);
        }

        //Reduce timer
        timer--;
    } else if(timer > 0) {
        //Float target
        float fMoveBy = 2 * (sin(timer / 50.f + M_PI / 2) - sin((timer - 1) / 50.f + M_PI / 2));
        m_fBaseHeight -= fMoveBy;
        target->getRenderModel()->moveBy(Point(0, 0, fMoveBy));
        target->getPhysicsModel()->moveBy(Point(0, 0, fMoveBy));

        //Move shadow/spell with target
        Point ptMoveBy = target->getPhysicsModel()->getPosition() - m_pShadow->getPhysicsModel()->getPosition();
        m_pShadow->moveBy(Point(ptMoveBy.x, ptMoveBy.y, 0));
        this->moveBy(ptMoveBy);

        //Create sparkles
        if((timer & 15) == 0) {
            PartitionedWorldEngine *we = PWE::get();
            Rect rcDA = target->getRenderModel()->getDrawArea();
            Point pos = target->getPhysicsModel()->getPosition() + Point(((rand() % rcDA.w) - rcDA.w / 2), ((rand() % rcDA.l) - rcDA.l / 2 - BASE_HEIGHT), 0);
            FXSprite *fx = new FXSprite(we->genID(), pos, FX_SPARKLE, ORE_LAYER_HIGH_FX);
            we->add(fx);
        }

        //Reduce timer
        timer--;
    } else {
        //Timer has run out, drop the target
        if(target != NULL) {
            m_pShadow->kill();
            if(target->getFlag(GAM_SPELLABLE)) {
                target->getRenderModel()->moveBy(Point(0, 0, m_fBaseHeight));
                target->getPhysicsModel()->moveBy(Point(0, 0, m_fBaseHeight));
                //((TimePhysicsModel*)(target->getPhysicsModel()))->m_fFriction = m_fPrevFriction;
                if(m_bWasStatic)
                    target->setFlag(TPE_STATIC, true);
                target->setFlag(TPE_FLOATING, false);

                //Create settling smoke effect
                PartitionedWorldEngine *we = PWE::get();
                Rect rcDA = ((TimePhysicsModel*)target->getPhysicsModel())->getCollisionVolume();
                //Vertical faces
                for(int i = 0; i < rcDA.l / 4; ++i) {
                    Point pos = Point(rcDA.x, rcDA.y + i * 4, 0);
                    FXSprite *fx = new FXSprite(we->genID(), pos, FX_SMOKE, ORE_LAYER_LOW_FX);
                    fx->setDuration(2 * (rand() % 5) + 3);
                    we->add(fx);

                    pos = Point(rcDA.x + rcDA.w, rcDA.y + i * 4, 0);
                    fx = new FXSprite(we->genID(), pos, FX_SMOKE, ORE_LAYER_LOW_FX);
                    fx->setDuration(2 * (rand() % 5) + 3);
                    we->add(fx);
                }
                //Horizontal face
                for(int i = 0; i < rcDA.w / 4; ++i) {
                    Point pos = Point(rcDA.x + i * 4, rcDA.y + rcDA.l, 0);
                    FXSprite *fx = new FXSprite(we->genID(), pos, FX_SMOKE, ORE_LAYER_OBJECTS);
                    fx->setDuration(2 * (rand() % 5) + 3);
                    we->add(fx);
                }
            }
        }
        return true;
    }

    //Stop when the timer is finished
    return false;
}

void AirSpell::callBack(uint cID, void *data, EventID id) {
    switch(id) {
    case ON_COLLISION:
        handleCollision((HandleCollisionData*)data);
        break;
    default:
        break;
    }
}

void AirSpell::handleCollision(HandleCollisionData* data) {
    if(target != NULL) return;

    target = data->obj;
    if(target->getFlag(GAM_SPELLABLE)) {
        //Lift target
        if(target->getFlag(TPE_STATIC)) {
            target->setFlag(TPE_STATIC, false);
            m_bWasStatic = true;
        }
        target->setFlag(TPE_FLOATING, true);
        TimePhysicsModel* mdl = (TimePhysicsModel*)(target->getPhysicsModel());
        //m_fPrevFriction = mdl->m_fFriction;
        //mdl->m_fFriction = 0.99f;

        //Hide spell
        setFlag(ORE_INVISIBLE, true);

        //Create shadow
        PWE *we = PWE::get();
        Box bxCV = target->getPhysicsModel()->getCollisionVolume();
        m_pShadow = new Decorative(we->genID(), ORE::get()->getMappedImage(IMG_SHADOW), bxCV, ORE_LAYER_LOW_FX);
        we->add(m_pShadow);

        //Shift the object up
        m_fBaseHeight = -BASE_HEIGHT;
        target->getRenderModel()->moveBy(Point(0, 0, -m_fBaseHeight));
        target->getPhysicsModel()->moveBy(Point(0, 0, -m_fBaseHeight));
    } else {
        timer = 0;  //die
        target = NULL;
    }
}

GameObject *createAirSpell(Point ptPlayerPos, Point ptClickPos, int power) {
    return new AirSpell(PWE::get()->genID(), ptPlayerPos, ptClickPos, power);
}
