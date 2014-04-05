/*
 * GaemObject.h
 * File defining the basic game object interface
 */

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "mge/defs.h"
#include "mge/PhysicsModel.h"
#include "mge/RenderModel.h"
#include "mge/Event.h"
#include "mge/ModularEngine.h"
#include <boost/property_tree/ptree.hpp>

class GameObject : public Listener {
public:
    //Destructor
    virtual ~GameObject() {
    }

    //General
    //virtual uint getId() = 0;
    virtual bool getFlag(uint flag) = 0;
    virtual void setFlag(uint flag, bool value) = 0;
    virtual bool update(uint time) = 0; //Returns true if the object should be deleted after this turn
    virtual uint getType() = 0;         //Returns some identifier indicating the object's class
    virtual const std::string getClass() = 0;   //Returns the human-readable class name
    virtual const std::string getName() {
        std::ostringstream name;
        name << getClass() << getId();
        return name.str();
    }
    virtual void moveBy(const Point &ptShift) {
        getPhysicsModel()->moveBy(ptShift);
    }

    //Render model
    virtual RenderModel  *getRenderModel() = 0;
    virtual PhysicsModel *getPhysicsModel() = 0;

    //File I/O &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase) = 0;
};

#endif
