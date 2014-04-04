/*
 * Source file for the Hud render model
 */

#include "d3re.h"
#include "mge/GameObject.h"
#include "TextRenderer.h"

D3HudRenderModel::D3HudRenderModel(uint uiImageId, const Rect &rcArea) {
    m_uiImageId = uiImageId;
    m_rcDrawArea = rcArea;

    m_fTextSize = -1.f;
    m_sData = "";
    m_ptTextPos = Point(rcArea.x, rcArea.y, 0.f);

    m_iFrameW = 0;
    m_iFrameH = 0;
    m_iRepsW = 1;
    m_iRepsH = 1;

    m_bVertCenter = false;
    m_bHorizCenter = false;

    m_crImageColor = Color(0xFF, 0xFF, 0xFF);
}

D3HudRenderModel::D3HudRenderModel(const std::string &data, const Rect &rcArea, float textSize) {
    m_uiImageId = 0;
    m_rcDrawArea = rcArea;

    m_fTextSize = textSize;
    m_sData = data;
    m_ptTextPos = Point(rcArea.x, rcArea.y, 0.f);

    m_iFrameW = 0;
    m_iFrameH = 0;
    m_iRepsW = 1;
    m_iRepsH = 1;

    m_crImageColor = Color(0xFF, 0xFF, 0xFF);
    m_bVertCenter = false;
    m_bHorizCenter = false;
}

D3HudRenderModel::D3HudRenderModel(uint uiImageId, const Rect &rcArea, const std::string &data, const Point &ptTextOffset, float textSize) {
    m_uiImageId = uiImageId;
    m_rcDrawArea = rcArea;

    m_fTextSize = textSize;
    m_sData = data;
    m_ptTextPos = Point(ptTextOffset.x + rcArea.x, ptTextOffset.y + rcArea.y, ptTextOffset.z);

    m_iFrameW = 0;
    m_iFrameH = 0;
    m_iRepsW = 1;
    m_iRepsH = 1;

    m_crImageColor = Color(0xFF, 0xFF, 0xFF);
    m_bVertCenter = false;
    m_bHorizCenter = false;
}

D3HudRenderModel::~D3HudRenderModel() {
}

void
D3HudRenderModel::render(RenderEngine *re) {
    Image *pImage = D3RE::get()->getImage(m_uiImageId);
    if(pImage != NULL) {
        renderImage(pImage);
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
D3HudRenderModel::moveBy(Point ptShift) {
    m_rcDrawArea.x += ptShift.x;
    m_rcDrawArea.y += ptShift.y;
    m_ptTextPos += ptShift;
    //z coord is discarded
}

void
D3HudRenderModel::updateDrawArea(const Rect &rc) {
    m_rcDrawArea = rc;
}

void
D3HudRenderModel::updateText(const std::string &data, float textSize) {
    if(textSize > 0.f) {
        m_fTextSize = textSize;
    } else if(m_fTextSize < 0.f) {
        m_fTextSize = 1.f;
    }
    Rect rcOldTextArea = TextRenderer::get()->getArea(m_sData.c_str(), 0.f, 0.f, m_fTextSize);
    m_sData = data;

    //Center the text in the box
    Rect rcNewTextArea = TextRenderer::get()->getArea(data.c_str(), 0.f, 0.f, m_fTextSize);

    if(m_bVertCenter) {
        m_ptTextPos.y -= rcNewTextArea.h / 2 - rcOldTextArea.h / 2;
    }
    if(m_bHorizCenter) {
        m_ptTextPos.x -= rcNewTextArea.w / 2 - rcOldTextArea.w / 2;
    }
}




void
D3HudRenderModel::centerVertically(bool bCenter) {
    if(m_bVertCenter && !bCenter) {
        //Shift the text so it is not centered
        Rect rcTextArea = TextRenderer::get()->getArea(m_sData.c_str(), 0.f, 0.f, m_fTextSize);
        m_ptTextPos.y -= m_rcDrawArea.h / 2 - rcTextArea.h / 2;
    } else if(!m_bVertCenter && bCenter) {
        //Shift the text so it is centered
        Rect rcTextArea = TextRenderer::get()->getArea(m_sData.c_str(), 0.f, 0.f, m_fTextSize);
        m_ptTextPos.y += m_rcDrawArea.h / 2 - rcTextArea.h / 2;
    }
    m_bVertCenter = bCenter;
}

void
D3HudRenderModel::centerHorizontally(bool bCenter) {
    if(m_bHorizCenter && !bCenter) {
        //Shift the text so it is NOT centered
        Rect rcTextArea = TextRenderer::get()->getArea(m_sData.c_str(), 0.f, 0.f, m_fTextSize);
        m_ptTextPos.x -= m_rcDrawArea.w / 2 - rcTextArea.w / 2;
    } else if(!m_bHorizCenter && bCenter) {
        //Shift the text so it IS centered
        Rect rcTextArea = TextRenderer::get()->getArea(m_sData.c_str(), 0.f, 0.f, m_fTextSize);
        m_ptTextPos.x += m_rcDrawArea.w / 2 - rcTextArea.w / 2;
    }
    m_bHorizCenter = bCenter;
}

void
D3HudRenderModel::renderImage(Image *pImage) {
    glPushMatrix();
    //D3RE::get()->prepHud();
    Color ourColor = m_crImageColor;

    //Render engine is responsible for resetting the camera
    float fTexLeft   = m_iFrameW * 1.0F / pImage->m_iNumFramesW,
          fTexTop    = m_iFrameH * 1.0F / pImage->m_iNumFramesH,
          fTexRight  = m_iFrameW * 1.0F / pImage->m_iNumFramesW + m_iRepsW * 1.0F / pImage->m_iNumFramesW,
          fTexBottom = m_iFrameH * 1.0F / pImage->m_iNumFramesH + m_iRepsH * 1.0F / pImage->m_iNumFramesH;

    //Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, pImage->m_uiTexture );
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
    glColor3f(1.f,1.f,1.f);
    if(!m_bHorizCenter) {
        TextRenderer::get()->splitText(splitData, m_rcDrawArea.w - margin * 2, m_fTextSize);
    }
    TextRenderer::get()->render(splitData.c_str(), m_ptTextPos.x, m_ptTextPos.y, m_fTextSize);
    glPopMatrix();
}
