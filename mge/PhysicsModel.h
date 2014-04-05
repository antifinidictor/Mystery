/*
 * PhysicsModel.h
 * Defines the physics model interface
 */

#ifndef PHYSICS_MODEL_H
#define PHYSICS_MODEL_H

#include "mge/defs.h"
#include "Positionable.h"

class PhysicsModel : public Positionable {
public:
    virtual ~PhysicsModel() {}
    virtual Point getLastVelocity() = 0;
    virtual void  applyForce(const Point &force) = 0;
    virtual Box   getCollisionVolume() = 0;
};

#endif
