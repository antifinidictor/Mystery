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
    virtual uint genID() = 0;
    virtual void free(uint id) = 0;
protected:
private:
};

#endif

