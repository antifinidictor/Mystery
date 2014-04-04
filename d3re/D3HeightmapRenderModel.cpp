/*
 * Source file for the sprite render model
 */

#include "d3re.h"
#include "mge/GameObject.h"

D3HeightmapRenderModel::D3HeightmapRenderModel(GameObject *parent, uint uiTexture, const PixelMap *pxMap, Box bxVolume)
    : m_uiTexture(uiTexture),
      m_pxMap(pxMap),
      m_crColor(0xFF, 0xFF, 0xFF),
      m_pParent(parent)
{
}


D3HeightmapRenderModel::~D3HeightmapRenderModel() {
}

void
D3HeightmapRenderModel::render(RenderEngine *re) {
    glPushMatrix();

    //Render collision box
    if(D3RE::get()->getDrawCollisions()) {
        D3RE::get()->drawBox(m_pParent->getPhysicsModel()->getCollisionVolume(), m_crColor);
    }

    Image *tex = D3RE::get()->getImage(m_uiTexture);
    if(tex == NULL) return;

    Color worldColor = D3RE::get()->getWorldColor();
    float fWeight = D3RE::get()->getColorWeight();
    Color ourColor = Color(m_crColor.r * (1 - fWeight) + worldColor.r * fWeight,
                           m_crColor.g * (1 - fWeight) + worldColor.g * fWeight,
                           m_crColor.b * (1 - fWeight) + worldColor.b * fWeight);

    Box bxVolume = m_pParent->getPhysicsModel()->getCollisionVolume();

    //glTranslatef(ptPos.x, ptPos.y, ptPos.z);
    glTranslatef(bxVolume.x, bxVolume.y, bxVolume.z);

    glColor3f(ourColor.r / 255.f, ourColor.g / 255.f, ourColor.b / 255.f);

    glBindTexture(GL_TEXTURE_2D, tex->m_uiTexture);

    //Render heightmap
    float w = bxVolume.w / (m_pxMap->m_uiW - 1);  //x resolution (width of one unit)
    float l = bxVolume.l / (m_pxMap->m_uiH - 1);  //y resolution (length of one unit)
    for(uint x = 0; x < m_pxMap->m_uiW - 1; ++x) {
        glBegin(GL_TRIANGLE_STRIP);
        for(uint z = 0; z < m_pxMap->m_uiH; z++) {
            float y0 = m_pxMap->m_pData[x][z] * (bxVolume.h);
            float y1 = m_pxMap->m_pData[x+1][z] * (bxVolume.h);
            //printf("(%f,%f) -> %f, (%f,%f) -> %f");
            glTexCoord2f(x * w, z * l);
            glVertex3f(x * w, y0, z * l);

            glTexCoord2f(x * w + w, z * l);
            glVertex3f(x * w + w, y1, z * l);
        }

        glTexCoord2f(x * w, (m_pxMap->m_uiH - 1) * l);
        //glVertex3f(x * w, bxVolume.y, (m_pxMap->m_uiH - 1) * l);
        glVertex3f(x * w, 0, (m_pxMap->m_uiH - 1) * l);

        glTexCoord2f(x * w + w, (m_pxMap->m_uiH - 1) * l);
        //glVertex3f(x * w + w, bxVolume.y, (m_pxMap->m_uiH - 1) * l);
        glVertex3f(x * w + w, 0, (m_pxMap->m_uiH - 1) * l);
        glEnd();
    }

    //Render edges
    uint x, z;

    //left edge
    glBegin(GL_QUAD_STRIP);
    x = 0;
    for(z = 0; z < m_pxMap->m_uiH; ++z) {
        float y = m_pxMap->m_pData[x][z] * (bxVolume.h);

        glTexCoord2f(z * l, 1);
        glVertex3f(x * w, 0, z * l);

        glTexCoord2f(z * l, 1 - y / bxVolume.h);
        glVertex3f(x * w, y, z * l);
    }
    glEnd();

    //right edge
    glBegin(GL_QUAD_STRIP);
    x = (m_pxMap->m_uiW - 1);
    for(z = 0; z < m_pxMap->m_uiH; ++z) {
        float y = m_pxMap->m_pData[x][z] * (bxVolume.h);

        glTexCoord2f(z * l, 1);
        glVertex3f(x * w, 0, z * l);

        glTexCoord2f(z * l, 1 - y / bxVolume.h);
        glVertex3f(x * w, y, z * l);
    }
    glEnd();

    //south edge
    glBegin(GL_QUAD_STRIP);
    z = (m_pxMap->m_uiH - 1);
    for(x = 0; x < m_pxMap->m_uiW; ++x) {
        float y = m_pxMap->m_pData[x][z] * (bxVolume.h);

        glTexCoord2f(x * w, 1);
        glVertex3f(x * w, 0, z * l);

        glTexCoord2f(x * w, 1 - y / bxVolume.h);
        glVertex3f(x * w, y, z * l);
    }
    glEnd();

    //north edge
    glBegin(GL_QUAD_STRIP);
    z = 0;
    for(x = 0; x < m_pxMap->m_uiW; ++x) {
        float y = m_pxMap->m_pData[x][z] * (bxVolume.h);

        glTexCoord2f(x * w, 1);
        glVertex3f(x * w, 0, z * l);

        glTexCoord2f(x * w, 1 - y / bxVolume.h);
        glVertex3f(x * w, y, z * l);
    }
    glEnd();
    glPopMatrix();
}


Rect
D3HeightmapRenderModel::getDrawArea() {
    return m_pParent->getPhysicsModel()->getCollisionVolume();
}

Point
D3HeightmapRenderModel::getPosition() {
    Point ptPos = Point();
    if(m_pParent != NULL) {
        ptPos = m_pParent->getPhysicsModel()->getPosition();
    }
    return ptPos;
}
