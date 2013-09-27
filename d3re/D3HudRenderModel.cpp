/*
 * Source file for the Hud render model
 */

#include "d3re.h"
#include "mge/GameObject.h"
#include "TextRenderer.h"

D3HudRenderModel::D3HudRenderModel(Image *img, const Rect &rcArea) {
    m_pImage = img;
    m_rcDrawArea = rcArea;

    m_fTextSize = -1.f;
    m_sData = "";
    m_ptTextPos = Point();

    m_iFrameW = 0;
    m_iFrameH = 0;
    m_iRepsW = 1;
    m_iRepsH = 1;

    m_crImageColor = Color(0xFF, 0xFF, 0xFF);
}

D3HudRenderModel::D3HudRenderModel(const std::string &data, const Rect &rcArea, float textSize) {
    m_pImage = NULL;
    m_rcDrawArea = rcArea;

    m_fTextSize = textSize;
    m_sData = data;
    m_ptTextPos = Point(rcArea.x, rcArea.y, 0.f);

    m_iFrameW = 0;
    m_iFrameH = 0;
    m_iRepsW = 1;
    m_iRepsH = 1;

    m_crImageColor = Color(0xFF, 0xFF, 0xFF);
}

D3HudRenderModel::D3HudRenderModel(Image *img, const Rect &rcArea, const std::string &data, const Point &ptTextOffset, float textSize) {
    m_pImage = img;
    m_rcDrawArea = rcArea;

    m_fTextSize = textSize;
    m_sData = data;
    m_ptTextPos = Point(ptTextOffset.x + rcArea.x, ptTextOffset.y + rcArea.y, ptTextOffset.z);

    m_iFrameW = 0;
    m_iFrameH = 0;
    m_iRepsW = 1;
    m_iRepsH = 1;

    m_crImageColor = Color(0xFF, 0xFF, 0xFF);
}

D3HudRenderModel::~D3HudRenderModel() {
}

void
D3HudRenderModel::render(RenderEngine *re) {
    if(m_pImage != NULL) {
        renderImage();
    }
    if(m_fTextSize > 0.f) {
        renderText();
    }
}

Rect
D3HudRenderModel::getDrawArea() {
    return m_rcDrawArea;
}

Point
D3HudRenderModel::getPosition() {
    return Point(m_rcDrawArea.x, m_rcDrawArea.y, 0.f);
}

void
D3HudRenderModel::updateText(const std::string &data, float textSize) {
    if(textSize > 0.f) {
        m_fTextSize = textSize;
    } else if(m_fTextSize < 0.f) {
        m_fTextSize = 1.f;
    }
    m_sData = data;
}


void
D3HudRenderModel::renderImage() {
    glPushMatrix();
    //D3RE::get()->prepHud();
    Color ourColor = m_crImageColor;

    //Render engine is responsible for resetting the camera
    float fTexLeft   = m_iFrameW * 1.0F / m_pImage->m_iNumFramesW,
          fTexTop    = m_iFrameH * 1.0F / m_pImage->m_iNumFramesH,
          fTexRight  = m_iFrameW * 1.0F / m_pImage->m_iNumFramesW + m_iRepsW * 1.0F / m_pImage->m_iNumFramesW,
          fTexBottom = m_iFrameH * 1.0F / m_pImage->m_iNumFramesH + m_iRepsH * 1.0F / m_pImage->m_iNumFramesH;

    //Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, m_pImage->m_uiTexture );
    //glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);
        glColor3f(ourColor.r / 255.f, ourColor.g / 255.f, ourColor.b / 255.f);
        //Top-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexTop);
        glVertex3f(m_rcDrawArea.x, m_rcDrawArea.y, 0.f);

        //Top-right vertex (corner)
        glTexCoord2f(fTexRight, fTexTop);
        glVertex3f(m_rcDrawArea.x + m_rcDrawArea.w, m_rcDrawArea.y, 0.f);

        //Bottom-right vertex (corner)
        glTexCoord2f(fTexRight, fTexBottom);
        glVertex3f(m_rcDrawArea.x + m_rcDrawArea.w, m_rcDrawArea.y + m_rcDrawArea.h, 0.f);

        //Bottom-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexBottom);
        glVertex3f(m_rcDrawArea.x, m_rcDrawArea.y + m_rcDrawArea.h, 0.f);
    glEnd();
    //glDepthMask(GL_TRUE);
    glPopMatrix();
}

void
D3HudRenderModel::renderText() {
    glPushMatrix();
    std::string splitData = m_sData;
    float margin = m_ptTextPos.x - m_rcDrawArea.x;
    TextRenderer::get()->splitText(splitData, m_rcDrawArea.w - margin * 2, m_fTextSize);
    TextRenderer::get()->render(splitData.c_str(), m_ptTextPos.x, m_ptTextPos.y, m_fTextSize);
    glPopMatrix();
}
