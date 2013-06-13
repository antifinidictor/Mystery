/*
 * Item.h
 * Defines a class of items that can be picked up and added to the player's inventory.
 */

#ifndef ITEM_H
#define ITEM_H
#include "mge/Event.h"
#include "mge/Image.h"
#include "mge/GameObject.h"
#include "game/GameDefs.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderModel.h"

struct ItemData {
    //For this game, all items have elemental values, from 0 to 256.  128 is neutral
    unsigned char earth, air, fire, water,
                  life, light, chaos, time;
    unsigned int uiItemID;
    ItemData() { earth = air = fire = water = life = light = chaos = time = 128; }
};


class ItemObject : public GameObject, public Listener {
public:
    //Constructor(s)/Destructor
    ItemObject(uint id, Image *img, Point pos);
    virtual ~ItemObject();

    //General
    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType() { return OBJ_ITEM; }

    virtual bool update(uint time);
    virtual void kill() { m_bDead = true; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Listener
    virtual void callBack(uint cID, void *data, EventID id);

private:
    uint m_uiID;
    uint m_uiFlags;
    TimePhysicsModel   *m_pPhysicsModel;
    OrderedRenderModel *m_pRenderModel;

    bool m_bDead;
};

#endif
