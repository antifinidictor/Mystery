#include "FluidOctree.h"
#define DEFAULT_VORTON_RADIUS 0.1
FluidOctreeNode::FluidOctreeNode(uint uiNodeId, uint uiLevel, const Box &bxBounds, float fOctreeMinRes)
    :   Octree3dNode<Vorton>(uiNodeId, uiLevel, bxBounds, fOctreeMinRes),
        m_vrtAggregate(0, bxCenter(bxBounds), DEFAULT_VORTON_RADIUS, Vec3f())
{
    //ctor
}

FluidOctreeNode::~FluidOctreeNode()
{
    //dtor
}

void
FluidOctreeNode::updateContents(float fTime) {
    //Update each vorton
    for(objmap_iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
        //it->second->update(fTime);
    }

    //Prepare to aggregate vortons
    m_vrtAggregate = Vorton(0, bxCenter(m_bxBounds), DEFAULT_VORTON_RADIUS, Vec3f());
    m_fTotalVortMag = 0.f;
}

void
FluidOctreeNode::handleChildUpdateResults(Octree3dNode<Vorton> *node, int q) {
    FluidOctreeNode *child = (FluidOctreeNode*)node;

    //The child's vorticity has been aggregated, but has not had the position
    // divided by the magnitude.
    Vec3f v3ChildVorticity = child->m_vrtAggregate.getVorticity();
    float fVortMag = v3ChildVorticity.magnitude();

    m_fTotalVortMag += fVortMag;
    child->m_vrtAggregate.getPosition();
}

Octree3dNode<Vorton> *
FluidOctreeNode::createChild(uint childId, const Box &bxBounds) const {
    //Calculate the node id
    uint uiNewLevel = m_uiLevel + 1;
    uint uiNewNodeId = (childId << (uiNewLevel * 4)) | m_uiNodeId;

    FluidOctreeNode *node;
    if(childIsLeafNode(bxBounds)) {
        node = new FluidOctreeNode(
            uiNewNodeId,
            uiNewLevel,
            bxBounds,
            m_fMinResolution
        );
    } else {
        node = new FluidOctreeLeaf(
            uiNewNodeId,
            uiNewLevel,
            bxBounds,
            m_fMinResolution
        );
    }
    return node;
}

/*
void
FluidOctreeNode::onAdd(Vorton *obj) {
}

void
FluidOctreeNode::onRemove(Vorton *obj) {
}

void
FluidOctreeNode::onErase(Vorton *obj) {
}
*/

/*
 * FluidOctreeLeaf
 */
FluidOctreeLeaf::FluidOctreeLeaf(uint uiNodeId, uint uiLevel, const Box &bxBounds, float fOctreeMinRes)
    :   FluidOctreeNode(uiNodeId, uiLevel, bxBounds, fOctreeMinRes)
{
    //ctor
}

FluidOctreeLeaf::~FluidOctreeLeaf()
{
    //dtor
}

void
FluidOctreeLeaf::updateContents(float fTime) {
    m_vrtAggregate = Vorton(0, bxCenter(m_bxBounds), DEFAULT_VORTON_RADIUS, Vec3f());

    //Update and aggregate vortons
}

/*
 * FluidOctree
 */
FluidOctree::FluidOctree(Positionable *parent, uint uiNodeId, const Box &bxBounds, float fOctreeMinRes, float fVelocityMinRes, float fJacobianMinRes)
    :   FluidOctreeNode(uiNodeId, 0, bxBounds, fOctreeMinRes),
        m_igVelocities(parent, bxBounds, fVelocityMinRes),
        m_igJacobians(parent, bxBounds, fJacobianMinRes),
        m_pParent(parent)
{
    //ctor
}

FluidOctree::~FluidOctree()
{
    //dtor
}
