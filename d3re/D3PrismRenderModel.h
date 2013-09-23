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
    void renderFace(uint texId, const Point &tl, const Point &tr, const Point &bl, const Point &br);

    uint m_aTextures[6];    //One for each side
    Box m_bxVolume;

    GameObject *m_pParent;
};

#endif
