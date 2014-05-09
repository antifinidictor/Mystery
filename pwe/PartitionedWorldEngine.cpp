/*
 * Source file for the basic world engine
 */

#include "PartitionedWorldEngine.h"
#include "mge/ModularEngine.h"
#include "game/ObjectFactory.h"
#include <list>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "tpe/fluids/FluidOctree3d.h"
#include "mge/ConfigManager.h"

using namespace std;

static int s_iNumMainThreadUpdates = 0;

int
PartitionedWorldEngine::nodeUpdateThread(void *data) {
    SDL_threadID threadID = SDL_ThreadID();    //For debugging
    int iNumUpdates = 0;
    printf("Thread %d executing\n", threadID);

    bool *bStop = (bool*)data;
    while(!*bStop) {
        //Get the queue lock
        SDL_LockMutex(pwe->m_mxUpdateNodeQueue);
        if(pwe->m_lsUpdateNodeQueue.size() < 2) {   //Last node or so needs to be updated by the root thread for some reason

            //If the list is empty, unlock and wait a bit before trying again
            SDL_UnlockMutex(pwe->m_mxUpdateNodeQueue);

            //printf("Thread %d did not find enough nodes to update\n", threadID);
            SDL_Delay(1);
        } else {

            //Otherwise, get the next node waiting for processing
            FluidOctreeNode *node = pwe->m_lsUpdateNodeQueue.front();
            pwe->m_lsUpdateNodeQueue.pop_front();
            //int iNumItems = pwe->m_lsUpdateNodeQueue.size();

            //Unlock the list so other nodes can be updated
            SDL_UnlockMutex(pwe->m_mxUpdateNodeQueue);

            //printf("Thread %d found %d+1 nodes awaiting update\n", threadID, iNumItems);
            iNumUpdates++;

            //Update the node
            node->update(pwe->m_fCurDeltaTime);
        }
    }

    printf("Thread %d exiting; updated %d times\n", threadID, iNumUpdates);
    return 0;
}

void
PartitionedWorldEngine::init() {
    pwe = new PartitionedWorldEngine();

    //Create the threads
    using boost::lexical_cast;
    int numThreads = ConfigManager::get()->get("pwe.threads", 4);
    string threadNameBase = "PWE_UpdateNode";
    for(int curThread = 0; curThread < numThreads; ++curThread) {
        string threadName = threadNameBase + lexical_cast<string>(curThread);
        printf("Created thread %s\n", threadName.c_str());
        SDL_Thread *thread = SDL_CreateThread(nodeUpdateThread, threadName.c_str(), &pwe->m_bFinalCleaning);
        pwe->m_lsUpdateThreads.push_back(thread);
    }
}

PartitionedWorldEngine *PartitionedWorldEngine::pwe;

PartitionedWorldEngine::PartitionedWorldEngine()
    :   pe(NULL),
        re(NULL),
        m_uiCurArea(0),
        m_uiNextArea(0),
        m_uiEffectiveArea(0),
        m_pCurArea(NULL),
        m_bFirstRun(true),
        m_bFinalCleaning(false),
        m_fCurDeltaTime(1.f),
        m_mxUpdateNodeQueue(SDL_CreateMutex()),
        m_pManagerObject(NULL),
        m_pCleanListener(NULL),
        m_uiNextId(ID_FIRST_UNUSED),    //The first few ids are reserved for the engines
        m_uiNextAreaId(1),
        m_eState(PWE_RUNNING),
        m_eNextState(PWE_RUNNING)
{
    assert(PWE_NUM_FLAGS <= WORLD_FLAGS_END);

    printf("World engine has id %d\n", getId());

    ModularEngine *mge = ModularEngine::get();
    mge->addListener(this, ON_MOUSE_MOVE);
    mge->addListener(this, ON_BUTTON_INPUT);

    //Create the mutex and initially lock it
    SDL_LockMutex(m_mxUpdateNodeQueue);

    //Threads are created after pwe is initialized
}

PartitionedWorldEngine::~PartitionedWorldEngine() {
    printf("World engine cleaning\n");
    m_bFinalCleaning = true;
    SDL_UnlockMutex(m_mxUpdateNodeQueue);
    for(list<SDL_Thread*>::iterator it = m_lsUpdateThreads.begin(); it != m_lsUpdateThreads.end(); ++it) {
        int status;
        SDL_WaitThread(*it, &status);
    }

    MGE::get()->removeListener(this->getId(), ON_MOUSE_MOVE);
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);
    //Need to free everything
    cleanAllAreas();

    SDL_DestroyMutex(m_mxUpdateNodeQueue);
    printf("Main thread updated %d times\n", s_iNumMainThreadUpdates);
}


uint
PartitionedWorldEngine::genId() {
    uint id;
    if(m_lsFreeIds.size() > 0) {
        id = m_lsFreeIds.back();
        m_lsFreeIds.pop_back();
    } else {
        id = m_uiNextId++;
    }
    printf("NOTE %s %d: Generated id %d\n", __FILE__, __LINE__, id);
    return id;
}

uint
PartitionedWorldEngine::peekId() {
    uint id;
    if(m_lsFreeIds.size() > 0) {
        id = m_lsFreeIds.back();
    } else {
        id = m_uiNextId;
    }
    return id;
}

void
PartitionedWorldEngine::freeId(uint id) {
    m_lsFreeIds.push_front(id);
    printf("NOTE %s %d: Freed id %d\n", __FILE__, __LINE__, id);
}

uint
PartitionedWorldEngine::reserveId(uint id) {
    if(id >= m_uiNextId) {
        while(id > m_uiNextId) {
            m_lsFreeIds.push_front(m_uiNextId++);
        }
        m_uiNextId++;   //This id is not actually free
    } else {
        //Verify Id is actually free!
        for(list<uint>::iterator iter = m_lsFreeIds.begin(); iter != m_lsFreeIds.end(); ++iter) {
            if(*iter == id) {
                m_lsFreeIds.erase(iter);
                printf("NOTE %s %d: Reserved id %d\n", __FILE__, __LINE__, id);
                return id;
            }
        }
        uint id2 = genId();
        printf("WARNING %s %d: Id %d already in use! Replacing with %d, which will break references to this object!\n",
               __FILE__, __LINE__, id, id2);
       id = id2;
    }
    printf("NOTE %s %d: Reserved id %d\n", __FILE__, __LINE__, id);
    return id;
}

void
PartitionedWorldEngine::update(float fDeltaTime) {
    m_fCurDeltaTime = fDeltaTime;

    //Manager is always updated
    if(m_pManagerObject != NULL) {
        m_pManagerObject->update(fDeltaTime);
    }

    pe->update(fDeltaTime);

    //m_pCurArea->m_pOctree->scheduleUpdates(BasicScheduler::get(fDeltaTime, m_eState == PWE_PAUSED));
    m_pCurArea->m_pOctree->scheduleUpdates(this);

    //Perform updates using this thread. The queue is locked by this thread between updates;
    // other threads
    while(m_lsUpdateNodeQueue.size() > 0) {
        //Get the first item from the queue
        FluidOctreeNode *node = pwe->m_lsUpdateNodeQueue.front();
        m_lsUpdateNodeQueue.pop_front();
        //int iNumItems = m_lsUpdateNodeQueue.size();

        //Unlock the list so other nodes can be updated by other threads
        SDL_UnlockMutex(pwe->m_mxUpdateNodeQueue);

        //printf("Main thread updates list, num items = %d\n", iNumItems);
        s_iNumMainThreadUpdates++;

        //Update the node
        if(m_eState == PWE_PAUSED) {
            node->updateAddRemoveErase();
        } else {
            node->update(pwe->m_fCurDeltaTime);
        }

        //Get the queue lock.  Notice we keep the lock if the queue is empty
        SDL_LockMutex(pwe->m_mxUpdateNodeQueue);
    }
    //printf("Main thread finished updating the list (%d items)\n", m_lsUpdateNodeQueue.size());

    if(m_pCleanListener) {
        re->clearScreen();
        cleanAllAreas();
        m_uiNextArea = 0;

        //Inform the clean listener that the world has been reset
        m_pCleanListener->callBack(getId(), NULL, PWE_ON_WORLD_CLEANED);
        m_pCleanListener = NULL;
    } else {
        list<uint>::iterator itAreaId;
        for(itAreaId = m_lsAreasToClean.begin(); itAreaId != m_lsAreasToClean.end(); ++itAreaId) {
            cleanAreaNow(*itAreaId);
        }
    }

    //If necessary, set the current area (we wanted to finish this update first)
    if(m_uiCurArea != m_uiNextArea) {
        setCurrentArea();
    }
    m_bFirstRun = false;
    m_eState = m_eNextState;
}

void
PartitionedWorldEngine::add(GameObject *obj) {
    //TODO: improve efficiency by sticking actual implementation here
    addTo(obj, m_uiEffectiveArea);
}

void
PartitionedWorldEngine::remove(uint id) {
    //TODO: improve efficiency by sticking actual implementation here
    removeFrom(id, m_uiEffectiveArea);
}

/*
 * Partitioned-World specific functions
 */
GameObject *
PartitionedWorldEngine::find(uint uiObjId) {
    return findIn(uiObjId, m_uiEffectiveArea);
}

void
PartitionedWorldEngine::addTo(GameObject *obj, uint uiAreaId) {
    if(obj == NULL) {
        printf("ERROR %s %d: Tried to add NULL object to area %d!\n", __FILE__, __LINE__, uiAreaId);
        return;
    }
    //m_lsObjsToAdd.push_back(pair<GameObject*,uint>(obj, uiAreaId));
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    if(itArea != m_mWorld.end()) {
        //itArea->second.m_mCurArea[obj->getId()] = obj;
        //obj->setFlag(PWE_INFORM_OBJ_ADD, true); //So the object is informed of its new place
        itArea->second.m_pOctree->add(obj);
        obj->callBack(getId(), &uiAreaId, PWE_ON_ADDED_TO_AREA);
    } else {
        printf("ERROR %s %d: Tried to add object to nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return;
    }
}

void
PartitionedWorldEngine::removeFrom(uint uiObjId, uint uiAreaId) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to remove object from nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return;
    }

    //TODO: Make less hacky
    GameObject *obj = itArea->second.m_pOctree->find(uiObjId);
    if(obj != NULL) {
        obj->setFlag(PWE_INFORM_OBJ_REMOVE, true);
        itArea->second.m_pOctree->remove(uiObjId);
        obj->callBack(getId(), &uiAreaId, PWE_ON_REMOVED_FROM_AREA);
    } else {
        printf("ERROR %s %d: Tried to remove nonexistent object %d\n", __FILE__, __LINE__, uiObjId);
        return;
    }
}

uint
PartitionedWorldEngine::generateArea() {
    generateArea(m_uiNextAreaId);
    return m_uiNextAreaId - 1;
}

GameObject *
PartitionedWorldEngine::findIn(uint uiObjId, uint uiAreaId) {
    //Extract the object
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    map<uint, GameObject*>::iterator itObj;
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to find object in area %d that doesn't exist\n", __FILE__, __LINE__, uiAreaId);
        return NULL;
    }

    return itArea->second.m_pOctree->find(uiObjId);
}

void
PartitionedWorldEngine::generateArea(uint uiAreaId) {
    while(uiAreaId >= m_uiNextAreaId) {
        m_uiNextAreaId++;
    }

    std::ostringstream name;
    name << "Area" << uiAreaId;
    m_mWorld[uiAreaId] = M_Area(name.str());

    //TODO: Areas should only really be generated by the editor, so we won't worry about the size for now
    //m_mWorld[uiAreaId].m_pOctree = new FluidOctreeRoot(getId(), uiAreaId, Box(-1.f,-1.f,-1.f,2.f,2.f,2.f));
printf(__FILE__" %d: New area %d, null octree\n",__LINE__, uiAreaId);
}

void
PartitionedWorldEngine::setCurrentArea(uint uiAreaId) {
    m_uiNextArea = uiAreaId;

    //Call immediately if the first update has not yet occurred
    if(m_bFirstRun) {
        setCurrentArea();
    }
}

void
PartitionedWorldEngine::setCurrentArea() {
    map<uint, M_Area>::iterator it = m_mWorld.find(m_uiNextArea);
    if(it != m_mWorld.end()) {
        //Inform listeners of old current area that we are switching away
        map<uint, Listener*>::iterator iter;
        if(m_pCurArea) {
            for(iter = m_pCurArea->m_mAreaChangeListeners.begin();
                iter != m_pCurArea->m_mAreaChangeListeners.end();
                ++iter ) {
                int status = iter->second->callBack(ID_MODULAR_ENGINE, NULL, PWE_ON_AREA_SWITCH_FROM);
                if(status == EVENT_CAUGHT) {
                    break;
                }
            }
		}

        //Switch away
        m_pCurArea = &(it->second);
        m_uiCurArea = m_uiNextArea;
        m_uiEffectiveArea = m_uiNextArea;
        re->clearScreen();
        re->moveScreenTo(Point());

        //Inform listeners of new current area that we are switching to
		for(iter = m_pCurArea->m_mAreaChangeListeners.begin();
			iter != m_pCurArea->m_mAreaChangeListeners.end();
			++iter ) {
			int status = iter->second->callBack(ID_MODULAR_ENGINE, NULL, PWE_ON_AREA_SWITCH_TO);
			if(status == EVENT_CAUGHT) {
                break;
			}
		}

    } else {
        printf("ERROR %s %d: Tried to set current area to a map %d that doesn't exist\n", __FILE__, __LINE__, m_uiNextArea);
    }
}

void
PartitionedWorldEngine::moveObjectToArea(uint uiObjId, uint uiStartAreaId, uint uiEndAreaId) {
    GameObject *obj = findIn(uiObjId, uiStartAreaId);
    removeFrom(uiObjId, uiStartAreaId);
    addTo(obj, uiEndAreaId);

#if 0
    if(obj == NULL) {
        map<uint, M_Area>::iterator itArea;
        for(itArea = m_mWorld.begin(); itArea != m_mWorld.end(); ++itArea) {
            obj = itArea->second.m_pOctree->find(uiObjId);
            if(obj != NULL) {
                printf("Found object in area %d instead of area %d\n", itArea->first, uiStartAreaId);
            }
        }
    } else {
        printf("Obj moved successfully\n");
    }
#endif
}

void
PartitionedWorldEngine::cleanArea(uint uiAreaId) {
    m_lsAreasToClean.push_back(uiAreaId);
}

const std::string
PartitionedWorldEngine::getAreaName(uint uiAreaId) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to get the name of a nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return "[nonexistent area]";
    }
    return itArea->second.m_sName;
}

void
PartitionedWorldEngine::setAreaName(uint uiAreaId, const std::string &name) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to name nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return;
    }
    itArea->second.m_sName = name;
}

void
PartitionedWorldEngine::writeArea(uint uiAreaId, boost::property_tree::ptree &pt, const std::string &keyBase, bool bIsSaveFile) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    map<uint, GameObject*>::iterator itObj;
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to write nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return;
    }

    //Key-base should already be the area name
    pt.put(keyBase, uiAreaId);

    itArea->second.m_pOctree->write(pt, keyBase, bIsSaveFile);
}


void
PartitionedWorldEngine::readArea(uint uiAreaId, boost::property_tree::ptree &pt, const std::string &keyBase) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    if(itArea == m_mWorld.end()) {
        generateArea(uiAreaId);
        itArea = m_mWorld.find(uiAreaId);   //Used for setting up octree

        //Set the area name
        int iStartAreaNameIndex = keyBase.find_last_of('.') + 1;
        setAreaName(uiAreaId, keyBase.substr(iStartAreaNameIndex));
    }

    Point ptObjMin;
    Point ptObjMax;
    list<GameObject *> lsObjsToAdd;

    using boost::property_tree::ptree;
    string className, key;
    BOOST_FOREACH(ptree::value_type &c, pt.get_child(keyBase)) {
        //Each class
        className = c.first.data();
        ObjectFactory::get()->initClass(className);
        key = keyBase + "." + className;
        BOOST_FOREACH(ptree::value_type &o, pt.get_child(key)) {
//printf(__FILE__" %d %s\n",__LINE__, className.c_str());
            GameObject *obj = ObjectFactory::get()->createFromTree(pt, key + "." + o.first.data());
            if(obj != NULL) {
                //addTo(obj, uiAreaId);
                lsObjsToAdd.push_back(obj);

                //Calculate base size of octree
                Box bxVolume = obj->getPhysicsModel()->getCollisionVolume();
                if(bxVolume.x < ptObjMin.x) {
                    ptObjMin.x = bxVolume.x;
                }
                if(bxVolume.x + bxVolume.w > ptObjMax.x) {
                    ptObjMax.x = bxVolume.x + bxVolume.w;
                }
                if(bxVolume.y < ptObjMin.y) {
                    ptObjMin.y = bxVolume.y;
                }
                if(bxVolume.y + bxVolume.h > ptObjMax.y) {
                    ptObjMax.y = bxVolume.y + bxVolume.h;
                }
                if(bxVolume.z < ptObjMin.z) {
                    ptObjMin.z = bxVolume.z;
                }
                if(bxVolume.z + bxVolume.l > ptObjMax.z) {
                    ptObjMax.z = bxVolume.z + bxVolume.l;
                }
            }
        }
    }

    //Octree bounds will be converted to nearest power-of-2s
    Box bxBounds = Box(
        floor(ptObjMin.x),
        floor(ptObjMin.y),
        floor(ptObjMin.z),
        ceil(ptObjMax.x) - floor(ptObjMin.x),
        ceil(ptObjMax.y) - floor(ptObjMin.y),
        ceil(ptObjMax.z) - floor(ptObjMin.z)
    );

    toPowerOfTwo(bxBounds);

    //Now we create the octree
    if(itArea->second.m_pOctree == NULL) {
        itArea->second.m_pOctree = new FluidOctreeRoot(getId(), uiAreaId, bxBounds, 2.f);
    }
    //itArea->second.m_pOctree->debugPrintBounds();

    //Fill the octree
    for(list<GameObject*>::iterator it = lsObjsToAdd.begin(); it != lsObjsToAdd.end(); ++it) {
        //Old add code
        //addTo(*it, uiAreaId);

        //New add code
        // (*it)->setFlag(PWE_INFORM_OBJ, true); //So the object is informed of its new place
        itArea->second.m_pOctree->add(*it);
        (*it)->callBack(getId(), &uiAreaId, PWE_ON_ADDED_TO_AREA);
    }

}

void
PartitionedWorldEngine::write(boost::property_tree::ptree &pt, const std::string &keyBase, bool bIsSaveFile) {
    map<uint, M_Area>::iterator itArea;
    for(itArea = m_mWorld.begin(); itArea != m_mWorld.end(); ++itArea) {
        writeArea(itArea->first, pt, keyBase + "." + itArea->second.m_sName, bIsSaveFile);
    }
}

void
PartitionedWorldEngine::read(boost::property_tree::ptree &pt, const std::string &keyBase) {
    using boost::property_tree::ptree;
    string key;
    try {
    BOOST_FOREACH(ptree::value_type &a, pt.get_child(keyBase)) {
        //Each area
        key = keyBase + "." + a.first.data();
        uint id = pt.get(key, 0);
        readArea(id, pt, key);
        //setAreaName(id, a.first.data());
    }
    } catch(exception &e) {
        printf("Could not read objects: %s\n", e.what());
    }
}

void
PartitionedWorldEngine::getAreas(std::vector<uint> &vAreas) {
    map<uint, M_Area>::iterator itArea;
    for(itArea = m_mWorld.begin(); itArea != m_mWorld.end(); ++itArea) {
        vAreas.push_back(itArea->first);
    }
}

//Event Handler
void
PartitionedWorldEngine::addListener(Listener *pListener, uint id, uint uiAreaId, char* triggerData) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to add listener to nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return;
    }

	switch(id) {
	case ON_MOUSE_MOVE:
		itArea->second.m_mMouseMoveListeners[pListener->getId()] = pListener;
		break;
	case ON_BUTTON_INPUT:
		itArea->second.m_mButtonInputListeners[pListener->getId()] = pListener;
		break;
    case PWE_ON_AREA_SWITCH_TO:
    case PWE_ON_AREA_SWITCH_FROM:
        itArea->second.m_mAreaChangeListeners[pListener->getId()] = pListener;
        break;
	default:
		printf("Unsupported event handle %d.\n", id);
	}
}

bool
PartitionedWorldEngine::removeListener(uint uiListenerId, uint eventId, uint uiAreaId) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
	map<uint, Listener*>::iterator iter;
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to remove listener from nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return false;
    }

	switch(eventId) {
	case ON_MOUSE_MOVE:
        iter = itArea->second.m_mMouseMoveListeners.find(uiListenerId);
        if(iter != itArea->second.m_mMouseMoveListeners.end()) {
            itArea->second.m_mMouseMoveListeners.erase(iter);
            return true;
        }
		break;
	case ON_BUTTON_INPUT:
        iter = itArea->second.m_mButtonInputListeners.find(uiListenerId);
        if(iter != itArea->second.m_mButtonInputListeners.end()) {
            itArea->second.m_mButtonInputListeners.erase(iter);
            return true;
        }
		break;
    case PWE_ON_AREA_SWITCH_TO:
    case PWE_ON_AREA_SWITCH_FROM:
        iter = itArea->second.m_mAreaChangeListeners.find(uiListenerId);
        if(iter != itArea->second.m_mAreaChangeListeners.end()) {
            itArea->second.m_mAreaChangeListeners.erase(iter);
            return true;
        }
        break;
	default:
		printf("Unsupported event handle %d.\n", eventId);
		return false;
	}
	return false;
}


void
PartitionedWorldEngine::scheduleUpdate(FluidOctreeNode *node) {
    //Assumes list is already locked
    m_lsUpdateNodeQueue.push_back(node);
}

int
PartitionedWorldEngine::callBack(uint cId, void *data, uint id) {
    if(m_pCurArea == NULL) { return EVENT_DROPPED; }

    int status = EVENT_DROPPED;
    //Pass directly on to the current list of listeners
	map<uint,Listener*>::iterator iter;
	switch(id) {
	case ON_MOUSE_MOVE:
		for( iter = m_pCurArea->m_mMouseMoveListeners.begin();
			iter != m_pCurArea->m_mMouseMoveListeners.end();
			++iter ) {
			status = iter->second->callBack(ID_MODULAR_ENGINE, data, id);
			if(status == EVENT_CAUGHT) {
                //printf("\tObject %d in pwe caught the mouse-move event\n", iter->second->getId());
                break;
			}
		}
		break;
	case ON_BUTTON_INPUT:
		for( iter = m_pCurArea->m_mButtonInputListeners.begin();
			iter != m_pCurArea->m_mButtonInputListeners.end();
			++iter ) {
			status = iter->second->callBack(ID_MODULAR_ENGINE, data, id);
			if(status == EVENT_CAUGHT) {
                //printf("\tObject %d in pwe caught the button event\n", iter->second->getId());
                break;
			}
		}
		break;
	default:
		printf("Unsupported event handle %d.\n", id);
	}
	return status;
}

void
PartitionedWorldEngine::cleanAllAreas() {
    map<uint, M_Area>::iterator itArea;
    for(itArea = m_mWorld.begin(); itArea != m_mWorld.end(); ++itArea) {
        //TODO: improve efficiency by sticking actual implementation here
        cleanAreaNow(itArea->first);
    }
    //m_mWorld.clear(); TODO: Somehow this line causes a major memory bug on new/load!
}


void
PartitionedWorldEngine::cleanAreaNow(uint uiAreaId) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    map<uint, GameObject*>::iterator itObj;
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to clean nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return;
    }
    printf("Cleaning area %s (%d)\n", itArea->second.m_sName.c_str(), itArea->first);

    //Delete the objects from the area
    //TODO: This may be the cause of a segfault.  Do we want to just erase the objects, or erase the octree?
    delete itArea->second.m_pOctree;
    itArea->second.m_pOctree = NULL;

    //Clear the lists of the bad pointers
    itArea->second.m_mMouseMoveListeners.clear();
    itArea->second.m_mButtonInputListeners.clear();

    //We assume the screen has already been cleared of these objects, which is
    // why you should never call this function on the current area
}


void
PartitionedWorldEngine::toPowerOfTwo(Box &bx) {
    int w = (int)ceil(bx.w);
    int h = (int)ceil(bx.h);
    int l = (int)ceil(bx.l);

    //Convert each to a power of two
    int logw = 0, logh = 0, logl = 0;
    while(w >>= 1) { logw++; }
    while(h >>= 1) { logh++; }
    while(l >>= 1) { logl++; }
    w = 1 << (logw + 1);
    h = 1 << (logh + 1);
    l = 1 << (logl + 1);

    Point ptCenter = bxCenter(bx);
    bx.x = floor(ptCenter.x) - w / 2;
    bx.y = floor(ptCenter.y) - h / 2;
    bx.z = floor(ptCenter.z) - l / 2;
    bx.w = w;
    bx.h = h;
    bx.l = l;
}
