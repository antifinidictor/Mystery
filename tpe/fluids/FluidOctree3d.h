#ifndef FLUIDOCTREE3D_H
#define FLUIDOCTREE3D_H

//#include "mge/Octree3d.h"
#include "Vorton.h"
#include "InterpGrid.h"
#include "mge/defs.h"
#include <map>
#include <list>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include "SDL.h"

class GameObject;
class Scheduler;
struct SDL_mutex;

class FluidOctreeNode {
public:
    void print(std::ostream &o, int line, const std::string &msg = "");

    FluidOctreeNode(uint uiEngineId, uint uiAreaId, uint uiLevel, FluidOctreeNode *parent, const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~FluidOctreeNode();

    //Schedules object for appending to the appropriate node and returns true if it can be done
    virtual bool add(GameObject *obj, bool bForce = true);

    //Schedules object for removal from the list if it can be found, but does not delete
    bool remove(uint uiObjId);

    //Schedules object for removal and deletion from the list if it can be found
    bool erase(uint uiObjId);

    //Returns a reference to the appropriate object
    GameObject *find(uint uiObjId);

    //Returns true if the object is empty
    bool empty() { return m_bEmpty; }


    void write(boost::property_tree::ptree &pt, const std::string &keyBase, bool bIsSaveFile = false);

    //WARNING: ONLY SCHEDULER SHOULD CALL
    virtual void update(float fTime);

    void updateAddRemoveErase();

    void onSchedule();

    virtual int tempGetClassId() { return 0; }
    uint getId() { return m_uiEngineId; }
    FluidOctreeNode *getParent() { return m_pParent; }
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
    typedef std::map<uint,GameObject*>::iterator iter_t;
    typedef std::list<GameObject*> objlist_t;       //Used in case a different STL container is faster for these operations
    typedef std::list<GameObject*>::iterator objlist_iter_t;

    void updateContents(float fTime);   //Performs old-school update & collision checks
    void handleChildrenUpdateResults();
    int recursiveScheduleUpdates(Scheduler *s);

    bool addToChildren(GameObject *obj);
    void addNow(GameObject *obj);   //Adds object to this node's contents
    void removeNow(uint uiObjId);   //Removes from this object's contents
    void eraseNow(uint uiObjId);    //Erases from this object's contents

    bool getChildBounds(int iQuadName, const Box &bxMyBounds, Box &bxChildBounds);
    bool childIsLeafNode(const Box &bxChildBounds);
    void updateEmptiness();

    void cleanResults() {
        m_lsDynamicObjs.clear();
        m_lsStaticObjs.clear();
        m_lsObjsLeftQuadrant.clear();
        m_bIsFinished = false;
    }

    //Octree node information
    Box m_bxBounds;                                 //Non-relative bounds.  Fluids may expand, but they don't actually move.
    FluidOctreeNode *m_pParent;
    FluidOctreeNode *m_apChildren[QUAD_NUM_QUADS];
    bool m_bEmpty;
    float m_fMinResolution;

    //Fluid physics
    Vorton m_vrtAggregate;

    //Container information
    uint m_uiEngineId;
    uint m_uiAreaId;
    uint m_uiLevel;
    std::map<uint,GameObject *> m_mContents;
    std::list<GameObject *> m_lsObjsToAdd;
    std::list<uint> m_lsObjsToErase;
    std::list<uint> m_lsObjsToRemove;

    //Locking information: Used to ensure parents execute only when the children are finished
    SDL_mutex *m_mutex;
    SDL_cond  *m_cond;
    bool       m_bIsFinished;

    //Information calculated on each scheduled update event, used by parents in their update event, cleared by parents
    objlist_t m_lsDynamicObjs;      //These objects have moved and must be compared against all objects
    objlist_t m_lsStaticObjs;       //These objects have not moved and must be compared against
    objlist_t m_lsObjsLeftQuadrant; //These objects left their quadrant and need to be added to the next level up
};

class FluidOctreeRoot : public FluidOctreeNode {
public:
    FluidOctreeRoot(uint uiEngineId, uint uiAreaId, const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~FluidOctreeRoot();

    void scheduleUpdates(Scheduler *s) {
        FluidOctreeNode::recursiveScheduleUpdates(s);
        //int updates = FluidOctreeNode::recursiveScheduleUpdates(s);
        //printf("%d updates scheduled\n", updates);
    }

    virtual void update(float fTime);

    virtual int tempGetClassId() { return -1; }

    void debugPrintBounds();

protected:
    typedef Matrix<3,3> Mat33;
    //FluidOctreeRoot *neighbors[NUM_CARDINAL_DIRECTIONS];
    //TimeField field;

    InterpGrid<Vec3f> m_igVelocities;
    InterpGrid<Mat33> m_igJacobians;
};

class FluidOctreeLeaf : public FluidOctreeNode {
public:
    FluidOctreeLeaf(uint uiEngineId, uint uiAreaId, uint uiLevel, FluidOctreeNode *parent, const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~FluidOctreeLeaf();

    //Adding to a leaf node is much simpler than adding to a general node
    virtual bool add(GameObject *obj, bool bForce = false);
    virtual void update(float fTime);
    virtual int tempGetClassId() { return 1; }

protected:
    void updateVortons(float fDeltaTime);

    FluidOctreeRoot *m_pRoot;   //Faster than accessing through the parent
    std::list<Vorton> m_lsVortons;
};


class Scheduler {
public:
    virtual void scheduleUpdate(FluidOctreeNode *node) = 0;
};

class BasicScheduler : public Scheduler {
    static BasicScheduler *m_sInstance;
    float m_fTime;
    bool m_bPaused;

public:
    static BasicScheduler *get(float fTime = -1.f, bool bPaused = false) {
        if(fTime >= 0.f) {
            m_sInstance->m_fTime = fTime;
            m_sInstance->m_bPaused = bPaused;
        }
        return m_sInstance;
    }
    virtual void scheduleUpdate(FluidOctreeNode *node) {
        node->onSchedule();
        if(m_bPaused) {
            node->updateAddRemoveErase();
        } else {
            node->update(m_fTime);
        }
    }
};

#endif // FLUIDOCTREE3D_H
