/*
 * Elevator
 * Object that raises and lowers
 */

#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "mge/GameObject.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderModel.h"
#include "ore/OrderedRenderEngine.h"
#include "game/GameDefs.h"

#define MAX_HEIGHT 32.0f
#define HALF_HEIGHT 16.0f
#define MIN_HEIGHT MAX_HEIGHT - 32.0f
#define MAX_TIMER 100
#define ELEV_UP 0
#define ELEV_PAUSE_TOP 1
#define ELEV_DOWN 2
#define ELEV_PAUSE_BOTTOM 3

class Elevator : public GameObject {
public:
    Elevator(uint id, Image *img, Box bxVolume) {
        m_uiID = id;
        m_uiFlags = 0;
        state = true;
        timer = 0;
        Rect rcDrawArea = Rect(bxVolume.x, bxVolume.y + bxVolume.l - img->h / img->m_iNumFramesH,
                               img->w / img->m_iNumFramesW, img->h / img->m_iNumFramesH);
        m_pRenderModel = new OrderedRenderModel(img, rcDrawArea, bxVolume.z, ORE_LAYER_OBJECTS);
        m_pPhysicsModel = new TimePhysicsModel(bxVolume);
        this->setFlag(TPE_STATIC, true);
    }

    virtual ~Elevator() {
        delete m_pRenderModel;
        delete m_pPhysicsModel;
    }

    //General
    virtual uint getID() { return m_uiID; }
    virtual bool update(uint time) {
        switch(state) {
        case ELEV_UP:
            if(m_pPhysicsModel->getPosition().z < MAX_HEIGHT + HALF_HEIGHT) {
                m_pPhysicsModel->applyForce(Point(0,0,1));
            } else {
                state = ELEV_PAUSE_TOP;
                timer = MAX_TIMER;
            }
            break;
        case ELEV_PAUSE_TOP:
            if(--timer < 0) {
                state = ELEV_DOWN;
            }
            break;
        case ELEV_DOWN:
            if(m_pPhysicsModel->getPosition().z > MIN_HEIGHT + HALF_HEIGHT) {
                m_pPhysicsModel->applyForce(Point(0,0,-1));
            } else {
                state = ELEV_PAUSE_BOTTOM;
                timer = MAX_TIMER;
            }
            break;
        case ELEV_PAUSE_BOTTOM:
            if(--timer < 0) {
                state = ELEV_UP;
            }
            break;
        }
        return false;
    }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType() { return OBJ_SIMPLE_PHYSICS; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

private:
    uint m_uiID;
    uint m_uiFlags;
    int state, timer;

    OrderedRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
};

#endif
