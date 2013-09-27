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
    ContainerRenderModel() {
        m_rcTotalArea = Rect();
        m_bFirst = true;    //We don't know the draw area, so the first should be added
    }

    ContainerRenderModel(Rect rcArea) {
        m_rcTotalArea = rcArea;
        m_bFirst = false;   //A primary drawing area has already been specified
    }

    virtual ~ContainerRenderModel() {
        clear();
    }

    virtual void render(RenderEngine *re) {
        for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
                iter != m_mModels.end(); ++iter) {
            iter->second->render(re);
        }
    }

    virtual void moveBy(Point ptShift) {
        for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
                iter != m_mModels.end(); ++iter) {
            iter->second->moveBy(ptShift);
        }
    }

    virtual Point getPosition() {
        return m_mModels[0]->getPosition();
    }

    void add(uint id, RenderModel *mdl) {
        m_mModels[id] = mdl;

        updateTotalArea(mdl);
    }

    void remove(uint id) {
        m_mModels.erase(id);

        //Recalculate the total area
        m_bFirst = true;
        for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
                iter != m_mModels.end(); ++iter) {
            updateTotalArea(iter->second);
        }
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
    bool m_bFirst;

    void updateTotalArea(RenderModel *mdl) {
        Rect rcArea = mdl->getDrawArea();

        if(m_bFirst) {
            m_rcTotalArea = rcArea;
            m_bFirst = false;
            return;
        }

        if(rcArea.x < m_rcTotalArea.x) {
            m_rcTotalArea.w += m_rcTotalArea.x - rcArea.x;
            m_rcTotalArea.x = rcArea.x;
        }
        if(rcArea.y < m_rcTotalArea.y) {
            m_rcTotalArea.h += m_rcTotalArea.y - rcArea.y;
            m_rcTotalArea.y = rcArea.y;
        }
        if(rcArea.x + rcArea.w > m_rcTotalArea.x + m_rcTotalArea.w) {
            m_rcTotalArea.w = (rcArea.x + rcArea.w) - (m_rcTotalArea.x);
        }
        if(rcArea.y + rcArea.h > m_rcTotalArea.y + m_rcTotalArea.h) {
            m_rcTotalArea.h = (rcArea.y + rcArea.h) - (m_rcTotalArea.y);
        }
    }
};

#endif //CONTAINER_RENDER_MODEL_H
