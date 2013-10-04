#ifndef WORLD_ENGINE_H
#define WORLD_ENGINE_H


#include "mge/defs.h"

class GameObject;

class WorldEngine {
public:
    virtual ~WorldEngine() {}
    virtual void update(uint time) = 0;
    virtual void add(GameObject *obj) = 0;
    virtual void remove(uint id) = 0;
    virtual uint genId() = 0;               //Returns a free ID
    virtual void freeId(uint id) = 0;       //Frees an ID for use by other objects.  Call in the object's destructor
    virtual uint reserveId(uint id) = 0;    //Requests the provided ID, and returns either the requested ID, or, if taken, a different id
protected:
private:
};

#endif

