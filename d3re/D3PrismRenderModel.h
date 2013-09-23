/*
 * Render model for sprite objects
 */
#ifndef D3_PRISM_RENDER_MODEL_H
#define D3_PRISM_RENDER_MODEL_H

#include "mge/RenderModel.h"
#include "mge/defs.h"
#include "mge/Image.h"

class D3PrismRenderModel : public RenderModel {
public:
    D3PrismRenderModel(GameObject *parent, Box bxVolume);
    virtual ~D3PrismRenderModel();

    virtual void render(RenderEngine *re);
    virtual void moveBy(Point ptShift) {}
    virtual Point getPosition();
    virtual Rect getDrawArea();
    
    void setTexture(int iFace, uint uiTexId);   //Assumes values are valid!

private:
    void renderFaceX(uint texId, float x, float y, float z0, float z1, float fRepsW, float fRepsH); //Left/Right faces
    void renderFaceY(uint texId, float x, float y, float z0, float z1, float fRepsW, float fRepsH); //Top/bottom faces
    void renderFaceZ(uint texId, float x, float y0, float y1, float z, float fRepsW, float fRepsH); //Front/back faces    

    uint m_aTextures[6];    //One for each side
    Box m_bxVolume;

    GameObject *m_pParent;
};

#endif
