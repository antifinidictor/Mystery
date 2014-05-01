/*
 * SimpleResettableObject
 * Objects that simply obey the laws of physics.  Nothing more.
 */

#ifndef SIMPLE_RESETTABLE_OBJECT_H
#define SIMPLE_RESETTABLE_OBJECT_H

#include "mge/GameObject.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"
#include "game/game_defs.h"

class SimpleResettableObject : public GameObject {
public:
    SimpleResettableObject(uint id, uint texId, Box bxVolume, float fDensity = DENSITY_WOOD);      //Approx. density of granite

    virtual ~SimpleResettableObject();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual uint getId() { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual bool update(float fDeltaTime);
    virtual uint getType() { return TYPE_GENERAL; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "SimpleResettableObject"; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Misc
    void setColor(const Color &cr) { m_pRenderModel->setColor(cr); }
    Color &getColor() { return m_pRenderModel->getColor(); }

    //Listener
    virtual int callBack(uint uiId, void *data, uint uiEventId);

private:
    void reset();

    uint m_uiID;
    flag_t m_uiFlags;
    uint m_uiArea;
    Point m_ptOriginalPosition;

    D3PrismRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
    int m_iSoundChannel;
};

#endif
