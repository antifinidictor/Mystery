#include "FluidOctree3d.h"
#include "mge/GameObject.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "SDL.h"    //for the threading stuff
#include <boost/lexical_cast.hpp>

using namespace std;

#define DEFAULT_VORTON_RADIUS 0.1

//Static vars
BasicScheduler *BasicScheduler::m_sInstance = new BasicScheduler();
static SDL_mutex *s_mxRenderEngine = SDL_CreateMutex();


//Debugging printing
void
FluidOctreeNode::print(ostream &o, int line, const std::string &msg) {
#define PRINT_CONTENTS 0
    //Dump the complete contents of this octree node
    SDL_LockMutex(s_mxRenderEngine);
    int levelprint = m_uiLevel * 4 + 10;
    string prefix(levelprint, ' ');
    string lineprefix = "line " + boost::lexical_cast<string>(line) + ";";
    string remprefix(levelprint - lineprefix.length(), ' ');
    o << lineprefix << remprefix << hex << m_uiEngineId << ":" << m_uiAreaId << ":" << m_uiLevel << dec
      //<< "\n" << prefix << "  child{";
      << " child{";
    for(int i = FluidOctreeNode::QUAD_FIRST; i < FluidOctreeNode::QUAD_NUM_QUADS; ++i) {
        if(m_apChildren[i] == NULL) {
            o << "-";
        } else {
            o << i;
        }
    }
#if 1
    //o << "}\n" << prefix << "  cont{";
    o << "}; cont{";
#if PRINT_CONTENTS
    bool first = true;
    for(map<uint,GameObject*>::iterator it = m_mContents.begin(); it != m_mContents.end(); ++it) {
        if(first) {
            first = false;
            o << it->first;
        } else {
            o << ", " << it->first;
        }
    }
#else
    o << m_mContents.size();
#endif
    //o << "}\n" << prefix << "  add{";
    o << "}; add{";
#if PRINT_CONTENTS
    first = true;
    for(list<GameObject*>::iterator it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
        if(first) {
            first = false;
            o << (*it)->getId();
        } else {
            o << ", " << (*it)->getId();
        }
    }
#else
    o << m_lsObjsToAdd.size();
#endif
    //o << "}\n" << prefix << "  rem{";
    o << "}; rem{";
#if PRINT_CONTENTS
    first = true;
    for(list<uint>::iterator it = m_lsObjsToRemove.begin(); it != m_lsObjsToRemove.end(); ++it) {
        if(first) {
            first = false;
            o << (*it);
        } else {
            o << ", " << (*it);
        }
    }
#else
    o << m_lsObjsToRemove.size();
#endif
    //o << "}\n" << prefix << "  erase{";
    o << "}; erase{";
#if PRINT_CONTENTS
    first = true;
    for(list<uint>::iterator it = m_lsObjsToErase.begin(); it != m_lsObjsToErase.end(); ++it) {
        if(first) {
            first = false;
            o << (*it);
        } else {
            o << ", " << (*it);
        }
    }
#else
    o << m_lsObjsToErase.size();
#endif
    //o << "}\n" << prefix << "  dyn{";
    o << "}; dyn{";
#if PRINT_CONTENTS
    first = true;
    for(FluidOctreeNode::objlist_iter_t it = m_lsDynamicObjs.begin(); it != m_lsDynamicObjs.end(); ++it) {
        if(first) {
            first = false;
            o << (*it)->getId();
        } else {
            o << ", " << (*it)->getId();
        }
    }
#else
    o << m_lsDynamicObjs.size();
#endif
    //o << "}\n" << prefix << "  stat{";
    o << "}; stat{";
#if PRINT_CONTENTS
    first = true;
    for(FluidOctreeNode::objlist_iter_t it = m_lsStaticObjs.begin(); it != m_lsStaticObjs.end(); ++it) {
        if(first) {
            first = false;
            o << (*it)->getId();
        } else {
            o << ", " << (*it)->getId();
        }
    }
#else
    o << m_lsStaticObjs.size();
#endif
    //o << "}\n" << prefix << "  left{";
    o << "}; left{";
#if PRINT_CONTENTS
    first = true;
    for(FluidOctreeNode::objlist_iter_t it = m_lsObjsLeftQuadrant.begin(); it != m_lsObjsLeftQuadrant.end(); ++it) {
        if(first) {
            first = false;
            o << (*it)->getId();
        } else {
            o << ", " << (*it)->getId();
        }
    }
#else
    o << m_lsObjsLeftQuadrant.size();
#endif
#endif
    o << "} " << msg << "\n";
    o.flush();
    SDL_UnlockMutex(s_mxRenderEngine);
}



FluidOctreeNode::FluidOctreeNode(uint uiEngineId, uint uiAreaId, uint uiLevel, const Box &bxBounds, float fMinResolution)
    :   m_mutex(SDL_CreateMutex()),
        m_cond(SDL_CreateCond()),
        m_bIsFinished(true)             //A node newly created has no contents, so it has finished updating
{
    SDL_LockMutex(m_mutex);
    m_uiEngineId = uiEngineId;
    m_uiAreaId = uiAreaId;
    m_uiLevel = uiLevel;

    m_bEmpty = true;
    m_bxBounds = bxBounds;
    m_fMinResolution = fMinResolution;

    //Children are allocated as needed when objects are added
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        m_apChildren[q] = NULL;
    }
    SDL_UnlockMutex(m_mutex);
}

FluidOctreeNode::~FluidOctreeNode() {
    SDL_LockMutex(m_mutex); //TODO: Necessary?
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL) {
            delete m_apChildren[q];
            m_apChildren[q] = NULL;
        }
    }
    iter_t it;
    for(it = m_mContents.begin(); it != m_mContents.end(); ++it) {
        it->second->callBack(m_uiEngineId, &m_uiAreaId, PWE_ON_ERASED_FROM_AREA);
        delete it->second;
    }
    m_mContents.clear();

    //Objects that got added
    for(list<GameObject *>::iterator it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
        (*it)->callBack(m_uiEngineId, &m_uiAreaId, PWE_ON_ERASED_FROM_AREA);
        delete (*it);
    }
    m_lsObjsToAdd.clear();

    //Clear all of our other lists
    m_lsObjsToErase.clear();
    m_lsObjsToRemove.clear();
    m_lsDynamicObjs.clear();
    m_lsStaticObjs.clear();
    m_lsObjsLeftQuadrant.clear();

    SDL_UnlockMutex(m_mutex);
    SDL_DestroyMutex(m_mutex);
}

//Adds object to the appropriate list
bool
FluidOctreeNode::add(GameObject *obj, bool bForce) {
    Box bxObjBounds = obj->getPhysicsModel()->getCollisionVolume();
    bool bCanAdd = false;

    char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);
    if(dirs) {
        //It doesn't fit in me, but if forced I will add it to me
        // Only the top-level may be forced to add
        if(bForce) {
            m_lsObjsToAdd.push_back(obj);
            bCanAdd = true;
        }

    } else {
        //Try recursively adding to children
        bCanAdd = addToChildren(obj);

        //We add it to us if none of our children could accept it, but it was within our bounds
        if(!bCanAdd) {
            m_lsObjsToAdd.push_back(obj);
            bCanAdd = true;
        }
    }

    //Update emptiness
    m_bEmpty = m_bEmpty && !bCanAdd;
    return bCanAdd;
}

//Removes from the list but does not call delete
bool
FluidOctreeNode::remove(uint uiObjId) {
    iter_t obj = m_mContents.find(uiObjId);
    if(obj != m_mContents.end()) {
        //m_mContents.erase(obj);
        //updateEmptiness();
        m_lsObjsToRemove.push_back(uiObjId);
        return true;
    }

    //TODO: THIS MAY CAUSE SERIOUS FAILURES!
    // Need to define behavior in the middle of an update.
    //Mid-update: Object might be in one of my transitional lists
    //If the object is in dynamic or static, then it hasn't left the quadrant
    // and should be in contents.
    for(objlist_iter_t it = m_lsObjsLeftQuadrant.begin(); it != m_lsObjsLeftQuadrant.end(); ++it) {
        if((*it)->getId() == uiObjId) {
            //No point in waiting to remove it
            m_lsObjsLeftQuadrant.erase(it);
            return true;
        }
    }

    //If the object is in remove or erase, it hasn't been removed yet and
    // should be in contents. If it is in add, it may still be in another list,
    // but it may not be, so this list must be searched as well.
    for(list<GameObject*>::iterator it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
        if((*it)->getId() == uiObjId) {
            //No point in waiting to erase it
            m_lsObjsToAdd.erase(it);
            return true;
        }
    }

    //Otherwise, search children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL && !m_apChildren[q]->empty()) {
            if(m_apChildren[q]->remove(uiObjId)) {
                //Successfully erased from child
                return true;
            }
        }
    }

    return false;
}

//Removes from the list and calls delete
bool
FluidOctreeNode::erase(uint uiObjId) {
    iter_t obj = m_mContents.find(uiObjId);
    if(obj != m_mContents.end()) {
        //delete obj->second;
        //m_mContents.erase(obj);
        //updateEmptiness();
        m_lsObjsToErase.push_back(uiObjId);
        return true;
    }

    //TODO: THIS MAY CAUSE SERIOUS FAILURES!
    // Need to define behavior in the middle of an update.
    //Mid-update: Object might be in one of my transitional lists
    //If the object is in dynamic or static, then it hasn't left the quadrant
    // and should be in contents.
    for(objlist_iter_t it = m_lsObjsLeftQuadrant.begin(); it != m_lsObjsLeftQuadrant.end(); ++it) {
        if((*it)->getId() == uiObjId) {
            //Already removed, no point in waiting to erase it
            delete *it;
            m_lsObjsLeftQuadrant.erase(it);
            return true;
        }
    }

    //If the object is in remove or erase, it hasn't been removed yet and
    // should be in contents. If it is in add, it may still be in another list,
    // but it may not be, so this list must be searched as well.
    for(list<GameObject*>::iterator it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
        if((*it)->getId() == uiObjId) {
            //Not yet added, no point in removing it
            delete *it;
            m_lsObjsToAdd.erase(it);
            return true;
        }
    }

    //Otherwise, search children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL && !m_apChildren[q]->empty()) {
            if(m_apChildren[q]->erase(uiObjId)) {
                //Child contains this object and it will be erased
                return true;
            }
        }
    }

    return false;
}

//Returns a reference to the appropriate object
GameObject *
FluidOctreeNode::find(uint uiObjId) {
    iter_t itContentObj = m_mContents.find(uiObjId);
    if(itContentObj != m_mContents.end()) {
        return itContentObj->second;
    }

    //Mid-update: Object might be in one of my transitional lists
    //If the object is in dynamic or static, then it hasn't left the quadrant
    // and should be in contents.
    for(objlist_iter_t it = m_lsObjsLeftQuadrant.begin(); it != m_lsObjsLeftQuadrant.end(); ++it) {
        if((*it)->getId() == uiObjId) {
            return *it;
        }
    }

    //If the object is in remove or erase, it hasn't been removed yet and
    // should be in contents. If it is in add, it may still be in another list,
    // but it may not be, so this list must be searched as well.
    for(list<GameObject*>::iterator it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
        if((*it)->getId() == uiObjId) {
            return *it;
        }
    }

#if 0
    //Search other lists
    {
        for(objlist_iter_t it = m_lsStaticObjs.begin(); it != m_lsStaticObjs.end(); ++it) {
            if((*it)->getId() == uiObjId) {
                printf("Obj is static in this list\n");
            }
        }
        for(objlist_iter_t it = m_lsDynamicObjs.begin(); it != m_lsDynamicObjs.end(); ++it) {
            if((*it)->getId() == uiObjId) {
                printf("Obj is dynamic in this list\n");
            }
        }
        for(list<uint>::iterator it = m_lsObjsToRemove.begin(); it != m_lsObjsToRemove.end(); ++it) {
            if((*it) == uiObjId) {
                printf("Obj scheduled for remove\n");
            }
        }
        for(list<uint>::iterator it = m_lsObjsToErase.begin(); it != m_lsObjsToErase.end(); ++it) {
            if((*it) == uiObjId) {
                printf("Obj scheduled for erase\n");
            }
        }
    }
#endif

    //Otherwise, search children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL && !m_apChildren[q]->empty()) {
            GameObject *obj = m_apChildren[q]->find(uiObjId);
            if(obj != NULL) {
                return obj;
            }
        }
    }

    return NULL;
}


void
FluidOctreeNode::write(boost::property_tree::ptree &pt, const std::string &keyBase, bool bIsSaveFile) {
    //Write contents
    for(iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
        GameObject *obj = it->second;

        //Two different kinds of passes: Write-to-world file (editor only) and write-to-save-file
        bool bIsSaveFileObj = obj->getFlag(PWE_SAVE_FILE_OBJ);
        bool bCanWrite = (bIsSaveFile && bIsSaveFileObj) ||
                        (!bIsSaveFile && !bIsSaveFileObj);
        if(bCanWrite) {
            obj->write(pt, keyBase + "." + obj->getClass() + "." + obj->getName());
        }
    }

    //Write children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL && !m_apChildren[q]->empty()) {
            m_apChildren[q]->write(pt, keyBase, bIsSaveFile);
        }
    }
}

void
FluidOctreeNode::update(float fTime) {
    SDL_LockMutex(m_mutex);

    //Update objects that should be added to/removed from/erased from this node
    updateAddRemoveErase();

    //Update internal container elements
    updateContents(fTime);

    //Deal with childrens' update-results
    handleChildrenUpdateResults();  //Children mutex's acquired in here

    updateEmptiness();

    m_bIsFinished = true;   //Reset by cleanResults()
    SDL_CondSignal(m_cond);
    SDL_UnlockMutex(m_mutex);
}

void
FluidOctreeNode::onSchedule() {
    //SDL_LockMutex(m_mutex);
}


void
FluidOctreeNode::updateAddRemoveErase() {
    //Erase queued objects from the container
    for(list<uint>::iterator itObjId = m_lsObjsToErase.begin(); itObjId != m_lsObjsToErase.end(); ++itObjId) {
        eraseNow(*itObjId);
    }
    m_lsObjsToErase.clear();

    //Remove queued objects from the container
    for(list<uint>::iterator itObjId = m_lsObjsToRemove.begin(); itObjId != m_lsObjsToRemove.end(); ++itObjId) {
        removeNow(*itObjId);
    }
    m_lsObjsToRemove.clear();

    //Add queued objects to the container
    for(list<GameObject*>::iterator itObj = m_lsObjsToAdd.begin(); itObj != m_lsObjsToAdd.end(); ++itObj) {
        addNow(*itObj);
    }
    m_lsObjsToAdd.clear();
}

void
FluidOctreeNode::updateContents(float fTime) {
    TimePhysicsEngine *pe = TimePhysicsEngine::get();

    //printf(__FILE__" %d: Octree node %5x updated at time %5d by thread 0x%8x\n", __LINE__, m_uiEngineId, Clock::get()->getTime(), SDL_ThreadID());

    for(iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
        //Update the object's logic
        if(it->second->update(fTime)) {
            //Object requested deletion
            m_lsObjsToErase.push_back(it->first);
            continue;
        }

        //Update physics
        bool bHasMoved = pe->applyPhysics(it->second);

        //If the object has moved, add it to the appropriate list so collision
        //checks can be applied later
        if(bHasMoved) {
            //Object has moved.  We will deal with the case where it can be pushed lower in the tree later
            m_lsDynamicObjs.push_back(it->second);

            //If the object has left the bounds, then some additional collision
            // processing must be done against its neighbors.  Notice that objs
            // that left the quadrant are also referenced as dynamic objects
            Box bxObjBounds = it->second->getPhysicsModel()->getCollisionVolume();
            char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);

            if(dirs) {
                //Leaves bounds in at least one direction
                m_lsObjsToRemove.push_back(it->first);
                m_lsObjsLeftQuadrant.push_back(it->second);
            } else {
                //If the object can be added to a child's list, then it should be removed
                if(addToChildren(it->second)) {
                    m_lsObjsToRemove.push_back(it->first);
                }
            }
        } else {
            //All dynamic objects need to be checked against it
            // (all children's, my own, my parents)
            m_lsStaticObjs.push_back(it->second);

            //Collision check against my dynamic objs
            for(objlist_iter_t mv = m_lsDynamicObjs.begin(); mv != m_lsDynamicObjs.end(); ++mv) {
                pe->applyPhysics(it->second, (*mv));
            }
        }
    }

    //Collision check the dynamic objects against contents they had not yet been checked against
    for(objlist_iter_t mv = m_lsDynamicObjs.begin(); mv != m_lsDynamicObjs.end(); ++mv) {

        for(iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
            if(it->second->getId() == (*mv)->getId()) {
                break;
            }
            pe->applyPhysics(it->second, *mv);
        }
    }
}

void
FluidOctreeNode::handleChildrenUpdateResults() {
    TimePhysicsEngine *pe = TimePhysicsEngine::get();
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL) {
        /*
            if(m_apChildren[q]->empty()) {
                m_apChildren[q]->updateAddRemoveErase();
                continue;
            }
        */

            //The while loop ensures the state does not change as soon as the thread awakens.
            //Such a state change should be impossible, but it is probably better form to
            //include the while loop.
            SDL_LockMutex(m_apChildren[q]->m_mutex);
            while(!m_apChildren[q]->m_bIsFinished) {
                SDL_CondWait(m_apChildren[q]->m_cond, m_apChildren[q]->m_mutex);
            }

            //To get here, the child must have already been updated
            //Perform collision checks on relevant lists
            //Dynamic objects from child must be checked against my static objects
            pe->applyCollisionPhysics(m_apChildren[q]->m_lsDynamicObjs, m_lsStaticObjs);

            //Static objects from child must be checked against my dynamic objects
            pe->applyCollisionPhysics(m_apChildren[q]->m_lsStaticObjs, m_lsDynamicObjs);

            //Dynamic objects from child must be checked against my dynamic objects
            pe->applyCollisionPhysics(m_apChildren[q]->m_lsDynamicObjs, m_lsDynamicObjs);

            //Objects that left the child's quadrant need to be checked against
            // the other children
            for(int q2 = q + 1; q2 < QUAD_NUM_QUADS; ++q2) {
                if(m_apChildren[q2] != NULL) {
                    pe->applyCollisionPhysics(
                        m_apChildren[q]->m_lsObjsLeftQuadrant,
                        m_apChildren[q]->m_lsDynamicObjs
                    );
                    pe->applyCollisionPhysics(
                        m_apChildren[q]->m_lsObjsLeftQuadrant,
                        m_apChildren[q]->m_lsStaticObjs
                    );
                }
            }

            //Augment my lists by children's lists
            //Objects that left the child's quadrant need to be added to other lists
            for(objlist_iter_t itObjLeftChild = m_apChildren[q]->m_lsObjsLeftQuadrant.begin();
                    itObjLeftChild != m_apChildren[q]->m_lsObjsLeftQuadrant.end();
                    ++itObjLeftChild)
            {

                Box bxObjBounds = (*itObjLeftChild)->getPhysicsModel()->getCollisionVolume();
                char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);

                if(dirs) {
                    //Object left my quadrant too
                    m_lsObjsLeftQuadrant.push_back(*itObjLeftChild);
                } else {
                    //Object is in my quadrant, so add it
                    m_lsObjsToAdd.push_back(*itObjLeftChild);
                }
            }

            //Copy child lists into my lists
            m_lsDynamicObjs.insert(
                m_lsDynamicObjs.end(),
                m_apChildren[q]->m_lsDynamicObjs.begin(),
                m_apChildren[q]->m_lsDynamicObjs.end()
            );

            m_lsStaticObjs.insert(
                m_lsStaticObjs.end(),
                m_apChildren[q]->m_lsStaticObjs.begin(),
                m_apChildren[q]->m_lsStaticObjs.end()
            );

            //Clear child lists
            m_apChildren[q]->cleanResults();

            //The child will unlock it
            SDL_UnlockMutex(m_apChildren[q]->m_mutex);
        }
    }
}

bool
FluidOctreeNode::addToChildren(GameObject *obj) {
    //Try adding this object to my children.  Note that they won't actually get
    // added to the contents here, only scheduled for addition.
    Box bxObjBounds = obj->getPhysicsModel()->getCollisionVolume();

#ifdef DEBUG_OCTREE
string spaces(m_uiLevel,'\t');
printf("%sInserting obj %d @ node %x (level %d) (%.1f,%.1f,%.1f; %.1f,%.1f,%.1f) vs (%.1f,%.1f,%.1f; %.1f,%.1f,%.1f)\n",
    spaces.c_str(), obj->getId(), m_uiEngineId, m_uiLevel,
    m_bxBounds.x, m_bxBounds.y, m_bxBounds.z,
    m_bxBounds.w, m_bxBounds.h, m_bxBounds.l,
    bxObjBounds.x, bxObjBounds.y, bxObjBounds.z,
    bxObjBounds.w, bxObjBounds.h, bxObjBounds.l
);
#endif

    bool bSomeChildCanAdd = false;
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        //Two cases: Child exists, and child does not exist.
        //If the child does not exist we may want to create it.
        if(m_apChildren[q] == NULL) {
            //See if we want to make a new child (can create child bounds & obj is inside bounds)
            Box bxChildBounds;
            //If we could make a valid child here and it would be inside this child
            if(getChildBounds(q, m_bxBounds, bxChildBounds) && bxOutOfBounds(bxObjBounds, bxChildBounds) == 0) {
                //Create a new child octree here
                uint uiNewLevel = m_uiLevel + 1;
                uint uiNewEngineId = (q << (uiNewLevel * 4)) | m_uiEngineId;
                if(childIsLeafNode(bxChildBounds)) {
                    //Leaf octree node
                    m_apChildren[q] = new FluidOctreeLeaf(
                        uiNewEngineId,
                        m_uiAreaId,
                        uiNewLevel,
                        bxChildBounds,
                        m_fMinResolution
                    );

                } else {
                    //Generic octree node
                    m_apChildren[q] = new FluidOctreeNode(
                        uiNewEngineId,
                        m_uiAreaId,
                        uiNewLevel,
                        bxChildBounds,
                        m_fMinResolution
                    );
                }

                //Add to child
                bSomeChildCanAdd = m_apChildren[q]->add(obj, false);
                break;
            }
        } else if(m_apChildren[q]->add(obj, false)) {
            //Child does exist
            bSomeChildCanAdd = true;
            break;
        }
    }
    return bSomeChildCanAdd;
}


void
FluidOctreeNode::addNow(GameObject *obj) {
    m_mContents[obj->getId()] = obj;

    //TODO: Is there a better way to do this?
    if(obj->getFlag(PWE_INFORM_OBJ_ADD)) {
        obj->setFlag(PWE_INFORM_OBJ_ADD, false);
        obj->callBack(m_uiEngineId, &m_uiAreaId, PWE_ON_ADDED_TO_AREA);
    }
    SDL_LockMutex(s_mxRenderEngine);
    D3RE::get()->manageObjOnScreen(obj);
    SDL_UnlockMutex(s_mxRenderEngine);
}

void
FluidOctreeNode::removeNow(uint uiObjId) {
    iter_t itFoundObj = m_mContents.find(uiObjId);
    if(itFoundObj != m_mContents.end()) {
        //TODO: Is there a better way to do this?
        if(itFoundObj->second->getFlag(PWE_INFORM_OBJ_REMOVE)) {
            itFoundObj->second->setFlag(PWE_INFORM_OBJ_REMOVE, false);
            //itFoundObj->second->callBack(m_uiEngineId, &m_uiAreaId, PWE_ON_REMOVED_FROM_AREA);
            SDL_LockMutex(s_mxRenderEngine);
            D3RE::get()->remove(itFoundObj->second);
            SDL_UnlockMutex(s_mxRenderEngine);
        }

        //Object exists, remove it
        m_mContents.erase(itFoundObj);
    } else {
        printf(__FILE__" %d ERROR: Failed to remove object %d from octree; obj does not exist\n", __LINE__, uiObjId);
    }
}

void
FluidOctreeNode::eraseNow(uint uiObjId) {
    //Does the object even exist in this node?
    iter_t itFoundObj = m_mContents.find(uiObjId);
    if(itFoundObj != m_mContents.end()) {
        SDL_LockMutex(s_mxRenderEngine);
        D3RE::get()->remove(itFoundObj->second);
        SDL_UnlockMutex(s_mxRenderEngine);

        //TODO: Always inform objects of imminent erasure
        itFoundObj->second->callBack(m_uiEngineId, &m_uiAreaId, PWE_ON_ERASED_FROM_AREA);

        //Object exists, delete it
        delete itFoundObj->second;
        m_mContents.erase(itFoundObj);
    } else {
        printf(__FILE__" %d ERROR: Failed to erase object %d from octree; obj does not exist\n", __LINE__, uiObjId);
    }
}

bool
FluidOctreeNode::getChildBounds(int iQuadName, const Box &bxMyBounds, Box &bxChildBounds) {
    //Half-widths
    float hw = bxMyBounds.w / 2;
    float hh = bxMyBounds.h / 2;
    float hl = bxMyBounds.l / 2;

    if(hw < m_fMinResolution && hh < m_fMinResolution && hl < m_fMinResolution) {
        //The parent of this 'child' is really a leaf node
        return false;
    }

    if((iQuadName & QUAD_X_MASK) != 0) {  //negative
        bxChildBounds.x = bxMyBounds.x;
        bxChildBounds.w = (hw < m_fMinResolution) ? bxMyBounds.w : hw;
    } else {                            //positive
        if(hw < m_fMinResolution) {
            //Invalid child: Positive
            return false;
        } else {
            bxChildBounds.x = bxMyBounds.x + hw;
            bxChildBounds.w = hw;
        }
    }

    if((iQuadName & QUAD_Y_MASK) != 0) {  //negative
        bxChildBounds.y = bxMyBounds.y;
        bxChildBounds.h = (hh < m_fMinResolution) ? bxMyBounds.h : hh;
    } else {                            //positive
        if(hh < m_fMinResolution) {
            //Invalid child: Positive
            return false;
        } else {
            bxChildBounds.y = bxMyBounds.y + hh;
            bxChildBounds.h = hh;
        }
    }

    if((iQuadName & QUAD_Z_MASK) != 0) {  //negative
        bxChildBounds.z = bxMyBounds.z;
        bxChildBounds.l = (hl < m_fMinResolution) ? bxMyBounds.l : hl;
    } else {                            //positive
        if(hl < m_fMinResolution) {
            //Invalid child: Positive
            return false;
        } else {
            bxChildBounds.z = bxMyBounds.z + hl;
            bxChildBounds.l = hl;
        }
    }

    //If we make it here, the child is valid
    return true;
}


bool
FluidOctreeNode::childIsLeafNode(const Box &bxChildBounds) {
    float hw = bxChildBounds.w / 2;
    float hh = bxChildBounds.h / 2;
    float hl = bxChildBounds.l / 2;
    return (hw < m_fMinResolution && hh < m_fMinResolution && hl < m_fMinResolution);
}

void
FluidOctreeNode::updateEmptiness() {
    //Emptiness has to do with both actual empty status and potential empty status
    m_bEmpty = m_mContents.size() == 0 &&
               m_lsObjsToAdd.size() == 0 &&
               m_lsObjsLeftQuadrant.size() == 0;
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL) {
            //Only empty if we have nothing and children are empty too
            m_bEmpty = m_bEmpty && m_apChildren[q]->empty();
        }
    }
}


/*
 * FluidOctreeRoot
 */

FluidOctreeRoot::FluidOctreeRoot(uint uiEngineId, uint uiAreaId, const Box &bxBounds, float fMinResolution)
    : FluidOctreeNode(uiEngineId, uiAreaId, 0, bxBounds, fMinResolution)
{
}

FluidOctreeRoot::~FluidOctreeRoot() {
}

void
FluidOctreeRoot::update(float fTime) {
    //Perform a standard update
    FluidOctreeNode::update(fTime);

    //Let the screen know about objects that moved
    SDL_LockMutex(s_mxRenderEngine);
    D3RE *re = D3RE::get();
    for(objlist_iter_t it = m_lsDynamicObjs.begin(); it != m_lsDynamicObjs.end(); ++it) {
        re->manageObjOnScreen(*it);
    }

    //If the screen moved, let it know about objects that didn't move
    if(re->screenHasMoved()) {
        for(objlist_iter_t it = m_lsStaticObjs.begin(); it != m_lsStaticObjs.end(); ++it) {
            re->manageObjOnScreen(*it);
        }
    }
    SDL_UnlockMutex(s_mxRenderEngine);

    //If any objects left the root, they should be returned to the root
    for(objlist_iter_t it = m_lsObjsLeftQuadrant.begin(); it != m_lsObjsLeftQuadrant.end(); ++it) {
        //For now, just add them to me
        m_mContents[(*it)->getId()] = *it;
    }

    //Clean the lists in preparation for the next update
    cleanResults();
}

struct TempDebugInfo {
    Box bounds;
    int level;
    uint id;
    TempDebugInfo(uint i, int l, Box bx) {
        bounds = bx;
        level = l;
        id = i;
    }
};

void
FluidOctreeRoot::debugPrintBounds() {
    printf("Size of each node: Root = %d, node = %d, leaf = %d\n", sizeof(FluidOctreeRoot), sizeof(FluidOctreeNode), sizeof(FluidOctreeLeaf));
    printf("Base bounds: (%f,%f,%f; %f,%f,%f)\n",
        m_bxBounds.x, m_bxBounds.y, m_bxBounds.z,
        m_bxBounds.x + m_bxBounds.w, m_bxBounds.y + m_bxBounds.l, m_bxBounds.z + m_bxBounds.h
    );
    list<TempDebugInfo> boxes;  //queue of boxes to have their children printed
    boxes.push_back(TempDebugInfo(0,0, m_bxBounds));
    do {
        TempDebugInfo info = boxes.front();
        int level = info.level + 1;
        boxes.pop_front();

        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            Box bxChild;
            uint id = (q << (level * 4)) | info.id;
            string s(level, '\t');

            if(getChildBounds(q, info.bounds, bxChild)) {
                printf("%sBounds for %x (child %x, lvl %d): (%f,%f,%f; %f,%f,%f)\n", s.c_str(), id, q, level,
                    bxChild.x, bxChild.y, bxChild.z,
                    bxChild.x + bxChild.w, bxChild.y + bxChild.l, bxChild.z + bxChild.h
                );

                if(!childIsLeafNode(bxChild)) {
                    boxes.push_back(TempDebugInfo(id, level, bxChild));
                }
            } else {
                printf("%sNode %x (child %x, lvl %d) is invalid\n",s.c_str(), id, q, level);
            }
        }
    } while(boxes.size() > 0);
    //For every level thereafter
}

/*
 * FluidOctreeLeaf
 */

FluidOctreeLeaf::FluidOctreeLeaf(uint uiEngineId, uint uiAreaId, uint uiLevel, const Box &bxBounds, float fMinResolution)
    : FluidOctreeNode(uiEngineId, uiAreaId, uiLevel, bxBounds, fMinResolution)
{
}

FluidOctreeLeaf::~FluidOctreeLeaf() {
}

bool
FluidOctreeLeaf::add(GameObject *obj, bool bForce) {
    Box bxObjBounds = obj->getPhysicsModel()->getCollisionVolume();
    bool bCanAdd = bForce;
    char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);
    if(dirs) {
        //It doesn't fit in me, but if forced I will add it to me
        // Only the top-level can be forced to add
        if(bForce) {
            m_lsObjsToAdd.push_back(obj);
        }

    } else {
        //Since it was in our bounds, we could add it to our list
        m_lsObjsToAdd.push_back(obj);
        bCanAdd = true;
    }
    m_bEmpty = m_bEmpty && !bCanAdd;
#ifdef DEBUG_OCTREE
if(bCanAdd) {
string spaces(m_uiLevel,'\t');
printf("%sInserting obj %d @ node %x (level %d)\n", spaces.c_str(), obj->getId(), m_uiEngineId, m_uiLevel);
}
#endif
    return bCanAdd;
}
