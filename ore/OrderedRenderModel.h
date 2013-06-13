/*
 * OrderedRenderModel
 * Ordered 2D render model
 */

#ifndef ORDERED_RENDER_MODEL_H
#define ORDERED_RENDER_MODEL_H

#include "mge/RenderModel.h"
#include "mge/RenderEngine.h"
#include "mge/Image.h"

#define Z_TO_Y(z) (-z * 0.75)

class OrderedRenderModel : public RenderModel {
public:
    OrderedRenderModel(Image *pImage, Rect rcArea, float fZ, int iLayer) {
        m_rcDrawArea = Rect(rcArea.x, rcArea.y + Z_TO_Y(fZ), rcArea.w, rcArea.l);
        m_pImage = pImage;
        m_iFrameW = m_iFrameH = 0;
        m_iRepsW = m_iRepsH = 1;
        m_iLayer = iLayer;
    }

    virtual ~OrderedRenderModel() {
    }

    virtual void render(RenderEngine *re) {
        Point ptScreenOffset = re->getRenderOffset();
        //Get the temporary texture rectangle
        float fTexLeft   = m_iFrameW * 1.0F / m_pImage->m_iNumFramesW,
              fTexTop    = m_iFrameH * 1.0F / m_pImage->m_iNumFramesH,
              fTexRight  = m_iFrameW * 1.0F / m_pImage->m_iNumFramesW + m_iRepsW * 1.0F / m_pImage->m_iNumFramesW,
              fTexBottom = m_iFrameH * 1.0F / m_pImage->m_iNumFramesH + m_iRepsH * 1.0F / m_pImage->m_iNumFramesH;

        //Get the temporary drawing rectangle
        float fDrawLeft   = m_rcDrawArea.x - ptScreenOffset.x,
              fDrawTop    = m_rcDrawArea.y - ptScreenOffset.y,
              fDrawRight  = m_rcDrawArea.x - ptScreenOffset.x + m_rcDrawArea.w,
              fDrawBottom = m_rcDrawArea.y - ptScreenOffset.y + m_rcDrawArea.l;

        //Bind the texture to which subsequent calls refer to
        glBindTexture( GL_TEXTURE_2D, m_pImage->m_uiTexture );

        glBegin( GL_QUADS );
            //Top-left vertex (corner)
            glTexCoord2f(fTexLeft, fTexTop);
            glVertex3f(fDrawLeft, fDrawTop, 0.0f);

            //Top-right vertex (corner)
            glTexCoord2f(fTexRight, fTexTop);
            glVertex3f(fDrawRight, fDrawTop, 0.f);

            //Bottom-right vertex (corner)
            glTexCoord2f(fTexRight, fTexBottom);
            glVertex3f(fDrawRight, fDrawBottom, 0.f);

            //Bottom-left vertex (corner)
            glTexCoord2f(fTexLeft, fTexBottom);
            glVertex3f(fDrawLeft, fDrawBottom, 0.f);
        glEnd();
    }

    virtual void moveBy(Point ptShift) {
        m_rcDrawArea += ptShift;
        m_rcDrawArea.y += Z_TO_Y(ptShift.z);
    }

    virtual Point getPosition() {
        return Point(m_rcDrawArea.x, m_rcDrawArea.y, m_iLayer);
    }

    void setFrameW(int fw) { m_iFrameW = fw; }
    void setFrameH(int fh) { m_iFrameH = fh; }
    void setRepsW(int rw)  { m_iRepsW = rw; }
    void setRepsH(int rh)  { m_iRepsH = rh; }
    void setLayer(int iLayer) { m_iLayer = iLayer; }

    virtual Rect getDrawArea() { return m_rcDrawArea; }
    Image *getImage() { return m_pImage; }

private:
    Image *m_pImage;
    Rect m_rcDrawArea;

    int m_iFrameW, m_iFrameH;
    int m_iRepsW, m_iRepsH;
    int m_iLayer;
};

#endif
