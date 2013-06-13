/*
 * CompositeRenderModel
 * Composite 2D render model
 */

#ifndef COMPOSITE_RENDER_MODEL_H
#define COMPOSITE_RENDER_MODEL_H

#include <vector>
#include "mge/RenderModel.h"
#include "mge/Image.h"
using namespace std;

class RenderEngine;

class CompositeRenderModel : public RenderModel {
public:
    CompositeRenderModel() {
        m_rcTotalArea = Rect();
        m_bFirst = true;    //We don't know the draw area, so the first should be added
    }

    CompositeRenderModel(Rect rcArea) {
        m_rcTotalArea = rcArea;
        m_bFirst = false;   //A primary drawing area has already been specified
    }

    virtual ~CompositeRenderModel() {
        for(vector<RenderModel*>::iterator iter = m_vModels.begin();
                iter != m_vModels.end(); ++iter) {
            delete (*iter);
        }
        m_vModels.clear();
    }

    virtual void render(RenderEngine *re) {
        for(vector<RenderModel*>::iterator iter = m_vModels.begin();
                iter != m_vModels.end(); ++iter) {
            (*iter)->render(re);
        }
    }

    virtual void moveBy(Point ptShift) {
        for(vector<RenderModel*>::iterator iter = m_vModels.begin();
                iter != m_vModels.end(); ++iter) {
            (*iter)->moveBy(ptShift);
        }
    }

    virtual Point getPosition() {
        return m_vModels[0]->getPosition();
    }

    virtual void add(RenderModel *mdl) {
        m_vModels.push_back(mdl);

        updateTotalArea(mdl);
    }

    virtual void remove(RenderModel *mdl) {
        //Used to recalculate the total area
        m_bFirst = true;
        for(vector<RenderModel*>::iterator iter = m_vModels.begin();
                iter != m_vModels.end(); ++iter) {
            if((*iter) == mdl) {
                //Remove the object
                m_vModels.erase(iter);
                return;
            } else {
                //Recalculate total area
                updateTotalArea(*iter);
            }
        }
    }

    virtual Rect getDrawArea() { return m_rcTotalArea; }

private:
    vector<RenderModel*> m_vModels;
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
            m_rcTotalArea.l += m_rcTotalArea.y - rcArea.y;
            m_rcTotalArea.y = rcArea.y;
        }
        if(rcArea.x + rcArea.w > m_rcTotalArea.x + m_rcTotalArea.w) {
            m_rcTotalArea.w = (rcArea.x + rcArea.w) - (m_rcTotalArea.x);
        }
        if(rcArea.y + rcArea.l > m_rcTotalArea.y + m_rcTotalArea.l) {
            m_rcTotalArea.l = (rcArea.y + rcArea.l) - (m_rcTotalArea.y);
        }
    }
};

#endif
