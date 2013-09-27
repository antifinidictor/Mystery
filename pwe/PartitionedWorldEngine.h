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

enum WorldState {
    PWE_PAUSED,
    PWE_RUNNING,
    PWE_NUM_STATES
};

enum WorldEvents {
    PWE_ON_AREA_SWITCH = NUM_EVENT_IDS,
    PWE_NUM_EVENT_IDS
};

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

    GameObject *find(uint uiObjId);
    GameObject *findIn(uint uiObjId, uint uiAreaId);

    //Specific to the partitioned world engine
    void generateArea(uint uiAreaId);                   //generates a new area and returns its id
    void setCurrentArea(uint uiAreaID);                 //Queue a set to the current area
    uint getCurrentArea() { return m_uiCurArea; }
    void setEffectiveArea(uint uiAreaID) { m_uiEffectiveArea = uiAreaID; }  //Set when adding objects to some area not currently set
    void restoreEffectiveArea() { m_uiEffectiveArea = m_uiCurArea; }        //Unset when finished adding objects to a non-current area
    void moveObjectToArea(uint uiObjID, uint uiStartAreaID, uint uiEndAreaID);
    void addTo(GameObject *obj, uint uiAreaID);         //Adds object to specified area
    void removeFrom(uint uiObjID, uint uiAreaID);       //Removes object from specified area and the screen
    void cleanArea(uint uiAreaID);                      //Removes all objects from the specified area and deletes them.  Do NOT use on the current area!

    const std::string getAreaName(uint uiAreaId);
    void setAreaName(uint uiAreaId, const std::string &name);
    void writeArea(uint uiAreaId, boost::property_tree::ptree &pt, const std::string &keyBase);
    void readArea(uint uiAreaId, boost::property_tree::ptree &pt, const std::string &keyBase);
    void write(boost::property_tree::ptree &pt, const std::string &keyBase);
    void read(boost::property_tree::ptree &pt, const std::string &keyBase);

    //Listener
    virtual uint getID() { return ID_WORLD_ENGINE; }
	virtual void callBack(uint cID, void *data, uint id);

    //Event Handler
    virtual void addListener(Listener *pListener, uint eventID, char* triggerData = 0) { addListener(pListener, eventID, m_uiEffectiveArea, triggerData); }
    virtual bool removeListener(uint uiListenerID, uint eventID) { return removeListener(uiListenerID, eventID, m_uiEffectiveArea); }

    void addListener(Listener *pListener, uint eventID, uint uiAreaID, char* triggerData = 0);
    bool removeListener(uint uiListenerID, uint eventID, uint uiAreaID);

    void setState(WorldState eState) { m_eNextState = eState; }
    void setManager(GameObject *obj) { m_pManagerObject = obj; }

private:
    //Helper structures
    struct M_Area {
        M_Area() {
            m_sName = "";
        }
        M_Area(const std::string &sName) {
            m_sName = sName;
        }
        std::string m_sName;
        std::map<uint, GameObject *> m_mCurArea;
        std::map<uint, Listener*> m_mMouseMoveListeners;
        std::map<uint, Listener*> m_mButtonInputListeners;
        std::map<uint, Listener*> m_mAreaChangeListeners;
    };

    //Private constructor(s)/Destructor
    PartitionedWorldEngine();
    virtual ~PartitionedWorldEngine();
    virtual void setCurrentArea();      //Actually set the current area

    void cleanAllAreas();
    void cleanAreaNow(uint uiAreaId);
    void removeFromNow(uint uiObjId, uint uiAreaId);
    void deleteFromNow(uint uiObjId, uint uiAreaId);
    void addToNow(GameObject *obj, uint uiAreaId);

    static PartitionedWorldEngine *pwe;

    PhysicsEngine *pe;
    RenderEngine  *re;

    std::map<uint, M_Area> m_mWorld;
    std::map<uint, GameObject *> *m_mCurArea;
    uint m_uiCurArea, m_uiNextArea, m_uiEffectiveArea;
    bool m_bFirstRun;

    //Scheduled events
    std::list<uint> m_lsAreasToClean;
    std::list<std::pair<uint,uint> > m_lsObjsToRemove;
    std::list<std::pair<uint,uint> > m_lsObjsToDelete;
    std::list<std::pair<GameObject*,uint> > m_lsObjsToAdd;

    GameObject *m_pManagerObject;   //This object performs management functions

    uint m_uiNextID;

    M_Area *m_pCurArea;
    WorldState m_eState, m_eNextState;
};

typedef PartitionedWorldEngine PWE;

#endif
