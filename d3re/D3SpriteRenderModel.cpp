/*
 * Source file for the sprite render model
 */

#include "d3re.h"
#include "mge/GameObject.h"

D3SpriteRenderModel::D3SpriteRenderModel(GameObject *parent, uint uiImageId, Rect rcArea) {

    m_uiImageId = uiImageId;
    m_rcDrawArea = rcArea;

    m_iFrameW = 0;
    m_iFrameH = 0;
    m_iRepsW = 1;
    m_iRepsH = 1;

    m_crColor = Color(0xFF, 0xFF, 0xFF);

    m_pParent = parent;
}

D3SpriteRenderModel::~D3SpriteRenderModel() {
}

void
D3SpriteRenderModel::render(RenderEngine *re) {
    D3RE::get()->prepCamera();
    Color worldColor = D3RE::get()->getWorldColor();
    Color ourColor = mix(2, &worldColor, &m_crColor);

    Image *pImage = D3RE::get()->getImage(m_uiImageId);
    if(pImage == NULL) return;

    //Render engine is responsible for resetting the camera
    float fTexLeft   = m_iFrameW * 1.0F / pImage->m_iNumFramesW,
          fTexTop    = m_iFrameH * 1.0F / pImage->m_iNumFramesH,
          fTexRight  = m_iFrameW * 1.0F / pImage->m_iNumFramesW + m_iRepsW * 1.0F / pImage->m_iNumFramesW,
          fTexBottom = m_iFrameH * 1.0F / pImage->m_iNumFramesH + m_iRepsH * 1.0F / pImage->m_iNumFramesH;

    Point ptPos = getPosition();
    glTranslatef((int)(ptPos.x + m_rcDrawArea.x), (int)(ptPos.y + m_rcDrawArea.y), (int)(ptPos.z));

    //Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, pImage->m_uiTexture );
    //glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);
        glColor3f(ourColor.r / 255.f, ourColor.g / 255.f, ourColor.b / 255.f);
        //Top-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexTop);
        glVertex3f(0.f, m_rcDrawArea.h, 0.f);

        //Top-right vertex (corner)
        glTexCoord2f(fTexRight, fTexTop);
        glVertex3f(m_rcDrawArea.w, m_rcDrawArea.h, 0.f);

        //Bottom-right vertex (corner)
        glTexCoord2f(fTexRight, fTexBottom);
        glVertex3f(m_rcDrawArea.w, 0.f, 0.f);

        //Bottom-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexBottom);
        glVertex3f(0.f, 0.f, 0.f);
    glEnd();
    //glDepthMask(GL_TRUE);
}

Rect
D3SpriteRenderModel::getDrawArea() {
    Point ptPos = getPosition();
    return Rect(ptPos.x + m_rcDrawArea.x, ptPos.y + m_rcDrawArea.y, m_rcDrawArea.w, m_rcDrawArea.h);
}

Point
D3SpriteRenderModel::getPosition() {
    Point ptPos = Point();
    if(m_pParent != NULL) {
        ptPos = m_pParent->getPhysicsModel()->getPosition();
    }
    return ptPos;
}
