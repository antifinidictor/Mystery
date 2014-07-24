/*
 * ContainerRenderModel
 * Composite 2D render model
 */

#ifndef CONTAINER_RENDER_MODEL_H
#define CONTAINER_RENDER_MODEL_H

#include <map>
#include "mge/RenderModel.h"
#include "mge/Positionable.h"

class RenderEngine;

typedef void (*opOnRenderModel)(uint id, RenderModel *rm);

class ContainerRenderModel : public RenderModel, public Positionable {
public:
    ContainerRenderModel(Positionable *pParent, Rect rcArea);
    //This is a hack
    ContainerRenderModel(Positionable *pParent, Rect rcArea, Point ptOffset);
    virtual ~ContainerRenderModel();

    virtual void render(RenderEngine *re);
    virtual void moveBy(const Point &ptShift);
    virtual Point getPosition();

    void add(uint id, RenderModel *mdl);
    void remove(uint id);
    void erase(uint id);
    void clear();

    template <class RenderModelType>
    RenderModelType get(uint id) {
        std::map<uint, RenderModel*>::iterator it = m_mModels.find(id);
        if(it != m_mModels.end()) {
            return dynamic_cast<RenderModelType>(it->second);
        } else {
            return NULL;
        }
    }

    template <class Functor>
    void forEachModel(Functor &f) {
        std::map<uint, RenderModel*>::iterator it;
        for(it = m_mModels.begin(); it != m_mModels.end(); ++it) {
            bool finished = f(it->first, it->second);
            if(finished) {
                break;
            }
        }
    }

    virtual Rect getDrawArea() { return m_rcTotalArea; }

private:
    std::map<uint, RenderModel*> m_mModels;
    Rect m_rcTotalArea;
    Point m_ptOffset;
};

#endif //CONTAINER_RENDER_MODEL_H
