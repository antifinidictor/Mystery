/*
 * PhysicsModel.h
 * Defines the physics model interface
 */

#ifndef PHYSICS_MODEL_H
#define PHYSICS_MODEL_H

#include "mge/defs.h"

class PhysicsModel {
public:
    virtual ~PhysicsModel() {}
    virtual Point getPosition() = 0;
    virtual Point getLastVelocity() = 0;
    virtual void  moveBy(Point ptShift) = 0;    //non-physics shift
    virtual void  applyForce(Point force) = 0;
    virtual Box   getCollisionVolume() = 0;
};

#endif
