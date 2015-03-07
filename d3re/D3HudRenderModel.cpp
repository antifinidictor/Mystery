/*
 * Source file for the Hud render model
 */

#include "d3re.h"
#include "mge/GameObject.h"
#include "TextRenderer.h"

D3HudRenderModel::D3HudRenderModel(uint uiImageId, const Rect &rcArea)
    :   m_uiImageId(uiImageId),
        m_filter(NULL),
        m_sData(""),
        m_rcDrawArea(rcArea),
        m_ptTextPos(rcArea.x, rcArea.y, 0.f),
        m_bVertCenter(false),
        m_bHorizCenter(false),
        m_iFrameW(0),
        m_iFrameH(0),
        m_iRepsW(1),
        m_iRepsH(1),
        m_crImageColor(0xFF, 0xFF, 0xFF)
{
}

D3HudRenderModel::D3HudRenderModel(const std::string &data, const Rect &rcArea, TextRenderer::CharacterFilter *filter)
    :   m_uiImageId(0),
        m_filter(filter),
        m_sData(data),
        m_rcDrawArea(rcArea),
        m_ptTextPos(rcArea.x, rcArea.y, 0.f),
        m_bVertCenter(false),
        m_bHorizCenter(false),
        m_iFrameW(0),
        m_iFrameH(0),
        m_iRepsW(1),
        m_iRepsH(1),
        m_crImageColor(0xFF, 0xFF, 0xFF)
{
    updateText(data);
}

D3HudRenderModel::D3HudRenderModel(uint uiImageId, const Rect &rcArea, const std::string &data, const Point &ptTextOffset, TextRenderer::CharacterFilter *filter)
    :   m_uiImageId(uiImageId),
        m_filter(filter),
        m_sData(data),
        m_rcDrawArea(rcArea),
        m_ptTextPos(ptTextOffset.x + rcArea.x, ptTextOffset.y + rcArea.y, ptTextOffset.z),
        m_bVertCenter(false),
        m_bHorizCenter(false),
        m_iFrameW(0),
        m_iFrameH(0),
        m_iRepsW(1),
        m_iRepsH(1),
        m_crImageColor(0xFF, 0xFF, 0xFF)
{
    updateText(data);
}

D3HudRenderModel::~D3HudRenderModel() {
    if(m_filter != NULL) {
        delete m_filter;
    }
}

void
D3HudRenderModel::render(RenderEngine *re) {
    Image *pImage = D3RE::get()->getImage(m_uiImageId);
    if(pImage != NULL) {
        renderImage(pImage);
    }
    if(m_filter != NULL) {
        renderText();
    }
}

Rect
D3HudRenderModel::getDrawArea() {
    return m_rcDrawArea;
}

Point
D3HudRenderModel::getPosition() {
    Point ptPos = getParentPosition();
    return Point(m_rcDrawArea.x + m_rcDrawArea.w / 2 + ptPos.x, m_rcDrawArea.y + m_rcDrawArea.h / 2 + ptPos.y, ptPos.z);
}

void
D3HudRenderModel::moveBy(const Point &ptShift) {
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
D3HudRenderModel::updateFilter(TextRenderer::CharacterFilter *filter) {
    //Clear out the old filter and insert the knew one
    if(m_filter != NULL) {
        delete m_filter;
    }
    m_filter = filter;
    updateText(m_sData);
}

void
D3HudRenderModel::updateText(const std::string &data) {
    Rect rcOldTextArea = TextRenderer::get()->getArea(m_sData, 0.f, 0.f, m_filter);
    m_sData = data;

    //Center the text in the box
    Rect rcNewTextArea = TextRenderer::get()->getArea(data, 0.f, 0.f, m_filter);

    if(m_bVertCenter) {
        m_ptTextPos.y -= rcNewTextArea.h / 2 - rcOldTextArea.h / 2;
    }
    if(m_bHorizCenter) {
        m_ptTextPos.x -= rcNewTextArea.w / 2 - rcOldTextArea.w / 2;
    }

    float margin = m_ptTextPos.x - m_rcDrawArea.x;
    if(!m_bHorizCenter) {
        TextRenderer::get()->splitText(m_sData, m_rcDrawArea.w - margin * 2, m_filter);
    }
}

void
D3HudRenderModel::centerVertically(bool bCenter) {
    if(m_bVertCenter && !bCenter) {
        //Shift the text so it is not centered
        Rect rcTextArea = TextRenderer::get()->getArea(m_sData, 0.f, 0.f, m_filter);
        m_ptTextPos.y -= m_rcDrawArea.h / 2 - rcTextArea.h / 2;
    } else if(!m_bVertCenter && bCenter) {
        //Shift the text so it is centered
        Rect rcTextArea = TextRenderer::get()->getArea(m_sData, 0.f, 0.f, m_filter);
        m_ptTextPos.y += m_rcDrawArea.h / 2 - rcTextArea.h / 2;
    }
    m_bVertCenter = bCenter;
}

void
D3HudRenderModel::centerHorizontally(bool bCenter) {
    if(m_bHorizCenter && !bCenter) {
        //Shift the text so it is NOT centered
        Rect rcTextArea = TextRenderer::get()->getArea(m_sData, 0.f, 0.f, m_filter);
        m_ptTextPos.x -= m_rcDrawArea.w / 2 - rcTextArea.w / 2;
    } else if(!m_bHorizCenter && bCenter) {
        //Shift the text so it IS centered
        Rect rcTextArea = TextRenderer::get()->getArea(m_sData, 0.f, 0.f, m_filter);
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

    Point ptPos = getParentPosition();
    glTranslatef(ptPos.x, ptPos.y, ptPos.z);

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

    Point ptPos = getParentPosition();
    glTranslatef(ptPos.x, ptPos.y, ptPos.z);

    glColor3f(1.f,1.f,1.f);
    //Old pos of splitting text
    TextRenderer::get()->render(m_sData.c_str(), m_ptTextPos.x, m_ptTextPos.y, m_filter);
    glPopMatrix();
}
