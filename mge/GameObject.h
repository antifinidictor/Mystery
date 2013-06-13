/*
 * GaemObject.h
 * File defining the basic game object interface
 */

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "mge/defs.h"
#include "mge/PhysicsModel.h"
#include "mge/RenderModel.h"

class GameObject {
public:
    //Destructor
    virtual ~GameObject() {}

    //General
    virtual uint getID() = 0;
    virtual bool update(uint time) = 0; //Returns true if the object should be deleted after this turn
    virtual bool getFlag(uint flag) = 0;
    virtual void setFlag(uint flag, bool value) = 0;
    virtual uint getType() = 0;         //Returns some identifier indicating the object's class
    virtual void moveBy(Point ptShift) {
        getRenderModel()->moveBy(ptShift);
        getPhysicsModel()->moveBy(ptShift);
    }

    //Render model
    virtual RenderModel  *getRenderModel() = 0;
    virtual PhysicsModel *getPhysicsModel() = 0;
};

#endif
