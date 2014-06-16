#ifndef TIME_PHYSICS_ENGINE_H
#define TIME_PHYSICS_ENGINE_H

#include <list>
#include "mge/PhysicsEngine.h"
#include "TimePhysicsModel.h"
#include "fluids/FluidOctree.h"
#include "fluids/InterpGrid.h"
#include "SDL.h"

//Some rock densities: http://geology.about.com/cs/rock_types/a/aarockspecgrav.htm
//Some wood densities: http://www.engineeringtoolbox.com/wood-density-d_40.html
#define DENSITY_STONE   2600.f  //Granite
#define DENSITY_WATER   1000.f
#define DENSITY_WOOD    740.f   //American Red Oak

class GameObject;
class PhysicsModel;
class AbstractTimePhysicsModel;

enum TpeFlags{
    TPE_STATIC = PHYSICS_FLAGS_BEGIN,   //True if the object cannot move
    TPE_PASSABLE,                       //True if the object cannot be collided with
    TPE_FLOATING,                       //True if the object is not subject to gravity
    TPE_FALLING,                        //Set to true every turn the object moves, unless a surface is found
    TPE_LIQUID,                         //True if this object can apply a bouyant force
    TPE_NUM_FLAGS
};

enum TpeEvents {
    TPE_ON_COLLISION = PHYSICS_EVENTS_BEGIN,
    TPE_NUM_EVENTS
};

struct HandleCollisionData {
    GameObject *obj;
    int iDirection;
    uint uiCollisionModel;
    Point ptShift;
    HandleCollisionData(GameObject *obj, int iDirection, uint uiCollisionModel, const Point &ptShift) {
        this->obj = obj;
        this->iDirection = iDirection;
        this->ptShift = ptShift;
        this->uiCollisionModel = uiCollisionModel;
    }
};

class TimePhysicsEngine : public PhysicsEngine
{
public:
    static void init();
    static void clean() { delete tpe; }
    static TimePhysicsEngine *get() { return tpe; }

    virtual void update(float fDeltaTime);

    virtual bool applyPhysics(GameObject *obj);
    virtual void applyPhysics(GameObject *obj1, GameObject *obj2);

    virtual void applyCollisionPhysics(std::list<GameObject*> &ls1, std::list<GameObject*> &ls2);

    //void updateFluid(FluidOctree *fluid);

private:
    TimePhysicsEngine();
    virtual ~TimePhysicsEngine();

    //Primary collision model handlers
    void boxOnUnknownCollision(GameObject *obj1, GameObject *obj2, uint uiMdl2);
    void hmapOnUnknownCollision(GameObject *objHmap, GameObject *obj2, uint uiMdlHmap);
    void vortonOnUnknownCollision(GameObject *objVorton, GameObject *obj2, uint uiMdlVorton);

    //Secondary collision model handlers
    void boxOnBoxCollision(GameObject *obj1, GameObject *obj2, uint uiMdl1, uint uiMdl2);
    void boxOnHmapCollision(GameObject *objBox, GameObject *objHmap, uint uiMdlBox, uint uiMdlHmap);
    void vortonOnVortonCollision(GameObject *obj1, GameObject *obj2, uint uiMdl1, uint uiMdl2);
    void vortonOnBoxCollision(GameObject *objVorton, GameObject *objBox, uint uiMdlVorton, uint uiMdlBox);
    void vortonOnHmapCollision(GameObject *objVorton, GameObject *objHmap, uint uiMdlVorton, uint uiMdlHmap);

    //Physics collision helpers
    void splitShift(GameObject *obj1, GameObject *obj2, float fShift, Point *ptShift1, Point *ptShift2);
    void applyBuoyantForce(AbstractTimePhysicsModel *tpmObj, AbstractTimePhysicsModel *tpmLiquid, const Box &bxObj, float liquidTop, float liquidBottom);
    bool isNotInArea(const Box &bxObj, const Box &bxBounds);
    bool isOnSurface(const Box &bxObj, const Box &bxSurface);
    void extractCollisionDirections(const Point &ptCenterDif, float fXShift, float fYShift, float fZShift, int *iDir1, int *iDir2);

    static TimePhysicsEngine *tpe;

    uint m_uiLastUpdated;
    float m_fDeltaTime;
    bool m_bFinalCleaning;
};

#endif // TIME_PHYSICS_ENGINE_H
