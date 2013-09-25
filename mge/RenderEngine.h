
#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include "mge/defs.h"

class GameObject;

class RenderEngine {
public:
    virtual ~RenderEngine() {}
    virtual void  render() = 0;
    virtual Point getRenderOffset() = 0;
    virtual void  moveScreenTo(Point pt) = 0;
    virtual void  manageObjOnScreen(GameObject *obj) = 0;
    virtual void  remove(GameObject *obj) = 0;
    virtual bool  screenHasMoved() = 0;
    virtual void  clearScreen() = 0;

protected:
private:
};

#endif
