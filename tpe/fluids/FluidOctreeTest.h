#ifndef FLUIDOCTREETEST_H
#define FLUIDOCTREETEST_H

#include "FluidOctree.h"
#include "tpe/TimePhysicsEngine.h"
#include "mge/GameObject.h"
#include "d3re/d3re.h"
#include "game/game_defs.h"

class FluidOctreeTest : public GameObject
{
public:
    FluidOctreeTest();
    virtual ~FluidOctreeTest();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual bool update(float fDeltaTime);

    virtual uint getId()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return TYPE_GENERAL; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "FluidOctreeTest"; }

    //Render model
    virtual RenderModel  *getRenderModel()      { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel()     { return m_pPhysicsModel; }

    //Input
    virtual int callBack(uint cID, void *data, uint uiEventId);

    //Scheduler
    void scheduleUpdate(Octree3dNode<Vorton> *node);

protected:
private:
    uint m_uiId;
    flag_t m_uiFlags;
    ContainerRenderModel *m_pRenderModel;
    NullTimePhysicsModel *m_pPhysicsModel;

    FluidOctree *m_pRoot;

    PixelMap *m_pxHmap;

    float m_fCurDeltaTime;
};

#endif // FLUIDOCTREETEST_H
