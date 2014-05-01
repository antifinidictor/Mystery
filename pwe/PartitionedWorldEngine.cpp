/*
 * Source file for the basic world engine
 */

#include "PartitionedWorldEngine.h"
#include "mge/ModularEngine.h"
#include "game/ObjectFactory.h"
#include <list>
#include <boost/foreach.hpp>
#include "tpe/fluids/FluidOctree3d.h"
using namespace std;

PartitionedWorldEngine *PartitionedWorldEngine::pwe;

PartitionedWorldEngine::PartitionedWorldEngine() {
    assert(PWE_NUM_FLAGS <= WORLD_FLAGS_END);

    printf("World engine has id %d\n", getId());
    m_uiNextId = ID_FIRST_UNUSED;   //The first few ids are reserved for the engines
    m_uiNextAreaId = 1;
    m_uiCurArea = m_uiNextArea = m_uiEffectiveArea = 0;
    pe = NULL;
    re = NULL;
    m_pCurArea = NULL;
    m_bFirstRun = true;
    ModularEngine *mge = ModularEngine::get();
    mge->addListener(this, ON_MOUSE_MOVE);
    mge->addListener(this, ON_BUTTON_INPUT);
    m_eNextState = m_eState = PWE_RUNNING;
    m_pManagerObject = NULL;
}

PartitionedWorldEngine::~PartitionedWorldEngine() {
    printf("World engine cleaning\n");
    MGE::get()->removeListener(this->getId(), ON_MOUSE_MOVE);
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);
    //Need to free everything
    cleanAllAreas();
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
                return id;
            }
        }
        uint id2 = genId();
        printf("WARNING %s %d: Id %d already in use! Replacing with %d, which will break references to this object!\n",
               __FILE__, __LINE__, id, id2);
       id = id2;
    }
    return id;
}

void
PartitionedWorldEngine::update(float fDeltaTime) {
    //Manager is always updated
    if(m_pManagerObject != NULL) {
        m_pManagerObject->update(fDeltaTime);
    }


    if(m_eState == PWE_RUNNING) {   //State can be disabled
        //Update the physics engine clock
        pe->update(fDeltaTime);

        m_pCurArea->m_pOctree->scheduleUpdates(BasicScheduler::get(fDeltaTime));
    }

    list<uint>::iterator itAreaId;
    for(itAreaId = m_lsAreasToClean.begin(); itAreaId != m_lsAreasToClean.end(); ++itAreaId) {
        cleanAreaNow(*itAreaId);
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
    //m_lsObjsToAdd.push_back(pair<GameObject*,uint>(obj, uiAreaId));
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    if(itArea != m_mWorld.end()) {
        //itArea->second.m_mCurArea[obj->getId()] = obj;
        //obj->callBack(getId(), &uiAreaId, PWE_ON_ADDED_TO_AREA);
        obj->setFlag(PWE_INFORM_OBJ, true); //So the object is informed of its new place
        itArea->second.m_pOctree->add(obj);
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
        obj->setFlag(PWE_INFORM_OBJ, true);
        itArea->second.m_pOctree->remove(uiObjId);
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
    m_mWorld[uiAreaId].m_pOctree = new FluidOctreeRoot(getId(), uiAreaId, Box(-1.f,-1.f,-1.f,2.f,2.f,2.f));
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
        m_pCurArea = &(it->second);
        m_uiCurArea = m_uiNextArea;
        m_uiEffectiveArea = m_uiNextArea;
        re->clearScreen();
        re->moveScreenTo(Point());

        map<uint, Listener*>::iterator iter;
		for(iter = m_pCurArea->m_mAreaChangeListeners.begin();
			iter != m_pCurArea->m_mAreaChangeListeners.end();
			++iter ) {
			int status = iter->second->callBack(ID_MODULAR_ENGINE, NULL, PWE_ON_AREA_SWITCH);
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
PartitionedWorldEngine::writeArea(uint uiAreaId, boost::property_tree::ptree &pt, const std::string &keyBase) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    map<uint, GameObject*>::iterator itObj;
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to write nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return;
    }

    //Key-base should already be the area name
    pt.put(keyBase, uiAreaId);

    itArea->second.m_pOctree->write(pt, keyBase);
}


void
PartitionedWorldEngine::readArea(uint uiAreaId, boost::property_tree::ptree &pt, const std::string &keyBase) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    if(itArea == m_mWorld.end()) {
        generateArea(uiAreaId);
        itArea = m_mWorld.find(uiAreaId);   //Used for setting up octree
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
    itArea->second.m_pOctree = new FluidOctreeRoot(getId(), itArea->first, bxBounds, 2.f);
    //itArea->second.m_pOctree->debugPrintBounds();

    //Fill the octree
    for(list<GameObject*>::iterator it = lsObjsToAdd.begin(); it != lsObjsToAdd.end(); ++it) {
        //Old add code
        //addTo(*it, uiAreaId);

        //New add code
        (*it)->setFlag(PWE_INFORM_OBJ, true); //So the object is informed of its new place
        itArea->second.m_pOctree->add(*it);
    }

}

void
PartitionedWorldEngine::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    map<uint, M_Area>::iterator itArea;
    for(itArea = m_mWorld.begin(); itArea != m_mWorld.end(); ++itArea) {
        writeArea(itArea->first, pt, keyBase + "." + itArea->second.m_sName);
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
        setAreaName(id, a.first.data());
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
    case PWE_ON_AREA_SWITCH:
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
    case PWE_ON_AREA_SWITCH:
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
}


void
PartitionedWorldEngine::cleanAreaNow(uint uiAreaId) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaId);
    map<uint, GameObject*>::iterator itObj;
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to clean nonexistent area %d\n", __FILE__, __LINE__, uiAreaId);
        return;
    }

    //Delete the objects from the area
    //TODO: This may be the cause of a segfault.  Do we want to just erase the objects, or erase the octree?
    delete itArea->second.m_pOctree;

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
printf("Before: %d,%d,%d\n",w,h,l);
    //Convert each to a power of two
    int logw = 0, logh = 0, logl = 0;
    while(w >>= 1) { logw++; }
    while(h >>= 1) { logh++; }
    while(l >>= 1) { logl++; }
    w = 1 << (logw + 1);
    h = 1 << (logh + 1);
    l = 1 << (logl + 1);
printf("After: %d,%d,%d\n",w,h,l);

    Point ptCenter = bxCenter(bx);
    bx.x = floor(ptCenter.x) - w / 2;
    bx.y = floor(ptCenter.y) - h / 2;
    bx.z = floor(ptCenter.z) - l / 2;
    bx.w = w;
    bx.h = h;
    bx.l = l;
}
