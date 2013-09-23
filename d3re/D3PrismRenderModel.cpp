/*
 * Source file for the sprite render model
 */

#include "d3re.h"
#include "mge/GameObject.h"

D3PrismRenderModel::D3PrismRenderModel(GameObject *parent, Box bxVolume) {
    m_bxVolume = bxVolume;
    m_pParent = parent;

    for(uint i = 0; i < 6; ++i) {
        m_aTextures[i] = 0;
    }
}


D3PrismRenderModel::~D3PrismRenderModel() {
}

void
D3PrismRenderModel::setTexture(int iFace, uint uiTexId) {
    m_aTextures[iFace] = uiTexId;
}

void
D3PrismRenderModel::render(RenderEngine *re) {
    Color worldColor = D3RE::get()->getWorldColor();
    Color ourColor = Color(worldColor); //Eventually there may be a mix done in here

    Point ptPos = getPosition();
    glTranslatef((int)(ptPos.x + m_bxVolume.x), (int)(ptPos.y + m_bxVolume.y), (int)(ptPos.z + m_bxVolume.z));

    //glColor3f(ourColor.r / 255.f, ourColor.g / 255.f, ourColor.b / 255.f);
    glColor3f(1.0f, 0.5f, 0.5f);

    // Top face (y = v.y)
    renderFaceY(
        m_aTextures[UP],    //Texture id to bind
        m_bxVolume.w,       //x-value
        m_bxVolume.l,       //y-value
        0.f,                //z0: 1st two z values
        m_bxVolume.h,       //z1: 2nd two z values
        1.f,                //reps wide
        1.f                 //reps high
    );

    // Bottom face (y = 0.f)
    renderFaceY(
        m_aTextures[DOWN],    //Texture id to bind
        m_bxVolume.w,       //x-value
        0.f,                //y-value
        m_bxVolume.h,       //z0: 1st two z values
        0.f,                //z1: 2nd two z values
        1.f,                //reps wide
        1.f                 //reps high
    );

    // Front face  (z = v.z)
    renderFaceZ(
        m_aTextures[SOUTH],    //Texture id to bind
        m_bxVolume.w,       //x-value
        m_bxVolume.l,       //y0: 1st two y values
        0.f,                //y1: 2nd two y values
        m_bxVolume.h,       //z-value
        1.f,                //reps wide
        1.f                 //reps high
    );

    // Back face (z = 0.f)
    renderFaceZ(
        m_aTextures[NORTH],    //Texture id to bind
        m_bxVolume.w,       //x-value
        0.f,                //y0: 1st two y values
        m_bxVolume.l,       //y1: 2nd two y values
        0.f,                //z-value
        1.f,                //reps wide
        1.f                 //reps high
    );

    // Left face (x = 0.f)
    renderFaceX(
        m_aTextures[WEST],    //Texture id to bind
        0.f,                //x-value
        m_bxVolume.l,       //y-value
        m_bxVolume.h,       //z0: first and last z values
        0.f,                //z1: middle z values
        1.f,                //reps wide
        1.f                 //reps high
    );

    // Right face (x = 1.0f)
    renderFaceX(
        m_aTextures[EAST],    //Texture id to bind
        m_bxVolume.w,       //x-value
        m_bxVolume.l,       //y-value
        0.f,                //z0: first and last z values
        m_bxVolume.h,       //z1: middle z values
        1.f,                //reps wide
        1.f                 //reps high
    );
}

void
D3PrismRenderModel::renderFaceX(uint texId, float x, float y, float z0, float z1, float fRepsW, float fRepsH) {
    Image *tex = D3RE::get()->getImage(texId);
    if(tex == NULL) return;

    float fTexLeft, fTexTop, fTexRight, fTexBottom;

    //Tiled texture
    fTexLeft   = 0;
    fTexTop    = 0;
    fTexRight  = fRepsW;
    fTexBottom = fRepsH;


    glBindTexture(GL_TEXTURE_2D, tex->m_uiTexture);

    
    //Must be inside a glBegin() and glEnd() pair
    glBegin(GL_QUADS);
        glTexCoord2f(fTexLeft,  fTexTop);
        glVertex3f(x, y,   z0);

        glTexCoord2f(fTexRight, fTexTop);
        glVertex3f(x, y,   z1);

        glTexCoord2f(fTexRight, fTexBottom);
        glVertex3f(x, 0.f, z1);

        glTexCoord2f(fTexLeft,  fTexBottom);
        glVertex3f(x, 0.f, z0);
    glEnd();
}

void
D3PrismRenderModel::renderFaceY(uint texId, float x, float y, float z0, float z1, float fRepsW, float fRepsH) {
    Image *tex = D3RE::get()->getImage(texId);
    if(tex == NULL) return;

    float fTexLeft, fTexTop, fTexRight, fTexBottom;

    //Tiled texture
    fTexLeft   = 0;
    fTexTop    = 0;
    fTexRight  = fRepsW;
    fTexBottom = fRepsH;


    glBindTexture(GL_TEXTURE_2D, tex->m_uiTexture);

    //Must be inside a glBegin() and glEnd() pair
    glBegin(GL_QUADS);
        glTexCoord2f(fTexLeft,  fTexTop);
        glVertex3f(x,   y, z0);

        glTexCoord2f(fTexRight, fTexTop);
        glVertex3f(0.f, y, z0);

        glTexCoord2f(fTexRight, fTexBottom);
        glVertex3f(0.f, y, z1);

        glTexCoord2f(fTexLeft,  fTexBottom);
        glVertex3f(x,   y, z1);
    glEnd();
}

void
D3PrismRenderModel::renderFaceZ(uint texId, float x, float y0, float y1, float z, float fRepsW, float fRepsH) {
    Image *tex = D3RE::get()->getImage(texId);
    if(tex == NULL) return;

    float fTexLeft, fTexTop, fTexRight, fTexBottom;

    //Tiled texture
    fTexLeft   = 0;
    fTexTop    = 0;
    fTexRight  = fRepsW;
    fTexBottom = fRepsH;

    glBindTexture(GL_TEXTURE_2D, tex->m_uiTexture);

    //Must be inside a glBegin() and glEnd() pair
    glBegin(GL_QUADS);
        glTexCoord2f(fTexLeft,  fTexTop);
        glVertex3f(x,   y0, z);

        glTexCoord2f(fTexRight, fTexTop);
        glVertex3f(0.f, y0, z);

        glTexCoord2f(fTexRight, fTexBottom);
        glVertex3f(0.f, y1, z);

        glTexCoord2f(fTexLeft,  fTexBottom);
        glVertex3f(x,   y1, z);
    glEnd();
}

Rect
D3PrismRenderModel::getDrawArea() {
    Point ptPos = getPosition();
    return m_bxVolume + ptPos;
}

Point
D3PrismRenderModel::getPosition() {
    Point ptPos = Point();
    if(m_pParent != NULL) {
        ptPos = m_pParent->getPhysicsModel()->getPosition();
    }
    return ptPos;
}
