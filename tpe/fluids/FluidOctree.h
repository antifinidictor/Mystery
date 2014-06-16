#ifndef FLUIDOCTREE_H
#define FLUIDOCTREE_H

#include "Vorton.h"
#include "mge/Octree3d.h"
#include "mge/mgeMath.h"
#include "mge/WorklistItem.h"
#include "InterpGrid.h"

class FluidOctree;

class FluidOctreeNode : public Octree3dNode<Vorton>
{
protected:
    virtual void updateContents(float fTime);
    virtual void handleChildUpdateResults(Octree3dNode<Vorton> *child, int q);
    //virtual void onAdd(Vorton *obj);
    //virtual void onRemove(Vorton *obj);
    //virtual void onErase(Vorton *obj);

    virtual Octree3dNode<Vorton> *createChild(uint childId, const Box &bxBounds) const;

    //This function only exists because of certain members' protected status.
    // It is only used by the root.
    void positionSearchForVelocity(const Point &ptPosition, Vec3f &v3Velocity);

    Vorton m_vrtAggregate;
    float  m_fTotalVortMag;
    FluidOctree *m_pRoot;

public:
    FluidOctreeNode(FluidOctree *pRoot, uint uiNodeId, uint uiLevel, const Box &bxBounds, float fOctreeMinRes = 1.f);
    virtual ~FluidOctreeNode();
};


/*
class FluidOctreeLeaf : public FluidOctreeNode
{
protected:
    virtual void handleChildUpdateResults(Octree3dNode<Vorton> *child, int q) {}

public:
    FluidOctreeLeaf(FluidOctree *pRoot, uint uiNodeId, uint uiLevel, const Box &bxBounds, float fOctreeMinRes = 1.f);
    virtual ~FluidOctreeLeaf();
};
*/

class FluidOctree : public FluidOctreeNode
{
protected:
    typedef Matrix<3,3> Mat33;
    InterpGrid<Vec3f> m_igVelocities;
    InterpGrid<Mat33> m_igJacobians;
    Positionable *m_pParent;

    //These variables are used to update contents
    uint m_uiVelocityUpdateCount;
    uint m_uiJacobianUpdateCount;
    SDL_mutex *m_mxUpdateCounts;
    SDL_cond  *m_cdWaitVelocities;
    SDL_cond  *m_cdWaitJacobians;

public:
    FluidOctree(Positionable *parent, uint uiNodeId, const Box &bxBounds, float fOctreeMinRes, float fVelocityMinRes, float fJacobianMinRes);
    virtual ~FluidOctree();

    virtual void update(float fTime);

    void computeVelocityAt(int x, int y, int z);
    void computeJacobianAt(int x, int y, int z);

    const InterpGrid<Vec3f> *getVelocityGrid() { return &m_igVelocities; }
    const InterpGrid<Mat33> *getJacobianGrid() { return &m_igJacobians; }

    //The following functions are used to make the world better
    void updateVelocityCount(uint uiCounts) {
        SDL_LockMutex(m_mxUpdateCounts);
        m_uiVelocityUpdateCount += uiCounts;    //Update the count
        SDL_CondBroadcast(m_cdWaitVelocities);  //Broadcast the condition variable, regardless of how many are left
        SDL_UnlockMutex(m_mxUpdateCounts);
    }

    void updateJacobianCount(uint uiCounts) {
        SDL_LockMutex(m_mxUpdateCounts);
        m_uiJacobianUpdateCount += uiCounts;    //Update the count
        SDL_CondBroadcast(m_cdWaitJacobians);   //Broadcast the condition variable, regardless of how many are left
        SDL_UnlockMutex(m_mxUpdateCounts);
    }

    void waitForVelocities() {
        //Calculate the total number of updates we are waiting for
        const InterpGrid<Vec3f> *grid = m_pRoot->getVelocityGrid();
        uint uiSize = grid->getSizeX() * grid->getSizeY() * grid->getSizeZ();

        //Wait for the updates to be completed
        SDL_LockMutex(m_mxUpdateCounts);
        while (m_uiVelocityUpdateCount < uiSize) {
            SDL_CondWait(m_cdWaitVelocities, m_mxUpdateCounts);
        }
        SDL_UnlockMutex(m_mxUpdateCounts);
    }

    void waitForJacobians() {
        //Calculate the total number of updates we are waiting for
        const InterpGrid<Matrix<3,3> > *grid = m_pRoot->getJacobianGrid();
        uint uiSize = grid->getSizeX() * grid->getSizeY() * grid->getSizeZ();

        //Wait for the updates to be completed
        SDL_LockMutex(m_mxUpdateCounts);
        while (m_uiJacobianUpdateCount < uiSize) {
            SDL_CondWait(m_cdWaitJacobians, m_mxUpdateCounts);
        }
        SDL_UnlockMutex(m_mxUpdateCounts);
    }

    void resetCounts() {
        SDL_LockMutex(m_mxUpdateCounts);
        m_uiVelocityUpdateCount = 0;
        m_uiJacobianUpdateCount = 0;
        SDL_UnlockMutex(m_mxUpdateCounts);
    }
};

/*
 * These items ensure that the list is updated properly.
 * The last FluidVelocityWorklistItem schedules the FluidJacobianWorklistItems.
 * The last FluidJacobianWorklistItem schedules the FluidOctreeWorklistItems.
 */
class FluidVelocityWorklistItem : public WorklistItem {
    FluidOctree *m_pRoot;
    uint m_uiMinIndex;
    uint m_uiMaxIndex;

public:
    FluidVelocityWorklistItem(FluidOctree *pRoot, uint uiMinIndex, uint uiMaxIndex)
        :   m_pRoot(pRoot),
            m_uiMinIndex(uiMinIndex),
            m_uiMaxIndex(uiMaxIndex)
    {
    }

    virtual void update() {
        const InterpGrid<Vec3f> *grid = m_pRoot->getVelocityGrid();
        uint uiSizeX = grid->getSizeX();
        uint uiSizeY = grid->getSizeY();
        uint uiSizeZ = grid->getSizeZ();
        uint uiSizeXY = uiSizeX * uiSizeY;

        for(uint uiIndex = m_uiMinIndex; uiIndex < m_uiMaxIndex; ++uiIndex) {
            //Calculate the x, y, z indices from the total index
            uint x = uiIndex % uiSizeX;
            uint y = (uiIndex / uiSizeX) % uiSizeY;
            uint z = (uiIndex / uiSizeXY) % uiSizeZ;
            m_pRoot->computeVelocityAt(x, y, z);
        }

        //These velocities have been updated
        m_pRoot->updateVelocityCount(m_uiMaxIndex - m_uiMinIndex);
    }
};

class FluidJacobianWorklistItem : public WorklistItem {
    FluidOctree *m_pRoot;
    uint m_uiMinIndex;
    uint m_uiMaxIndex;

public:
    FluidJacobianWorklistItem(FluidOctree *pRoot, uint uiMinIndex, uint uiMaxIndex)
        :   m_pRoot(pRoot),
            m_uiMinIndex(uiMinIndex),
            m_uiMaxIndex(uiMaxIndex)
    {
    }

    virtual void update() {
        //Make sure the velocities are available
        m_pRoot->waitForVelocities();

        const InterpGrid<Matrix<3,3> > *grid = m_pRoot->getJacobianGrid();
        uint uiSizeX = grid->getSizeX();
        uint uiSizeY = grid->getSizeY();
        uint uiSizeZ = grid->getSizeZ();
        uint uiSizeXY = uiSizeX * uiSizeY;

        for(uint uiIndex = m_uiMinIndex; uiIndex < m_uiMaxIndex; ++uiIndex) {
            //Calculate the x, y, z indices from the total index
            uint x = uiIndex % uiSizeX;
            uint y = (uiIndex / uiSizeX) % uiSizeY;
            uint z = (uiIndex / uiSizeXY) % uiSizeZ;
            m_pRoot->computeJacobianAt(x, y, z);
        }

        //These velocities have been updated
        m_pRoot->updateJacobianCount(m_uiMaxIndex - m_uiMinIndex);
    }
};

class FluidOctreeWorklistItem : public WorklistItem {
    FluidOctree *m_pRoot;
    FluidOctreeNode *m_pNode;
    float m_fDeltaTime;

public:
    FluidOctreeWorklistItem(FluidOctree *pRoot, FluidOctreeNode *pNode, float fDeltaTime)
        :   m_pRoot(pRoot),
            m_pNode(pNode),
            m_fDeltaTime(fDeltaTime)
    {
    }

    virtual void update() {
        //Make sure that the jacobians are available
        m_pRoot->waitForJacobians();

        m_pNode->update(m_fDeltaTime);
    }
};

#endif // FLUIDOCTREE_H
