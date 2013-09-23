/*
 * PartitionedWorldEngine.h
 * Defines an absolutely basic world engine
 */

#ifndef PARTITIONED_WORLD_ENGINE_H
#define PARTITIONED_WORLD_ENGINE_H

#include <map>
#include <list>
#include "mge/defs.h"
#include "mge/WorldEngine.h"
#include "mge/PhysicsEngine.h"
#include "mge/RenderEngine.h"
#include "mge/GameObject.h"
#include "mge/Event.h"

class PartitionedWorldEngine : public WorldEngine, public Listener, public EventHandler {
public:
    static void init()  { pwe = new PartitionedWorldEngine(); }
    static void clean() { delete pwe; }
    static PartitionedWorldEngine *get() { return pwe; }

    virtual void setPhysicsEngine(PhysicsEngine *pe) { this->pe = pe; }
    virtual void setRenderEngine(RenderEngine *re)   { this->re = re; }
    virtual uint genID() { return m_uiNextID++; }
    virtual void free(uint id) {}

    virtual void update(uint time);
    virtual void add(GameObject *obj);      //Adds object to current area
    virtual void remove(uint id);           //Removes object from current area and the screen

    //Specific to the partitioned world engine
    virtual void generateArea(uint uiAreaId);                   //generates a new area and returns its id
    virtual void setCurrentArea(uint uiAreaID);                 //Queue a set to the current area
    virtual uint getCurrentArea() { return m_uiCurArea; }
    virtual void setEffectiveArea(uint uiAreaID) { m_uiEffectiveArea = uiAreaID; }  //Set when adding objects to some area not currently set
    virtual void restoreEffectiveArea() { m_uiEffectiveArea = m_uiCurArea; }        //Unset when finished adding objects to a non-current area
    virtual void moveObjectToArea(uint uiObjID, uint uiStartAreaID, uint uiEndAreaID);
    virtual void addTo(GameObject *obj, uint uiAreaID);         //Adds object to specified area
    virtual void removeFrom(uint uiObjID, uint uiAreaID);       //Removes object from specified area and the screen
    virtual void cleanArea(uint uiAreaID);                      //Removes all objects from the specified area and deletes them.  Do NOT use on the current area!

    //Listener
    virtual uint getID() { return ID_WORLD_ENGINE; }
	virtual void callBack(uint cID, void *data, EventID id);

    //Event Handler
    virtual void addListener(Listener *pListener, EventID id, char* triggerData = 0) { addListener(pListener, id, m_uiEffectiveArea, triggerData); }
    virtual bool removeListener(uint uiListenerID, EventID eventID) { return removeListener(uiListenerID, eventID, m_uiEffectiveArea); }

    virtual void addListener(Listener *pListener, EventID id, uint uiAreaID, char* triggerData = 0);
    virtual bool removeListener(uint uiListenerID, EventID eventID, uint uiAreaID);

private:
    //Helper structures
    struct M_Area {
        std::map<uint, GameObject *> m_mCurArea;
        std::map<uint, Listener*> m_mMouseMoveListeners;
        std::map<uint, Listener*> m_mButtonInputListeners;
    };

    //Private constructor(s)/Destructor
    PartitionedWorldEngine();
    virtual ~PartitionedWorldEngine();
    virtual void setCurrentArea();      //Actually set the current area

    static PartitionedWorldEngine *pwe;

    PhysicsEngine *pe;
    RenderEngine  *re;

    std::map<uint, M_Area> m_mWorld;
    std::map<uint, GameObject *> *m_mCurArea;
    uint m_uiCurArea, m_uiNextArea, m_uiEffectiveArea;
    bool m_bFirstRun;

    uint m_uiNextID;

    M_Area *m_pCurArea;
};

typedef PartitionedWorldEngine PWE;

#endif
