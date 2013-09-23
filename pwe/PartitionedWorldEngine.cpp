/*
 * Source file for the basic world engine
 */

#include "PartitionedWorldEngine.h"
#include "mge/ModularEngine.h"
#include <list>
using namespace std;

PartitionedWorldEngine *PartitionedWorldEngine::pwe;

PartitionedWorldEngine::PartitionedWorldEngine() {
    m_uiNextID = ID_FIRST_UNUSED;   //The first few ids are reserved for the engines
    m_uiCurArea = m_uiNextArea = m_uiEffectiveArea = 0;
    pe = NULL;
    re = NULL;
    m_pCurArea = NULL;
    m_bFirstRun = true;
    ModularEngine *mge = ModularEngine::get();
    mge->addListener(this, ON_MOUSE_MOVE);
    mge->addListener(this, ON_BUTTON_INPUT);
}

PartitionedWorldEngine::~PartitionedWorldEngine() {
    //Need to free everything
    map<uint, M_Area>::iterator itArea;
    for(itArea = m_mWorld.begin(); itArea != m_mWorld.end(); ++itArea) {
        //TODO: improve efficiency by sticking actual implementation here
        cleanArea(itArea->first);
    }
}

void PartitionedWorldEngine::update(uint time) {
    list<GameObject*> lsHasMoved;

    //Update the physics engine clock
    pe->update(time);

    //Update the world (including non-physics updates)
    for(map<uint, GameObject*>::iterator it = m_pCurArea->m_mCurArea.begin();
            it != m_pCurArea->m_mCurArea.end(); ++it) {

        //Update non-physics bit first
        if(it->second->update(time)) {
            //This object requested that it be removed
            GameObject *obj = it->second;

            //Get the previous iterator, then go back to the next element
            map<uint, GameObject*>::iterator itTemp = (--it)++;

            //Remove the object from the world map, the screen, and memory
            m_pCurArea->m_mCurArea.erase(it);
            re->remove(obj);
            delete obj;

            //Restore the iterator to a valid position in the map
            it = itTemp;
            continue;
        }

        //Update physics.  If it returns true, it has moved and needs collision checks.
        bool bHasMoved = pe->applyPhysics(it->second);

        //Apply joint physics
        for(list<GameObject*>::iterator mv = lsHasMoved.begin();
                mv != lsHasMoved.end(); ++mv) {
            pe->applyPhysics(it->second, (*mv));
        }

        if(bHasMoved || re->screenHasMoved()) {
            re->manageObjOnScreen(it->second);
        }

        if(bHasMoved) {
            lsHasMoved.push_back(it->second);
        }
    }

    //Collision check the remaining objects
    for(list<GameObject*>::iterator mv = lsHasMoved.begin();
            mv != lsHasMoved.end(); ++mv) {
        for(map<uint, GameObject*>::iterator it = m_pCurArea->m_mCurArea.begin();
                it != m_pCurArea->m_mCurArea.end(); ++it) {
            if(it->second->getID() == (*mv)->getID()) {
                break;
            }
            pe->applyPhysics(it->second, *mv);
        }
    }

    //Clear the list of moved objects
    lsHasMoved.clear();

    //If necessary, set the current area (we wanted to finish this update first)
    if(m_uiCurArea != m_uiNextArea) {
        setCurrentArea();
    }
    m_bFirstRun = false;
}

void PartitionedWorldEngine::add(GameObject *obj) {
    //TODO: improve efficiency by sticking actual implementation here
    addTo(obj, m_uiEffectiveArea);
}

void PartitionedWorldEngine::remove(uint id) {
    //TODO: improve efficiency by sticking actual implementation here
    removeFrom(id, m_uiEffectiveArea);
}

/*
 * Partitioned-World specific functions
 */
void PartitionedWorldEngine::generateArea(uint uiAreaId) {
    m_mWorld[uiAreaId] = M_Area();
}

void PartitionedWorldEngine::setCurrentArea(uint uiAreaID) {
    m_uiNextArea = uiAreaID;

    //Call immediately if the first update has not yet occurred
    if(m_bFirstRun) {
        setCurrentArea();
    }
}

void PartitionedWorldEngine::setCurrentArea() {
    map<uint, M_Area>::iterator it = m_mWorld.find(m_uiNextArea);
    if(it != m_mWorld.end()) {
        m_pCurArea = &(it->second);
        m_uiCurArea = m_uiNextArea;
        m_uiEffectiveArea = m_uiNextArea;
        re->clearScreen();
        re->moveScreenTo(Point());
    } else {
        printf("ERROR %s %d: Tried to set current area to a map that doesn't exist\n", __FILE__, __LINE__);
    }
}

void PartitionedWorldEngine::moveObjectToArea(uint uiObjID, uint uiStartAreaID, uint uiEndAreaID) {
    map<uint, M_Area>::iterator itStartArea = m_mWorld.find(uiStartAreaID),
                                itEndArea   = m_mWorld.find(uiEndAreaID);
    map<uint, GameObject*>::iterator itObj;
    if(itStartArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to move object from area that doesn't exist\n", __FILE__, __LINE__);
        return;
    } else if(itEndArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to move object to area that doesn't exist\n", __FILE__, __LINE__);
        return;
    }

    itObj = itStartArea->second.m_mCurArea.find(uiObjID);
    if(itObj != itStartArea->second.m_mCurArea.end()) {
        itEndArea->second.m_mCurArea[itObj->second->getID()] = itObj->second;
        itStartArea->second.m_mCurArea.erase(itObj);
    } else {
        printf("ERROR %s %d: Tried to move nonexistent object\n", __FILE__, __LINE__);
        return;
    }
}

void PartitionedWorldEngine::addTo(GameObject *obj, uint uiAreaID) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaID);
    if(itArea != m_mWorld.end()) {
        itArea->second.m_mCurArea[obj->getID()] = obj;
    } else {
        printf("ERROR %s %d: Tried to add object to nonexistent area\n", __FILE__, __LINE__);
        return;
    }
}

void PartitionedWorldEngine::removeFrom(uint uiObjID, uint uiAreaID) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaID);
    map<uint, GameObject*>::iterator itObj;
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to remove object from nonexistent area\n", __FILE__, __LINE__);
        return;
    }

    itObj = itArea->second.m_mCurArea.find(uiObjID);
    if(itObj != itArea->second.m_mCurArea.end()) {
        itArea->second.m_mCurArea.erase(itObj);
    } else {
        printf("ERROR %s %d: Tried to erase nonexistent object\n", __FILE__, __LINE__);
        return;
    }
}

void PartitionedWorldEngine::cleanArea(uint uiAreaID) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaID);
    map<uint, GameObject*>::iterator itObj;
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to clean nonexistent area\n", __FILE__, __LINE__);
        return;
    }

    //Delete the objects from the area
    for(itObj = itArea->second.m_mCurArea.begin(); itObj != itArea->second.m_mCurArea.end(); ++itObj) {
        delete (itObj->second);
    }

    //Clear the lists of the bad pointers
    itArea->second.m_mCurArea.clear();
    itArea->second.m_mMouseMoveListeners.clear();
    itArea->second.m_mButtonInputListeners.clear();

    //We assume the screen has already been cleared of these objects, which is
    // why you should never call this function on the current area
}

//Event Handler
void PartitionedWorldEngine::addListener(Listener *pListener, EventID id, uint uiAreaID, char* triggerData) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaID);
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to add listener to nonexistent area\n", __FILE__, __LINE__);
        return;
    }

	switch(id) {
	case ON_MOUSE_MOVE:
		itArea->second.m_mMouseMoveListeners[pListener->getID()] = pListener;
		break;
	case ON_BUTTON_INPUT:
		itArea->second.m_mButtonInputListeners[pListener->getID()] = pListener;
		break;
	default:
		printf("Unsupported event handle %d.\n", id);
	}
}

bool PartitionedWorldEngine::removeListener(uint uiListenerID, EventID eventID, uint uiAreaID) {
    map<uint, M_Area>::iterator itArea = m_mWorld.find(uiAreaID);
	map<uint, Listener*>::iterator iter;
    if(itArea == m_mWorld.end()) {
        printf("ERROR %s %d: Tried to remove listener from nonexistent area\n", __FILE__, __LINE__);
        return false;
    }

	switch(eventID) {
	case ON_MOUSE_MOVE:
        iter = itArea->second.m_mMouseMoveListeners.find(uiListenerID);
        if(iter != itArea->second.m_mMouseMoveListeners.end()) {
            itArea->second.m_mMouseMoveListeners.erase(iter);
            return true;
        }
		break;
	case ON_BUTTON_INPUT:
        iter = itArea->second.m_mButtonInputListeners.find(uiListenerID);
        if(iter != itArea->second.m_mButtonInputListeners.end()) {
            itArea->second.m_mButtonInputListeners.erase(iter);
            return true;
        }
		break;
	default:
		printf("Unsupported event handle %d.\n", eventID);
		return false;
	}
	return false;
}

void PartitionedWorldEngine::callBack(uint cID, void *data, EventID id) {
    if(m_pCurArea == NULL) { return; }

    //Pass directly on to the current list of listeners
	map<uint,Listener*>::iterator iter;
	switch(id) {
	case ON_MOUSE_MOVE:
		for( iter = m_pCurArea->m_mMouseMoveListeners.begin();
			iter != m_pCurArea->m_mMouseMoveListeners.end();
			++iter ) {
			iter->second->callBack(ID_MODULAR_ENGINE, data, id);
		}
		break;
	case ON_BUTTON_INPUT:
		for( iter = m_pCurArea->m_mButtonInputListeners.begin();
			iter != m_pCurArea->m_mButtonInputListeners.end();
			++iter ) {
			iter->second->callBack(ID_MODULAR_ENGINE, data, id);
		}
		break;
	default:
		printf("Unsupported event handle %d.\n", id);
	}
}
