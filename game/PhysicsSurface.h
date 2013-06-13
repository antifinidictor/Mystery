/*
 * PhysicsSurface.h
 * Defines a surface with friction
 */

#ifndef PHYSICS_SURFACE_H
#define PHYSICS_SURFACE_H
#include "mge/GameObject.h"
#include "mge/defs.h"
#include "mge/Image.h"
#include "tpe/TimePhysicsModel.h"
#include "tpe/TimePhysicsEngine.h"
#include "ore/OrderedRenderModel.h"
#include "ore/OrderedRenderEngine.h"
#include "game/GameDefs.h"

class PhysicsSurface : public GameObject {
public:
    //Constructor/Destructor
    PhysicsSurface(uint id, Image *img, Box bxArea) {
        m_uiID = id;
        m_pPhysicsModel = new TimePhysicsModel(bxArea);
        m_pRenderModel = new OrderedRenderModel(img, bxArea, bxArea.z, ORE_LAYER_SURFACE);
        m_pRenderModel->setRepsW(bxArea.w / img->w);
        m_pRenderModel->setRepsH(bxArea.l / img->h);
        m_uiFlags = 0;
        this->setFlag(TPE_STATIC, true);
        printf("Physics surface given id %d\n", id);
    }

    virtual ~PhysicsSurface() {
        delete m_pRenderModel;
        delete m_pPhysicsModel;
    }

    //General
    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType() { return OBJ_SURFACE; }

    virtual bool update(uint time) { return false; }

    //models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

private:
    uint m_uiID;
    uint m_uiFlags;
    OrderedRenderModel  *m_pRenderModel;
    TimePhysicsModel *m_pPhysicsModel;
};

#endif
