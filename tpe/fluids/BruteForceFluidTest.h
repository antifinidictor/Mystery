#ifndef BRUTEFORCEFLUIDTEST_H
#define BRUTEFORCEFLUIDTEST_H

#include "mge/PixelMap.h"
#include "mge/GameObject.h"
#include "Vorton.h"
#include "InterpGrid.h"
#include "game/game_defs.h"
#include "d3re/ContainerRenderModel.h"
#include "tpe/TimePhysicsModel.h"
#include <vector>


class BruteForceFluidTest : public GameObject
{
public:
    BruteForceFluidTest(PixelMap *pxHmap, Box bxBounds, int numVortons, float fCellSize, float fViscocity);
    virtual ~BruteForceFluidTest();

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
    static const std::string getClassName()     { return "BruteForceFluidTest"; }

    //Render model
    virtual RenderModel  *getRenderModel()      { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel()     { return m_pPhysicsModel; }

    //Input
    virtual int callBack(uint cID, void *data, uint uiEventId);

    std::vector<GameObject *> m_vTestParticles; //FX sprites or something similar

protected:
private:
    typedef Matrix<3,3> Mat33;

    void computeJacobians();
    void computeVelocities();
    void shareVorticities(float fDeltaTime);
    void updateVortons(float fDeltaTime);

    uint m_uiId, m_uiFlags;
    ContainerRenderModel *m_pRenderModel;
    NullTimePhysicsModel *m_pPhysicsModel;

    PixelMap *m_pxHmap;
    float m_fViscocity;

    std::vector<Vorton> m_vVortons;
    InterpGrid<Vec3f> m_cgVelocities;
    InterpGrid<Mat33> m_cgJacobians;
};

#endif // BRUTEFORCEFLUIDTEST_H
