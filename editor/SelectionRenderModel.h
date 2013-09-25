/*
 * SelectionRenderModel
 * A simple render model that renders a box as a particular color.
 */
#ifndef SELECTION_RENDER_MODEL_H
#define SELECTION_RENDER_MODEL_H

#include "d3re/d3re.h"

class SelectionRenderModel : public RenderModel {
public:
    SelectionRenderModel(const Box &bx, const Color &cr) {
        setVolume(bx);
        setColor(cr);
    }

    virtual ~SelectionRenderModel() {}

    virtual void render(RenderEngine *re) {
        D3RE::get()->drawBox(m_bxVolume, m_crColor);
    }

    virtual void moveBy(Point ptShift) {
        m_bxVolume.x += ptShift.x;
        m_bxVolume.y += ptShift.y;
        m_bxVolume.z += ptShift.z;
    }

    virtual Point getPosition() {
        return m_bxVolume;
    }

    virtual Rect getDrawArea() { return m_bxVolume; }

    Box getVolume() { return m_bxVolume; }
    
    void setVolume(const Box &bx) {
        m_bxVolume = bx;
    }
    
    void expandVolume(const Point &pt) {
        m_bxVolume.w += pt.x;
        m_bxVolume.h += pt.y;
        m_bxVolume.l += pt.z;
        while (m_bxVolume.w < TILE_SIZE) {
            m_bxVolume.x -= TILE_SIZE;
            m_bxVolume.w += TILE_SIZE;
        }
        while (m_bxVolume.l < TILE_SIZE) {
            m_bxVolume.z -= TILE_SIZE;
            m_bxVolume.l += TILE_SIZE;
        }
        while (m_bxVolume.h < TILE_SIZE) {
            m_bxVolume.y -= TILE_SIZE;
            m_bxVolume.h += TILE_SIZE;
        }
    }
    
    void setColor(const Color &cr) {
        m_crColor = cr;
    }
private:
    Color m_crColor;
    Box m_bxVolume;
};

#endif //SELECTION_RENDER_MODEL_H
