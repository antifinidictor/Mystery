#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "mge/defs.h"

class GameObject;

class PhysicsEngine {
public:
    virtual ~PhysicsEngine() {}
    virtual void update(uint time) = 0;

    virtual bool applyPhysics(GameObject *obj) = 0;
    virtual void applyPhysics(GameObject *obj1, GameObject *obj2) = 0;
};

#endif
