/*
 * Decorative.h
 * Game objects whose only purpose is to be rendered.
 */

#ifndef DECORATIVE_H
#define DECORATIVE_H
#include "mge/defs.h"
#include "mge/GameObject.h"
#include "ore/OrderedRenderModel.h"
#include "ore/OrderedRenderEngine.h"
#include "tpe/TimePhysicsEngine.h"
#include "game/GameDefs.h"

class Decorative : public GameObject {
public:
    Decorative(uint id, Image *img, Point pos, int iLayer) {
        int iw = img->w / img->m_iNumFramesW,
            ih = img->h / img->m_iNumFramesH;
        m_bDead = false;
        m_uiID = id;
        m_uiFlags = 0;
        Box bxArea = Box(pos.x - iw / 2, pos.y - ih / 2, pos.z, iw, ih, 1);
        m_pRenderModel =  new OrderedRenderModel(img, bxArea, pos.z, iLayer);
        m_pPhysicsModel = new TimePhysicsModel(bxArea);
        setFlag(TPE_STATIC, true);
        setFlag(TPE_PASSABLE, true);
    }

    Decorative(uint id, Image *img, Box bxArea, int iLayer) {
        m_bDead = false;
        m_uiID = id;
        m_uiFlags = 0;
        m_pRenderModel =  new OrderedRenderModel(img, bxArea, bxArea.z, iLayer);
        m_pPhysicsModel = new TimePhysicsModel(bxArea);
        setFlag(TPE_STATIC, true);
        setFlag(TPE_PASSABLE, true);
    }
    
    Decorative(uint id, RenderModel *rm) {
        m_bDead = false;
        m_uiID = id;
        m_uiFlags = 0;
        m_pRenderModel =  rm;
        m_pPhysicsModel = new TimePhysicsModel(rm->getDrawArea());
        setFlag(TPE_STATIC, true);
        setFlag(TPE_PASSABLE, true);
    }

    virtual ~Decorative() {
        delete m_pRenderModel;
        delete m_pPhysicsModel;
    }

    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual bool update(uint time) { return m_bDead; }
    virtual uint getType() { return OBJ_DISPLAY; }

    void kill() { m_bDead = true; }

    //Render model
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

private:
    uint m_uiID;
    uint m_uiFlags;
    TimePhysicsModel  *m_pPhysicsModel;
    RenderModel *m_pRenderModel;
    bool m_bDead;
};

#endif
