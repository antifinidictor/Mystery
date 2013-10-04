/*
 * AreaLinkObject
 * Objects that simply obey the laws of physics.  Nothing more.
 */

#ifndef AREA_LINK_OBJECT_H
#define AREA_LINK_OBJECT_H

#include "mge/GameObject.h"
#include "mge/Event.h"
#include "tpe/TimePhysicsModel.h"
#include "d3re/d3re.h"
#include "game/game_defs.h"

class AreaLinkObject : public GameObject {
public:
    AreaLinkObject(uint id, uint uiDestAreaId, const Point &ptDestPos, const Box &bxTriggerVolume);
    virtual ~AreaLinkObject();

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
    static const std::string getClassName()     { return "AreaLinkObject"; }

    //Listener
	virtual void callBack(uint uiID, void *data, uint eventId);

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

private:
    uint m_uiID, m_uiFlags;
    uint m_uiDestAreaId;
    Point m_ptDestPos;

    D3PrismRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
};

#endif
