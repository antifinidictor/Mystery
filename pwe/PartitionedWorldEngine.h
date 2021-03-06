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
#include "tpe/fluids/FluidOctree3d.h"

enum WorldState {
    PWE_PAUSED,
    PWE_RUNNING,
    PWE_NUM_STATES
};

enum WorldEvents {
    PWE_ON_AREA_SWITCH_TO = WORLD_EVENTS_BEGIN,
    PWE_ON_AREA_SWITCH_FROM,
    PWE_ON_ADDED_TO_AREA,
    PWE_ON_REMOVED_FROM_AREA,
    PWE_ON_ERASED_FROM_AREA,
    PWE_ON_WORLD_CLEANED,
    PWE_NUM_EVENT_IDS
};

enum WorldFlags {
    PWE_INFORM_OBJ_ADD = WORLD_FLAGS_BEGIN,
    PWE_INFORM_OBJ_REMOVE,
    PWE_SAVE_FILE_OBJ,  //Obj should be read from/written to a save file
    PWE_NUM_FLAGS
};

class PartitionedWorldEngine : public WorldEngine, public Listener, public EventHandler, public Scheduler {
public:
    static void init();
    static void clean() { delete pwe; }
    static PartitionedWorldEngine *get() { return pwe; }

    virtual void setPhysicsEngine(PhysicsEngine *pe) { this->pe = pe; }
    virtual void setRenderEngine(RenderEngine *re)   { this->re = re; }
    virtual uint genId();
    virtual uint peekId();  //Gets a new id while allowing it to remain free: Use if the ID will be reserved later
    virtual void freeId(uint id);
    virtual uint reserveId(uint id);

    virtual void update(float fDeltaTime);
    virtual void add(GameObject *obj);      //Adds object to current area
    virtual void remove(uint id);           //Removes object from current area and the screen

    GameObject *find(uint uiObjId);
    GameObject *findIn(uint uiObjId, uint uiAreaId);

    //Specific to the partitioned world engine
    uint generateArea();                                //generates a new area and returns its id
    void generateArea(uint uiAreaId);                   //generates a new area with the requested id
    void setCurrentArea(uint uiAreaId);                 //Queue a set to the current area
    uint getCurrentArea() { return m_uiCurArea; }
    void setEffectiveArea(uint uiAreaId) { m_uiEffectiveArea = uiAreaId; }  //Set when adding objects to some area not currently set
    void restoreEffectiveArea() { m_uiEffectiveArea = m_uiCurArea; }        //Unset when finished adding objects to a non-current area
    void moveObjectToArea(uint uiObjId, uint uiStartAreaId, uint uiEndAreaId);
    void addTo(GameObject *obj, uint uiAreaId);         //Adds object to specified area
    void removeFrom(uint uiObjId, uint uiAreaId);       //Removes object from specified area and the screen
    void cleanArea(uint uiAreaId);                      //Removes all objects from the specified area and deletes them.  Do NOT use on the current area!

    const std::string getAreaName(uint uiAreaId);
    void setAreaName(uint uiAreaId, const std::string &name);

    //Write/read particular areas
    void writeArea(uint uiAreaId, boost::property_tree::ptree &pt, const std::string &keyBase, bool bIsSaveFile = false);
    void readArea(uint uiAreaId, boost::property_tree::ptree &pt, const std::string &keyBase);

    //Write/read world files
    void write(boost::property_tree::ptree &pt, const std::string &keyBase, bool bIsSaveFile = false);
    void read(boost::property_tree::ptree &pt, const std::string &keyBase);

    void getAreas(std::vector<uint> &vAreas); //Populates the list with a list of areas

    //Listener
    virtual uint getId() { return ID_WORLD_ENGINE; }
	virtual int callBack(uint cId, void *data, uint id);

    //Event Handler
    virtual void addListener(Listener *pListener, uint eventId, char* triggerData = 0) { addListener(pListener, eventId, m_uiEffectiveArea, triggerData); }
    virtual bool removeListener(uint uiListenerId, uint eventId) { return removeListener(uiListenerId, eventId, m_uiEffectiveArea); }

    void addListener(Listener *pListener, uint eventId, uint uiAreaId, char* triggerData = 0);
    bool removeListener(uint uiListenerId, uint eventId, uint uiAreaId);

    //Scheduler
    virtual void scheduleUpdate(FluidOctreeNode *node);

    //General
    void setState(WorldState eState) { m_eNextState = eState; }
    void setManager(GameObject *obj) { m_pManagerObject = obj; }

    void cleanWorld(Listener *pCleanListener) { m_pCleanListener = pCleanListener; }

private:
    //Helper structures
    struct M_Area {
        M_Area() {
            m_sName = "";
            m_pOctree = NULL;
        }
        M_Area(const std::string &sName) {
            m_sName = sName;
            m_pOctree = NULL;
        }
        ~M_Area() {
            if(m_pOctree != NULL) {
                delete m_pOctree;
            }
        }
        std::string m_sName;
        FluidOctreeRoot *m_pOctree;
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

    void toPowerOfTwo(Box &in);

    static int nodeUpdateThread(void *data);
    static PartitionedWorldEngine *pwe;

    PhysicsEngine *pe;
    RenderEngine  *re;

    std::map<uint, M_Area> m_mWorld;
    uint m_uiCurArea, m_uiNextArea, m_uiEffectiveArea;
    M_Area *m_pCurArea;

    bool m_bFirstRun;
    bool m_bFinalCleaning;
    float m_fCurDeltaTime;  //used by the threads

    //Scheduled events
    std::list<uint> m_lsAreasToClean;
    std::list<FluidOctreeNode*> m_lsUpdateNodeQueue;
    std::list<SDL_Thread*> m_lsUpdateThreads;
    SDL_mutex *m_mxUpdateNodeQueue;


    GameObject *m_pManagerObject;   //This object performs management functions
    Listener *m_pCleanListener;     //This object listens for mid-game world-clean events

    uint m_uiNextId;
    uint m_uiNextAreaId;
    std::list<uint> m_lsFreeIds;

    WorldState m_eState, m_eNextState;
};

typedef PartitionedWorldEngine PWE;

#endif
