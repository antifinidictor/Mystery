#ifndef REGION_H
#define REGION_H

#include "mge/GameObject.h"
#include "d3re/d3re.h"
#include "tpe/TimePhysicsEngine.h"

class Region : public GameObject
{
public:
/*
    Region(uint uiId, Box bxVolume);
    virtual ~Region();

    //Region-specific
    void split(Box bxVolume);
    Box  getVolume() { return m_bxVolume; }

    //General
    virtual bool update(uint time);

    virtual uint getID()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return TYPE_PLAYER; }

    //Render model
    virtual RenderModel  *getRenderModel()      { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel()     { return m_pPhysicsModel; }

private:
    void buildRegion(Box bxVolume);
    bool isValid(const Box &bxVolume);

    uint m_uiId, m_uiFlags;
    TimePhysicsModel *m_pPhysicsModel;
    OrderedRenderModel *m_pRenderModel;

    bool m_bIsDead;
*/
};

#endif // REGION_H
