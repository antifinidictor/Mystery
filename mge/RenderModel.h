/*
 * RenderModel.h
 * Interface for the RenderModel, the basis of all renderable game objects
 */

#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#include "mge/defs.h"
#include "mge/Positionable.h"

class RenderEngine;

class RenderModel {
protected:
    Positionable *m_pParent;

    inline Point getParentPosition() { return (m_pParent != NULL) ? m_pParent->getPosition() : Point(); }

public:
    RenderModel() { m_pParent = NULL; }
    virtual ~RenderModel() {}
    virtual void render(RenderEngine *re) = 0;

    void setParent(Positionable *pParent) { m_pParent = pParent; }
//    virtual void moveBy(const Point &ptShift) = 0;
//    virtual Point getPosition() = 0;
//    virtual Rect getDrawArea() = 0;
};

class NullRenderModel : public RenderModel {
private:
    static NullRenderModel *rm;

public:
    static void init() { rm = new NullRenderModel(); }
    static NullRenderModel *get() { return rm; }
    static void clean() { delete rm; }

    virtual void render(RenderEngine *re) {}
//    virtual void moveBy(const Point &ptShift) {}
//    virtual Point getPosition() { return Point(); }
//    virtual Rect getDrawArea() { return Rect(); }
};

#endif
