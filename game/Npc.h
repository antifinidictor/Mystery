/*
 * Npc.h
 * Defines a first try at an Npc class.
 */
#ifndef Npc_H
#define Npc_H

#include "mge/GameObject.h"
#include "mge/Event.h"
#include "mge/Image.h"
#include "mge/defs.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderModel.h"
#include "tpe/TimePhysicsEngine.h"
#include "game/GameDefs.h"

class Npc : public GameObject, public Listener {
public:
    //Constructor(s)/Destructor
    Npc(uint uiID, Image *pImage, Point pos);
    ~Npc();

    //General
    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }

    virtual bool update(uint time);
    virtual uint getType() { return OBJ_NPC; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Listener
    virtual void callBack(uint cID, void *data, EventID id);

private:
    uint m_uiID;
    uint m_uiFlags;
    TimePhysicsModel *m_pPhysicsModel;
    OrderedRenderModel  *m_pRenderModel;
    int m_iTimer, m_iFrame, m_iDirection;
    uint m_uiLastUpdated;

    enum NpcStates {
        NPC_STAND,
        NPC_TURN,
        NPC_WALK,
        NPC_NUM_STATES
    };

    NpcStates m_eState;
};

#endif
