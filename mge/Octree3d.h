#ifndef OCTREE3D_H
#define OCTREE3D_H

#include "mge/defs.h"
#include <map>
#include <list>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include "SDL.h"

template<class Object>
class Octree3dNode {
public:

    Octree3dNode(uint uiNodeId, uint uiLevel, const Box &bxBounds, float fMinResolution = 1.f)
        :   m_bxBounds(bxBounds),
            m_bEmpty(true),
            m_fMinResolution(fMinResolution),
            m_uiNodeId(uiNodeId),
            m_uiLevel(uiLevel),
            m_mutex(SDL_CreateMutex()),
            m_cond(SDL_CreateCond()),
            m_bIsFinished(true)             //A node newly created has no contents, so it has finished updating
    {
        SDL_LockMutex(m_mutex);

        //Children are allocated as needed when objects are added
        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            m_apChildren[q] = NULL;
        }
        SDL_UnlockMutex(m_mutex);
    }

    virtual ~Octree3dNode() {
        SDL_LockMutex(m_mutex); //TODO: Necessary?
        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            if(m_apChildren[q] != NULL) {
                delete m_apChildren[q];
                m_apChildren[q] = NULL;
            }
        }
        objmap_iter_t it;
        for(it = m_mContents.begin(); it != m_mContents.end(); ++it) {
            onErase(it->second);
            delete it->second;
        }
        m_mContents.clear();

        //Objects that got added
        for(objlist_iter_t it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
            onErase(*it);
            delete (*it);
        }
        m_lsObjsToAdd.clear();

        //Clear all of our other lists
        m_lsObjsToErase.clear();
        m_lsObjsToRemove.clear();
        m_lsObjsLeftQuadrant.clear();

        SDL_UnlockMutex(m_mutex);
        SDL_DestroyMutex(m_mutex);
    }

    //Schedules object for appending to the appropriate node and returns true if it can be done
    virtual bool add(Object *obj, bool bForce = true) {
        Box bxObjBounds = obj->getBounds();
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

    //Schedules object for removal from the list if it can be found, but does not delete
    bool remove(uint uiObjId) {
        objmap_iter_t obj = m_mContents.find(uiObjId);
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
        for(objlist_iter_t it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
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

    //Schedules object for removal and deletion from the list if it can be found
    bool erase(uint uiObjId) {
        objmap_iter_t obj = m_mContents.find(uiObjId);
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
        for(objlist_iter_t it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
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
    Object *find(uint uiObjId) {
        objmap_iter_t itContentObj = m_mContents.find(uiObjId);
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
        for(objlist_iter_t it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
            if((*it)->getId() == uiObjId) {
                return *it;
            }
        }

        //Otherwise, search children
        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            if(m_apChildren[q] != NULL && !m_apChildren[q]->empty()) {
                Object *obj = m_apChildren[q]->find(uiObjId);
                if(obj != NULL) {
                    return obj;
                }
            }
        }

        return NULL;
    }

    //Returns true if the object is empty
    bool empty() { return m_bEmpty; }

    //WARNING: ONLY SCHEDULER SHOULD CALL THESE
    virtual void update(float fTime) {
        SDL_LockMutex(m_mutex);

        //Update objects that should be added to/removed from/erased from this node
        updateAddRemoveErase();

        //Update internal container elements
        updateContents(fTime);

        //Deal with childrens' update-results
        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            if(m_apChildren[q] != NULL) {
                //Handle the child's update as soon as it is ready.
                //The while loop ensures the state does not change as soon as the thread awakens.
                //Such a state change should be impossible, but it is probably better form to
                //include the while loop.
                SDL_LockMutex(m_apChildren[q]->m_mutex);
                while(!m_apChildren[q]->m_bIsFinished) {
                    SDL_CondWait(m_apChildren[q]->m_cond, m_apChildren[q]->m_mutex);
                }

                //Handle child's results
                handleChildUpdateResults(m_apChildren[q], q);

                //Augment my lists by children's lists
                //Objects that left the child's quadrant need to be added to other lists
                for(objlist_iter_t itObjLeftChild = m_apChildren[q]->m_lsObjsLeftQuadrant.begin();
                        itObjLeftChild != m_apChildren[q]->m_lsObjsLeftQuadrant.end();
                        ++itObjLeftChild)
                {

                    Box bxObjBounds = (*itObjLeftChild)->getBounds();
                    char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);

                    if(dirs) {
                        //Object left my quadrant too
                        m_lsObjsLeftQuadrant.push_back(*itObjLeftChild);
                    } else {
                        //Object is in my quadrant, so add it
                        m_lsObjsToAdd.push_back(*itObjLeftChild);
                    }
                }

                //Clear child lists
                m_apChildren[q]->cleanResults();

                //Allow access to this child
                SDL_UnlockMutex(m_apChildren[q]->m_mutex);
            }
        }

        updateEmptiness();

        m_bIsFinished = true;   //Reset by cleanResults()
        SDL_CondSignal(m_cond);
        SDL_UnlockMutex(m_mutex);
    }

    void updateAddRemoveErase() {
        //Erase queued objects from the container
        for(idlist_iter_t itObjId = m_lsObjsToErase.begin(); itObjId != m_lsObjsToErase.end(); ++itObjId) {
            eraseNow(*itObjId);
        }
        m_lsObjsToErase.clear();

        //Remove queued objects from the container
        for(idlist_iter_t itObjId = m_lsObjsToRemove.begin(); itObjId != m_lsObjsToRemove.end(); ++itObjId) {
            removeNow(*itObjId);
        }
        m_lsObjsToRemove.clear();

        //Add queued objects to the container
        for(objlist_iter_t itObj = m_lsObjsToAdd.begin(); itObj != m_lsObjsToAdd.end(); ++itObj) {
            addNow(*itObj);
        }
        m_lsObjsToAdd.clear();
    }

    template<class Scheduler>
    int scheduleUpdates(Scheduler *scheduler) {
        int numUpdatesScheduled = 1;
        //Updates are scheduled as a stack, with children getting updated first
        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            if(m_apChildren[q] != NULL/* && !m_apChildren[q]->empty()*/) {
                numUpdatesScheduled += m_apChildren[q]->scheduleUpdates(scheduler);    //The node should schedule itself
            }
        }
        /*
        printf("Scheduled %4x:%x {%2.2f,%2.2f,%2.2f;%2.2f,%2.2f,%2.2f}\n",
            m_uiNodeId, m_uiLevel,
            m_bxBounds.x, m_bxBounds.y, m_bxBounds.z,
            m_bxBounds.w, m_bxBounds.h, m_bxBounds.l);
        */
        onSchedule();
        scheduler->scheduleUpdate(this);
        return numUpdatesScheduled;
    }

    //Getters/setters
    uint getId() { return m_uiNodeId; }
    uint getLevel() { return m_uiLevel; }

    //Debugging functions
    struct DebugInfo {
        Box bounds;
        int level;
        uint id;
        DebugInfo(uint i, int l, Box bx) {
            bounds = bx;
            level = l;
            id = i;
        }
    };

    void debugPrintBounds() {
        printf("Base bounds: (%f,%f,%f; %f,%f,%f)\n",
            m_bxBounds.x, m_bxBounds.y, m_bxBounds.z,
            m_bxBounds.x + m_bxBounds.w, m_bxBounds.y + m_bxBounds.l, m_bxBounds.z + m_bxBounds.h
        );
        std::list<DebugInfo> boxes;  //queue of boxes to have their children printed
        boxes.push_back(DebugInfo(0,0, m_bxBounds));
        do {
            DebugInfo info = boxes.front();
            int level = info.level + 1;
            boxes.pop_front();

            for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
                Box bxChild;
                uint id = (q << (level * 4)) | info.id;
                std::string s(level, '\t');

                if(getChildBounds(q, info.bounds, bxChild)) {
                    printf("%sBounds for %x (child %x, lvl %d): (%f,%f,%f; %f,%f,%f)\n", s.c_str(), id, q, level,
                        bxChild.x, bxChild.y, bxChild.z,
                        bxChild.x + bxChild.w, bxChild.y + bxChild.l, bxChild.z + bxChild.h
                    );

                    if(!childIsLeafNode(bxChild)) {
                        boxes.push_back(DebugInfo(id, level, bxChild));
                    }
                } else {
                    printf("%sNode %x (child %x, lvl %d) is invalid\n",s.c_str(), id, q, level);
                }
            }
        } while(boxes.size() > 0);
        //For every level thereafter
    }

    void print(std::ostream &o, int line, const std::string &msg = "") {
    #define PRINT_CONTENTS 0
        //Dump the complete contents of this Octree3d node
        SDL_LockMutex(m_mutex);
        int levelprint = m_uiLevel * 4 + 10;
        std::string prefix(levelprint, ' ');
        std::string lineprefix = "line " + boost::lexical_cast<std::string>(line) + ";";
        std::string remprefix(levelprint - lineprefix.length(), ' ');
        o << lineprefix << remprefix << std::hex << m_uiNodeId << ":" << m_uiLevel << std::dec
          //<< "\n" << prefix << "  child{";
          << " child{";
        for(int i = Octree3dNode<Object>::QUAD_FIRST; i < Octree3dNode<Object>::QUAD_NUM_QUADS; ++i) {
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
        for(map<uint,Object*>::iterator it = m_mContents.begin(); it != m_mContents.end(); ++it) {
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
        for(objlist_iter_t it = m_lsObjsToAdd.begin(); it != m_lsObjsToAdd.end(); ++it) {
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
        for(idlist_iter_t it = m_lsObjsToRemove.begin(); it != m_lsObjsToRemove.end(); ++it) {
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
        for(idlist_iter_t it = m_lsObjsToErase.begin(); it != m_lsObjsToErase.end(); ++it) {
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
        //o << "}\n" << prefix << "  left{";
        o << "}; left{";
    #if PRINT_CONTENTS
        first = true;
        for(Octree3dNode<Object>::objlist_iter_t it = m_lsObjsLeftQuadrant.begin(); it != m_lsObjsLeftQuadrant.end(); ++it) {
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
        SDL_UnlockMutex(m_mutex);
    }

protected:
    enum QuadrantNames {
        QUAD_FIRST = 0,                     //Used for iterating
        QUAD_POSX_POSY_POSZ = QUAD_FIRST,
        QUAD_POSX_POSY_NEGZ,
        QUAD_POSX_NEGY_POSZ,
        QUAD_POSX_NEGY_NEGZ,
        QUAD_NEGX_POSY_POSZ,
        QUAD_NEGX_POSY_NEGZ,
        QUAD_NEGX_NEGY_POSZ,
        QUAD_NEGX_NEGY_NEGZ,
        QUAD_NUM_QUADS      //Used for iterating
    };
    #define QUAD_X_MASK 0x4
    #define QUAD_Y_MASK 0x2
    #define QUAD_Z_MASK 0x1
    typedef std::map<uint,Object*> objmap_t;
    typedef typename objmap_t::iterator objmap_iter_t;
    typedef std::list<Object*> objlist_t;       //Used in case a different STL container is faster for these operations
    typedef typename objlist_t::iterator objlist_iter_t;
    typedef std::list<uint> idlist_t;
    typedef typename idlist_t::iterator idlist_iter_t;

    //Functions that probably should be implemented by the base class
    virtual void updateContents(float fTime) {}
    virtual void handleChildUpdateResults(Octree3dNode<Object> *child, int q) {}
    virtual void onAdd(Object *obj) {}
    virtual void onRemove(Object *obj) {}
    virtual void onErase(Object *obj)  {}
    virtual void onSchedule() {}
    virtual void cleanResults() {
        m_lsObjsLeftQuadrant.clear();
        m_bIsFinished = false;
    }
    virtual Octree3dNode<Object> *createChild(uint childId, const Box &bxBounds) const {
        uint uiNewLevel = m_uiLevel + 1;
        uint uiNewNodeId = (childId << (uiNewLevel * 4)) | m_uiNodeId;
        Octree3dNode<Object> *node = new Octree3dNode<Object>(
            uiNewNodeId,
            uiNewLevel,
            bxBounds,
            m_fMinResolution
        );
        return node;
    }

    bool addToChildren(Object *obj) {
        //Try adding this object to my children.  Note that they won't actually get
        // added to the contents here, only scheduled for addition.
        Box bxObjBounds = obj->getBounds();

#ifdef DEBUG_Octree3d
string spaces(m_uiLevel,'\t');
printf("%sInserting obj %d @ node %x (level %d) (%.1f,%.1f,%.1f; %.1f,%.1f,%.1f) vs (%.1f,%.1f,%.1f; %.1f,%.1f,%.1f)\n",
    spaces.c_str(), obj->getId(), m_uiNodeId, m_uiLevel,
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
                    //Create a new child Octree3d here
                    m_apChildren[q] = createChild(q, bxChildBounds);

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

    void addNow(Object *obj) {
        m_mContents[obj->getId()] = obj;

        //Allow subclass to perform some action
        onAdd(obj);
    }

    void removeNow(uint uiObjId) {
        objmap_iter_t itFoundObj = m_mContents.find(uiObjId);
        if(itFoundObj != m_mContents.end()) {
            //Allow subclass to perform some action
            onRemove(itFoundObj->second);

            //Object exists, remove it
            m_mContents.erase(itFoundObj);
        } else {
            printf(__FILE__" %d ERROR: Failed to remove object %d from Octree3d; obj does not exist\n", __LINE__, uiObjId);
        }
    }

    void eraseNow(uint uiObjId) {
        //Does the object even exist in this node?
        objmap_iter_t itFoundObj = m_mContents.find(uiObjId);
        if(itFoundObj != m_mContents.end()) {
            //Allow subclass to perform some action
            onErase(itFoundObj->second);

            //Object exists, delete it
            delete itFoundObj->second;
            m_mContents.erase(itFoundObj);
        } else {
            printf(__FILE__" %d ERROR: Failed to erase object %d from Octree3d; obj does not exist\n", __LINE__, uiObjId);
        }
    }

    bool getChildBounds(int iQuadName, const Box &bxMyBounds, Box &bxChildBounds) const {
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

    bool childIsLeafNode(const Box &bxChildBounds) const {
        float hw = bxChildBounds.w / 2;
        float hh = bxChildBounds.h / 2;
        float hl = bxChildBounds.l / 2;
        return (hw < m_fMinResolution && hh < m_fMinResolution && hl < m_fMinResolution);
    }

    void updateEmptiness() {
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

    //Octree3d node information
    Box m_bxBounds;                                 //Non-relative bounds.  Fluids may expand, but they don't actually move.
    //Octree3dNode *m_pParent;
    Octree3dNode *m_apChildren[QUAD_NUM_QUADS];
    bool m_bEmpty;
    float m_fMinResolution;

    //Container information
    uint m_uiNodeId;
    uint m_uiLevel;
    objmap_t  m_mContents;
    objlist_t m_lsObjsToAdd;
    idlist_t  m_lsObjsToErase;
    idlist_t  m_lsObjsToRemove;

    //Locking information: Used to ensure parents execute only when the children are finished
    SDL_mutex *m_mutex;
    SDL_cond  *m_cond;
    bool       m_bIsFinished;

    //Information calculated on each scheduled update event, used by parents in their update event, cleared by parents
    objlist_t m_lsObjsLeftQuadrant; //These objects left their quadrant and need to be added to the next level up
};

#if 0
//Schedulers are expected to implement this function for their particular type of octree node
class Scheduler {
public:
    virtual void scheduleUpdate(Node *node) = 0;
};
#endif

#endif // OctREE3D_H
