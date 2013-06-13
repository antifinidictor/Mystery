/*
 * StripRenderModel.h
 * Renders horizontal strips of an image
 */
/*
 * EdgeRenderModel
 * Ordered 2D render model
 */

#ifndef STRIP_RENDER_MODEL_H
#define STRIP_RENDER_MODEL_H

#include "mge/RenderModel.h"
#include "mge/Image.h"

#define Z_TO_Y(z) (-z * 0.75)
#define TILE_SIZE 32

struct SRM_Strip {
    int x, y;
    int m_iReps;
    int m_iFrame;
};

class StripRenderModel : public RenderModel {
public:
    StripRenderModel(Image *pImage, Rect rcArea, int iNumStrips, int iLayer);

    virtual ~StripRenderModel() {
        free(m_aStrips);
    }

    virtual void render(RenderEngine *re);


    virtual void moveBy(Point ptShift) {
        m_rcDrawArea += ptShift;
        m_rcDrawArea.y += Z_TO_Y(ptShift.z);
    }

    virtual Point getPosition() {
        return Point(m_rcDrawArea.x, m_rcDrawArea.y, m_iLayer);
    }

    inline void setStrip(int iStrip, int ix, int iy, int iReps, int iFrame) {
        SRM_Strip *pStrip = &m_aStrips[iStrip];
        pStrip->x = ix;
        pStrip->y = iy;
        pStrip->m_iReps = iReps;
        pStrip->m_iFrame = iFrame;
    }

    inline void setStrip(int iStrip, int iReps, int iFrame) {
        SRM_Strip *pStrip = &m_aStrips[iStrip];
        pStrip->m_iReps = iReps;
        pStrip->m_iFrame = iFrame;
    }

    void setLayer(int iLayer) { m_iLayer = iLayer; }

    virtual Rect getDrawArea() { return m_rcDrawArea; }
    Image *getImage() { return m_pImage; }

private:
    Image *m_pImage;
    Rect m_rcDrawArea;
    SRM_Strip *m_aStrips;

    int m_iLayer;
    int m_iNumStrips;

    void renderStrip(RenderEngine *re, SRM_Strip *pStrip);
};

#endif
