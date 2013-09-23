/*
 * SimplePhysicsObject
 * Objects that simply obey the laws of physics.  Nothing more.
 */

#ifndef SIMPLE_PHYSICS_OBJECT_H
#define SIMPLE_PHYSICS_OBJECT_H

#include "mge/GameObject.h"
#include "tpe/TimePhysicsModel.h"
#include "d3re/d3re.h"
#include "game/game_defs.h"

class SimplePhysicsObject : public GameObject {
public:
    SimplePhysicsObject(uint id, Image *img, Box bxVolume) {
        m_pRenderModel = new D3PrismRenderModel(this, Box(-bxVolume.w / 2, -bxVolume.l / 2, -bxVolume.h / 2,
                                                           bxVolume.w,      bxVolume.l,      bxVolume.h));
        m_pRenderModel->setTexture(NORTH, img->m_uiID);
        m_pRenderModel->setTexture(SOUTH, img->m_uiID);
        m_pRenderModel->setTexture(EAST,  img->m_uiID);
        m_pRenderModel->setTexture(WEST,  img->m_uiID);
        m_pRenderModel->setTexture(UP,    img->m_uiID);
        m_pRenderModel->setTexture(DOWN,  img->m_uiID);

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
    virtual uint getType() { return TYPE_GENERAL; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

private:
    uint m_uiID;
    uint m_uiFlags;

    D3PrismRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
};

#endif
