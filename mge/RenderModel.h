/*
 * RenderModel.h
 * Interface for the RenderModel, the basis of all renderable game objects
 */

#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#include "mge/defs.h"

class RenderEngine;

class RenderModel {
public:
    virtual void render(RenderEngine *re) = 0;
    virtual void moveBy(Point ptShift) = 0;
    virtual Point getPosition() = 0;
    virtual Rect getDrawArea() = 0;
};

#endif
