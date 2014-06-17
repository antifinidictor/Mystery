#ifndef WORLDOCTREE_H
#define WORLDOCTREE_H

#include "mge/Octree3d.h"
#include "mge/GameObject.h"
#include <boost/property_tree/ptree.hpp>

class WorldOctreeNode : public Octree3dNode<GameObject>
{
public:
    WorldOctreeNode(uint uiNodeId, uint uiLevel, uint uiAreaId, const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~WorldOctreeNode();

    void write(boost::property_tree::ptree &pt, const std::string &keyBase, bool bIsSaveFile);

    uint getAreaId() { return m_uiAreaId; }

protected:
    virtual void updateContents(float fTime);
    virtual void handleChildUpdateResults(Octree3dNode<GameObject> *child, int q);
    virtual void onAdd(GameObject *obj);
    virtual void onRemove(GameObject *obj);
    virtual void onErase(GameObject *obj);
    //virtual void onSchedule() {}
    virtual void cleanResults() {
        m_lsDynamicObjs.clear();
        m_lsStaticObjs.clear();
        m_lsObjsLeftQuadrant.clear();
        m_bIsFinished = false;
    }
    virtual Octree3dNode<GameObject> *createChild(uint childId, const Box &bxBounds) const;

    uint m_uiAreaId;
    objlist_t m_lsDynamicObjs;      //These objects have moved and must be compared against all objects
    objlist_t m_lsStaticObjs;       //These objects have not moved and must be compared against
};

class WorldOctree : public WorldOctreeNode
{
public:
    WorldOctree(uint uiNodeId, uint uiAreaId, const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~WorldOctree();

    //virtual void update(float fTime);
    void postUpdate(float fTime);
protected:
};

#endif // WORLDOCTREE_H
