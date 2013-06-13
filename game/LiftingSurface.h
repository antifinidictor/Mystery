/*
 * LiftingSurface
 * Surface designed for spellcasting support
 */
#ifndef LIFTING_SURFACE_H
#define LIFTING_SURFACE_H

#include <vector>
#include "mge/GameObject.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderModel.h"
#include "ore/OrderedRenderEngine.h"
#include "ore/EdgeRenderModel.h"
#include "game/GameDefs.h"
#include "mge/Event.h"

class LiftingSurface : public GameObject, public Listener, public EdgeList {
public:
    LiftingSurface(uint id, Image *img, Box bxVolume);

    virtual ~LiftingSurface() {
        delete m_pRenderModel;
        delete m_pPhysicsModel;
        m_vNorth.clear();
        m_vEast.clear();
        m_vSouth.clear();
        m_vWest.clear();
    }

    //General
    virtual uint getID() { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType() { return OBJ_SURFACE; }
    virtual bool update(uint time);
	virtual void callBack(uint cID, void *data, EventID id);
    
    //EdgeList
    virtual Box getVolume() { return Box(); }
    virtual bool hasNext();
    virtual Box nextVolume();
    virtual void setList(int iDir);

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

private:
    uint m_uiID;
    uint m_uiFlags;
    
    std::vector<LiftingSurface*> m_vNorth, m_vEast, m_vSouth, m_vWest;
    std::vector<LiftingSurface*>::iterator m_itrCurList;
    int m_iCount;

    EdgeRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
};

#endif
