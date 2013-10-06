/*
 * SimplePhysicsObject
 * Objects that simply obey the laws of physics.  Nothing more.
 */

#ifndef SIMPLE_PHYSICS_OBJECT_H
#define SIMPLE_PHYSICS_OBJECT_H

#include "mge/GameObject.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"
#include "game/game_defs.h"

class SimplePhysicsObject : public GameObject {
public:
    SimplePhysicsObject(uint id, uint texId, Box bxVolume, float fDensity = DENSITY_WOOD);      //Approx. density of granite

    virtual ~SimplePhysicsObject();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual uint getId() { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual bool update(uint time)              { return false; }
    virtual uint getType() { return TYPE_GENERAL; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "SimplePhysicsObject"; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Misc
    void setColor(const Color &cr) { m_pRenderModel->setColor(cr); }
    Color &getColor() { return m_pRenderModel->getColor(); }

    //Listener
    virtual void callBack(uint uiID, void *data, uint id) {}

private:
    uint m_uiID, m_uiFlags;

    D3PrismRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
};

#endif
