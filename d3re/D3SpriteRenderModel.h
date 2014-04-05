/*
 * Render model for sprite objects
 */
#ifndef D3_SPRITE_RENDER_MODEL_H
#define D3_SPRITE_RENDER_MODEL_H

#include "mge/RenderModel.h"
#include "mge/defs.h"
#include "mge/Image.h"
#include "mge/Positionable.h"

class D3SpriteRenderModel : public RenderModel {
public:
    D3SpriteRenderModel(Positionable *parent, uint uiImageId, Rect rcArea);
    virtual ~D3SpriteRenderModel();

    virtual void render(RenderEngine *re);
    //virtual void moveBy(const Point &ptShift) {}
    //virtual Point getPosition();
    //virtual Rect getDrawArea();

    //Image *getTexture() { return m_pImage; }

    void setFrameW(int fw) { m_iFrameW = fw; }
    void setFrameH(int fh) { m_iFrameH = fh; }
    void setRepsW(int rw)  { m_iRepsW = rw; }
    void setRepsH(int rh)  { m_iRepsH = rh; }


    int getFrameW() { return m_iFrameW; }
    int getFrameH() { return m_iFrameH; }
    int getRepsW()  { return m_iRepsW; }
    int getRepsH()  { return m_iRepsH; }

    uint getImageId() { return m_uiImageId; }

    void setColor(const Color &cr) { m_crColor = cr; }
    Color &getColor() { return m_crColor; }

private:
    void billboardEnd();
    void billboardSphericalBegin(
			float camX, float camY, float camZ,
			float objPosX, float objPosY, float objPosZ
    );
    uint m_uiImageId;
    Rect m_rcDrawArea;

    int m_iFrameW, m_iFrameH;
    int m_iRepsW, m_iRepsH;

    Color m_crColor;

    Positionable *m_pParent;
};

#endif
