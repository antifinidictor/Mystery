/*
 * SimplePhysicsObject
 * Objects that simply obey the laws of physics.  Nothing more.
 */

#ifndef SIMPLE_PHYSICS_OBJECT_H
#define SIMPLE_PHYSICS_OBJECT_H

#include "mge/GameObject.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderModel.h"
#include "ore/OrderedRenderEngine.h"
#include "game/GameDefs.h"

class SimplePhysicsObject : public GameObject {
public:
    SimplePhysicsObject(uint id, Image *img, Box bxVolume) {
        Rect rcDrawArea = Rect(bxVolume.x, bxVolume.y + bxVolume.l - img->h / img->m_iNumFramesH,
                               img->w / img->m_iNumFramesW, img->h / img->m_iNumFramesH);
        m_pRenderModel = new OrderedRenderModel(img, rcDrawArea, bxVolume.z, ORE_LAYER_OBJECTS);
        m_pPhysicsModel = new TimePhysicsModel(bxVolume);
        m_uiID = id;
        m_uiFlags = 0;
    }

    virtual ~SimplePhysicsObject() {
        delete m_pRenderModel;
        delete m_pPhysicsModel;
    }

    //General
    virtual uint getID() { return m_uiID; }
    virtual bool update(uint time)              { return false; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType() { return OBJ_SIMPLE_PHYSICS; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

private:
    uint m_uiID;
    uint m_uiFlags;

    OrderedRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
};

#endif
