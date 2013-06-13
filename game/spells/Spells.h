/*
 * Spells.h
 * Complete set of class definitions for spell classes
 * There will eventually be 400 of them.
 * Base class is a private class that takes care of the basic GameObject
 * functionality so the rest don't have to worry about it.
 */

#ifndef SPELLS_H
#define SPELLS_H
#include "mge/defs.h"
#include "mge/Image.h"
#include "mge/GameObject.h"
#include "mge/Event.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderModel.h"
#include "game/Decorative.h"

class HandleCollisionData;

enum AreaOfEffect {
    AOE_DIRECTED,       //Effect fired like a projectile
    AOE_AREA,           //Affects a wide area around target
    AOE_ENVIRONMENT,    //Affects the entire environment
    AOE_NUM_TYPES
};

class BaseSpell : public GameObject, public Listener {
public:
    BaseSpell(uint uiID, OrderedRenderModel *rm, TimePhysicsModel *pm);
    BaseSpell(uint uiID, Point ptPlayerPos, Point ptClickPos, int iLayer, AreaOfEffect aoe);
    virtual ~BaseSpell();

    //General
    virtual uint getID() { return m_uiID; }
    //virtual bool update(uint time);
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType() { return OBJ_SPELL; }

    //Listener
	//virtual void callBack(uint cID, void *data, EventID id);

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Access
    virtual void setFrameW(uint f);
    virtual void setFrameH(uint f);

private:
    uint m_uiID;
    uint m_uiFlags;

    OrderedRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
};

/* * * * * * * * * *\
 * Level 1 Spells  *
\* * * * * * * * * */
/*
//Earth: Summon block
class EarthSpell : public BaseSpell {
public:
    EarthSpell(Point ptPlayerPos, Point ptClickPos, int power);
    virtual bool update(uint time);
	virtual void callBack(uint cID, void *data, EventID id);
};
GameObject *createEarthSpell(Point ptPlayerPos, Point ptClickPos, int power);
*/

//Air: Lift
class AirSpell : public BaseSpell {
public:
    AirSpell(uint uiID, Point ptPlayerPos, Point ptClickPos, int power);
    virtual ~AirSpell();
    virtual bool update(uint time);
	virtual void callBack(uint cID, void *data, EventID id);
private:
    int timer;
    Point direction;
    GameObject *target;
    float m_fBaseHeight;
    void handleCollision(HandleCollisionData* data);
    Decorative *m_pShadow;
    bool m_bWasStatic;
    float m_fPrevFriction;
    static const int BASE_HEIGHT = 5;
    uint m_uiFrame;
};
GameObject *createAirSpell(Point ptPlayerPos, Point ptClickPos, int power);

//Fire: Burn something
//Water: Generate water stream
#endif
