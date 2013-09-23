#ifndef TIME_PHYSICS_ENGINE_H
#define TIME_PHYSICS_ENGINE_H

#include <list>
#include "mge/PhysicsEngine.h"
#include "TimePhysicsModel.h"

class GameObject;
class PhysicsModel;
class AbstractTimePhysicsModel;

enum TpeFlags{
    TPE_STATIC = PHYSICS_FLAGS_BEGIN,   //True if the object cannot move
    TPE_PASSABLE,                       //True if the object cannot be collided with
    TPE_FLOATING,                       //True if the object is not subject to gravity
    TPE_FALLING,                        //Set to true every turn the object moves, unless a surface is found
    TPE_NUM_FLAGS
};

struct HandleCollisionData {
    GameObject *obj;
    int iDirection;
    HandleCollisionData(GameObject *obj, int iDirection) {
        this->obj = obj;
        this->iDirection = iDirection;
    }
};

class TimePhysicsEngine : public PhysicsEngine
{
public:
    static void init()  { tpe = new TimePhysicsEngine(); }
    static void clean() { delete tpe; }
    static TimePhysicsEngine *get() { return tpe; }

    virtual void update(uint time);

    virtual bool applyPhysics(GameObject *obj);
    virtual void applyPhysics(GameObject *obj1, GameObject *obj2);

private:
    TimePhysicsEngine();
    virtual ~TimePhysicsEngine();

    static TimePhysicsEngine *tpe;

    uint m_uiLastUpdated,
         m_uiDeltaTime;
};

#endif // TIME_PHYSICS_ENGINE_H
