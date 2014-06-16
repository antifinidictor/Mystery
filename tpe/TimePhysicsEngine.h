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

    void updateFluid(FluidOctree *fluid);

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


    static int fluidUpdateThread(void *data);

    static TimePhysicsEngine *tpe;

    uint m_uiLastUpdated;
    float m_fDeltaTime;
    bool m_bFinalCleaning;

    //Fluid thread scheduling
    struct FluidInfo {
        FluidOctree *m_pRoot;
        int m_iVelocityUpdatesStarted,  //Used to determine which velocity updates need to be performed
            m_iVelocityUpdatesFinished, //Used to determine when all velocity updates are complete
            m_iTotalVelocityUpdates;    //Used to bound the number of velocity updates
        int m_iJacobianUpdatesStarted,
            m_iJacobianUpdatesFinished,
            m_iTotalJacobianUpdates;
        std::list<FluidOctreeNode*> m_lsUpdateNodeQueue;
        SDL_mutex *m_mxUpdateFluid;

        FluidInfo(FluidOctree *pRoot)
            :   m_pRoot(pRoot),
                m_iVelocityUpdatesStarted(0),
                m_iVelocityUpdatesFinished(0),
                m_iJacobianUpdatesStarted(0),
                m_iJacobianUpdatesFinished(0),
                m_mxUpdateFluid(SDL_CreateMutex())
        {
            //Make sure none of the threads try to access me while I'm initializing
            SDL_LockMutex(m_mxUpdateFluid);

            //Calculate the total velocity/jacabian updates
            const InterpGrid<Vec3f> *vg = m_pRoot->getVelocityGrid();
            const InterpGrid<Matrix<3,3> > *jg = m_pRoot->getJacobianGrid();
            m_iTotalVelocityUpdates = vg->getSizeX() * vg->getSizeY() * vg->getSizeZ();
            m_iTotalJacobianUpdates = jg->getSizeX() * jg->getSizeY() * jg->getSizeZ();

            //Schedule updates
            m_pRoot->scheduleUpdates(this);

            //Done initializing, unlock
            SDL_UnlockMutex(m_mxUpdateFluid);
        }

        FluidInfo(const FluidInfo &info)
            :   m_pRoot(info.m_pRoot),
                m_iVelocityUpdatesStarted(info.m_iVelocityUpdatesStarted),
                m_iVelocityUpdatesFinished(info.m_iVelocityUpdatesFinished),
                m_iTotalVelocityUpdates(info.m_iTotalVelocityUpdates),
                m_iJacobianUpdatesStarted(info.m_iJacobianUpdatesStarted),
                m_iJacobianUpdatesFinished(info.m_iJacobianUpdatesFinished),
                m_iTotalJacobianUpdates(info.m_iTotalJacobianUpdates),
                m_mxUpdateFluid(SDL_CreateMutex())
        {
            //Make sure none of the threads try to access me while I'm initializing
            SDL_LockMutex(m_mxUpdateFluid);

            //for(std::list<FluidOctreeNode*>::iterator it = info.m_lsUpdateNodeQueue.begin(); it != info.m_lsUpdateNodeQueue.end(); ++it) {
            //    m_lsUpdateNodeQueue.push_back(*it);
            //}
            m_pRoot->scheduleUpdates(this);

            //Done initializing, unlock
            SDL_UnlockMutex(m_mxUpdateFluid);
        }

        ~FluidInfo() {
            SDL_DestroyMutex(m_mxUpdateFluid);
        }

        void scheduleUpdate(Octree3dNode<Vorton> *node) {
            m_lsUpdateNodeQueue.push_back((FluidOctreeNode*)node);
        }
    };
    std::list<FluidInfo> m_lsUpdateFluidQueue;
    std::list<SDL_Thread*> m_lsUpdateThreads;
    SDL_mutex *m_mxUpdateFluidQueue;
};

#endif // TIME_PHYSICS_ENGINE_H
