/*
 * WallRenderModel
 * Wall 2D render model
 */

#ifndef WALL_RENDER_MODEL_H
#define WALL_RENDER_MODEL_H

#include "mge/RenderModel.h"
#include "mge/RenderEngine.h"
#include "mge/Image.h"

class WallRenderModel : public RenderModel {
public:
    WallRenderModel(Image *pImage, Rect rcArea, int iLength, int iDirection) {
        m_rcDrawArea = rcArea;
        m_pImage = pImage;

        m_fTexLeft   = 0;
        m_fTexTop    = 0;
        m_fTexBottom = 1;
        m_fTexRight  = iLength * 1.0F / pImage->w;
        m_iDirection = iDirection;
    }

    virtual ~WallRenderModel() {
    }

    virtual void render(RenderEngine *re) {
        Point ptScreenOffset = re->getRenderOffset();
        
        //Get the temporary drawing rectangle
        float fDrawLeft   = m_rcDrawArea.x - ptScreenOffset.x,
              fDrawTop    = m_rcDrawArea.y - ptScreenOffset.y,
              fDrawRight  = m_rcDrawArea.x - ptScreenOffset.x + m_rcDrawArea.w,
              fDrawBottom = m_rcDrawArea.y - ptScreenOffset.y + m_rcDrawArea.l;
        float fX0, fY0,
              fX1, fY1,
              fX2, fY2,
              fX3, fY3;

        switch(m_iDirection) {
        case NORTH:
            fX0 = fDrawLeft;
            fY0 = fDrawTop;
            fX1 = fDrawRight;
            fY1 = fDrawTop;
            fX2 = fDrawRight;
            fY2 = fDrawBottom;
            fX3 = fDrawLeft;
            fY3 = fDrawBottom;
            break;
        case EAST:
            fX0 = fDrawRight;
            fY0 = fDrawTop;
            fX1 = fDrawRight;
            fY1 = fDrawBottom;
            fX2 = fDrawLeft;
            fY2 = fDrawBottom;
            fX3 = fDrawLeft;
            fY3 = fDrawTop;
            break;
        case SOUTH:
            fX0 = fDrawRight;
            fY0 = fDrawBottom;
            fX1 = fDrawLeft;
            fY1 = fDrawBottom;
            fX2 = fDrawLeft;
            fY2 = fDrawTop;
            fX3 = fDrawRight;
            fY3 = fDrawTop;
            break;
        case WEST:
            fX0 = fDrawLeft;
            fY0 = fDrawBottom;
            fX1 = fDrawLeft;
            fY1 = fDrawTop;
            fX2 = fDrawRight;
            fY2 = fDrawTop;
            fX3 = fDrawRight;
            fY3 = fDrawBottom;
            break;
        }

        //Bind the texture to which subsequent calls refer to
        glBindTexture( GL_TEXTURE_2D, m_pImage->m_uiTexture );

        glBegin( GL_QUADS );
            //Top-left vertex (corner)
            glTexCoord2f(m_fTexLeft, m_fTexTop);
            glVertex3f(fX0, fY0, 0.0f);

            //Top-right vertex (corner)
            glTexCoord2f(m_fTexRight, m_fTexTop);
            glVertex3f(fX1, fY1, 0.f);

            //Bottom-right vertex (corner)
            glTexCoord2f(m_fTexRight, m_fTexBottom);
            glVertex3f(fX2, fY2, 0.f);

            //Bottom-left vertex (corner)
            glTexCoord2f(m_fTexLeft, m_fTexBottom);
            glVertex3f(fX3, fY3, 0.f);
        glEnd();
    }

    virtual void moveBy(Point ptShift) {
        m_rcDrawArea += ptShift;
    }

    virtual Point getPosition() {
        return Point(m_rcDrawArea);
    }
    
    virtual Rect getDrawArea() { return m_rcDrawArea; }

    Image *getImage() { return m_pImage; }

private:
    Image *m_pImage;
    Rect m_rcDrawArea;

    int m_iDirection;
    float m_fTexLeft, m_fTexTop,
          m_fTexRight, m_fTexBottom;
};

#endif
