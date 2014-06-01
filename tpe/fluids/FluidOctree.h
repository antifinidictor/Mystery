#ifndef FLUIDOCTREE_H
#define FLUIDOCTREE_H

#include "Vorton.h"
#include "mge/Octree3d.h"
#include "mge/mgeMath.h"
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

public:
    FluidOctree(Positionable *parent, uint uiNodeId, const Box &bxBounds, float fOctreeMinRes, float fVelocityMinRes, float fJacobianMinRes);
    virtual ~FluidOctree();

    virtual void update(float fTime);

    void computeVelocityAt(int x, int y, int z);
    void computeJacobianAt(int x, int y, int z);

    const InterpGrid<Vec3f> *getVelocityGrid() { return &m_igVelocities; }
    const InterpGrid<Mat33> *getJacobianGrid() { return &m_igJacobians; }
};

#endif // FLUIDOCTREE_H
