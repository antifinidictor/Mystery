#ifndef FLUIDOCTREE3D_H
#define FLUIDOCTREE3D_H

//#include "mge/Octree3d.h"
#include "Vorton.h"
#include "mge/defs.h"
#include <map>
#include <list>

class GameObject;
class Scheduler;
struct SDL_mutex;

class FluidOctreeNode {
public:
    FluidOctreeNode(uint uiEngineId, uint uiAreaId, uint uiLevel, const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~FluidOctreeNode();

    //Schedules object for appending to the appropriate node and returns true if it can be done
    virtual bool add(GameObject *obj, bool bForce = true);

    //Schedules object for removal from the list if it can be found, but does not delete
    bool remove(uint uiObjId);

    //Schedules object for removal and deletion from the list if it can be found
    bool erase(uint uiObjId);

    //Returns a reference to the appropriate object
    GameObject *find(uint uiObjId);

    //WARNING: ONLY SCHEDULER SHOULD CALL
    void update(float fTime);

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
    void recursiveScheduleUpdates(Scheduler *s);

    bool addToChildren(GameObject *obj);
    void addNow(GameObject *obj);   //Adds object to this node's contents
    void removeNow(uint uiObjId);   //Removes from this object's contents
    void eraseNow(uint uiObjId);    //Erases from this object's contents

    bool getChildBounds(int iQuadName, const Box &bxMyBounds, Box &bxChildBounds);
    bool childIsLeafNode(const Box &bxChildBounds);
    void updateEmptiness();
    bool empty() { return m_bEmpty; }

    void cleanResults() {
        m_lsDynamicObjs.clear();
        m_lsStaticObjs.clear();
        m_lsObjsLeftQuadrant.clear();
    }

    //Octree node information
    Box m_bxBounds;                                 //Non-relative bounds.  Fluids may expand, but they don't actually move.
    //FluidOctreeNode *m_pParent;
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

    //Information calculated on each scheduled update event, used by parents in their update event, cleared by parents
    SDL_mutex *m_mutex;
    objlist_t m_lsDynamicObjs;      //These objects have moved and must be compared against all objects
    objlist_t m_lsStaticObjs;       //These objects have not moved and must be compared against
    objlist_t m_lsObjsLeftQuadrant; //These objects left their quadrant and need to be added to the next level up
};

class FluidOctreeRoot : public FluidOctreeNode {
public:
    FluidOctreeRoot(uint uiEngineId, uint uiAreaId, const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~FluidOctreeRoot();

    void scheduleUpdates(Scheduler *s) { FluidOctreeNode::recursiveScheduleUpdates(s); }
    virtual int tempGetClassId() { return -1; }

    void debugPrintBounds();

protected:
    FluidOctreeRoot *neighbors[NUM_CARDINAL_DIRECTIONS];
    //TimeField field;
};

class FluidOctreeLeaf : public FluidOctreeNode {
public:
    FluidOctreeLeaf(uint uiEngineId, uint uiAreaId, uint uiLevel, const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~FluidOctreeLeaf();

    //Adding to a leaf node is much simpler than adding to a general node
    virtual bool add(GameObject *obj, bool bForce = false);
    virtual int tempGetClassId() { return 1; }

protected:
    std::list<Vorton> m_lsVortons;
};


class Scheduler {
public:
    virtual void scheduleUpdate(FluidOctreeNode *node) = 0;
};

class BasicScheduler : public Scheduler {
    static BasicScheduler *m_pInstance;
    float m_fTime;

public:
    static BasicScheduler *get(float fTime = -1.f) {
        if(fTime >= 0.f) {
            m_pInstance->m_fTime = fTime;
        }
        return m_pInstance;
    }
    virtual void scheduleUpdate(FluidOctreeNode *node) {
        node->update(m_fTime);
    }
};






#if 0

class FluidOctree3d
{
public:

    FluidOctree3d(const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~FluidOctree3d();

    //Adds object to the appropriate list
    bool add(Vorton *obj, bool bForce = true);

    //Removes from the list but does not call delete
    bool remove(uint uiObjId);

    //Removes from the list and calls delete
    bool erase(uint uiObjId);

    //Returns a reference to the appropriate object
    Vorton *find(uint uiObjId);

    //The Octree is essentially incomplete, you need to implement your own searching functions & other ops

protected:
//private:
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

    bool getChildBounds(int iQuadName, Box &bx);
    void updateEmptiness();
    bool empty() { return m_bEmpty; }

    //Fellow octree information
    //Octree3d *m_pParent;
    FluidOctree3d *m_aChildren[QUAD_NUM_QUADS];

    //Object information
    typedef std::map<uint,Vorton*>::iterator iter_t;
    std::map<uint, Vorton*> m_mObjs;
    bool m_bEmpty;

    //Boundary information
    Box m_bxBounds;
    float m_fMinResolution;
};
#endif

#endif // FLUIDOCTREE3D_H
