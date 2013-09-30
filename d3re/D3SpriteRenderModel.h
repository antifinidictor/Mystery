/*
 * Render model for sprite objects
 */
#ifndef D3_SPRITE_RENDER_MODEL_H
#define D3_SPRITE_RENDER_MODEL_H

#include "mge/RenderModel.h"
#include "mge/defs.h"
#include "mge/Image.h"

class D3SpriteRenderModel : public RenderModel {
public:
    D3SpriteRenderModel(GameObject *parent, uint uiImageId, Rect rcArea);
    virtual ~D3SpriteRenderModel();

    virtual void render(RenderEngine *re);
    virtual void moveBy(Point ptShift) {}
    virtual Point getPosition();
    virtual Rect getDrawArea();

    //Image *getTexture() { return m_pImage; }

    void setFrameW(int fw) { m_iFrameW = fw; }
    void setFrameH(int fh) { m_iFrameH = fh; }
    void setRepsW(int rw)  { m_iRepsW = rw; }
    void setRepsH(int rh)  { m_iRepsH = rh; }

    void setColor(const Color &cr) { m_crColor = cr; }
    Color &getColor() { return m_crColor; }

private:
    uint m_uiImageId;
    Rect m_rcDrawArea;

    int m_iFrameW, m_iFrameH;
    int m_iRepsW, m_iRepsH;

    Color m_crColor;

    GameObject *m_pParent;
};

#endif
