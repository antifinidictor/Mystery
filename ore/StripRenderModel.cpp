/*
 * StripRenderModel.cpp
 */
#include "StripRenderModel.h"
#include "ore/OrderedRenderEngine.h"
using namespace std;

StripRenderModel::StripRenderModel(Image *pImage, Rect rcArea, int iNumStrips, int iLayer) {
    m_pImage = pImage;
    m_rcDrawArea = rcArea;
    m_aStrips = (SRM_Strip*)malloc(sizeof(SRM_Strip) * iNumStrips);

    m_iLayer = iLayer;
    m_iNumStrips = iNumStrips;
}

void StripRenderModel::render(RenderEngine *re) {
    SRM_Strip *ptr = m_aStrips;
    for(int i = 0; i++ < m_iNumStrips; ++ptr) {
        renderStrip(re, ptr);
    }
}

void StripRenderModel::renderStrip(RenderEngine *re, SRM_Strip *pStrip) {
    Point ptScreenOffset = re->getRenderOffset();
    int iFrame = pStrip->m_iFrame,
        iReps = pStrip->m_iReps;

    //Get the temporary texture rectangle
    float fTexLeft   = 0.0F,
          fTexTop    = iFrame * 1.0F / m_pImage->m_iNumFramesH,
          fTexRight  = iReps * 1.0F / m_pImage->m_iNumFramesW,
          fTexBottom = (iFrame + 1.0F) / m_pImage->m_iNumFramesH;

    //Get the temporary drawing rectangle
    int iDrawLeft   = (int)m_rcDrawArea.x + pStrip->x * TILE_SIZE - (int)ptScreenOffset.x,
        iDrawTop    = (int)m_rcDrawArea.y + pStrip->y * TILE_SIZE - (int)ptScreenOffset.y,
        iDrawRight  = iDrawLeft + iReps * TILE_SIZE,//m_rcDrawArea.w,
        iDrawBottom = iDrawTop + TILE_SIZE;//m_rcDrawArea.l;

    //Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, m_pImage->m_uiTexture );

    glBegin( GL_QUADS );
        //Top-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexTop);
        glVertex3f(iDrawLeft, iDrawTop, 0.0f);

        //Top-right vertex (corner)
        glTexCoord2f(fTexRight, fTexTop);
        glVertex3f(iDrawRight, iDrawTop, 0.f);

        //Bottom-right vertex (corner)
        glTexCoord2f(fTexRight, fTexBottom);
        glVertex3f(iDrawRight, iDrawBottom, 0.f);

        //Bottom-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexBottom);
        glVertex3f(iDrawLeft, iDrawBottom, 0.f);
    glEnd();
}
