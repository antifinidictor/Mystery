/*
 * ContainerRenderModel
 * Composite 2D render model
 */

#ifndef CONTAINER_RENDER_MODEL_H
#define CONTAINER_RENDER_MODEL_H

#include <map>
#include "mge/RenderModel.h"

class RenderEngine;

class ContainerRenderModel : public RenderModel {
public:
    ContainerRenderModel(Rect rcArea) {
        m_rcTotalArea = rcArea;
    }

    virtual ~ContainerRenderModel() {
        clear();
    }

    virtual void render(RenderEngine *re) {
        glPushMatrix();
        glTranslatef(m_rcTotalArea.x, m_rcTotalArea.y, 0.f);
        for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
                iter != m_mModels.end(); ++iter) {
            iter->second->render(re);
        }
        glPopMatrix();
    }

    virtual void moveBy(Point ptShift) {
        for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
                iter != m_mModels.end(); ++iter) {
            iter->second->moveBy(ptShift);
        }
    }

    virtual Point getPosition() {
        return m_rcTotalArea;
    }

    void add(uint id, RenderModel *mdl) {
        m_mModels[id] = mdl;
    }

    void remove(uint id) {
        m_mModels.erase(id);
    }

    void clear() {
        for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
                iter != m_mModels.end(); ++iter) {
            delete iter->second;
        }
        m_mModels.clear();
    }

    template <class RenderModelType>
    RenderModelType get(uint id) {
        std::map<uint, RenderModel*>::iterator it = m_mModels.find(id);
        if(it != m_mModels.end()) {
            return dynamic_cast<RenderModelType>(it->second);
        } else {
            return NULL;
        }
    }

    virtual Rect getDrawArea() { return m_rcTotalArea; }

private:
    std::map<uint, RenderModel*> m_mModels;
    Rect m_rcTotalArea;
};

#endif //CONTAINER_RENDER_MODEL_H
