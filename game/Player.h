/*
 * Player.h
 * Defines a simple player class.
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "mge/GameObject.h"
#include "mge/Event.h"
#include "mge/Image.h"
#include "mge/defs.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderModel.h"
#include "game/FileManager.h"
#include "game/Item.h"
#include "game/GameDefs.h"

class Player : public GameObject, public Listener {
public:
    //Constructor(s)/Destructor
    Player(uint id, Image *img, Point pos);
    virtual ~Player();
    
    //General
    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }

    virtual bool update(uint time);
    virtual uint getType() { return OBJ_PLAYER; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Listener
    virtual void callBack(uint cID, void *data, EventID id);

    //File I/O
    static Player *read(FileManager *mgr);
    void write(FileManager *mgr);

private:
    uint m_uiID;
    uint m_uiFlags;
    TimePhysicsModel *m_pPhysicsModel;
    OrderedRenderModel  *m_pRenderModel;
    
    int dx, dy;
    int timer, state;
    int m_iDirection;
    
    void handleButton(InputData* data);
    void handleCollision(HandleCollisionData *mdl);
};

#endif
