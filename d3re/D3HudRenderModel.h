/*
 * Render model for Hud objects
 */
#ifndef D3_HUD_RENDER_MODEL_H
#define D3_HUD_RENDER_MODEL_H

#include "mge/RenderModel.h"
#include "mge/defs.h"
#include "mge/Image.h"

class D3HudRenderModel : public RenderModel {
public:
    D3HudRenderModel(uint uiImageId, const Rect &rcArea);
    D3HudRenderModel(const std::string &data, const Rect &rcArea, float textSize = 1.f);
    D3HudRenderModel(uint uiImageId, const Rect &rcArea, const std::string &data, const Point &ptTextOffset, float textSize = 1.f);
    virtual ~D3HudRenderModel();

    virtual void render(RenderEngine *re);
    virtual void moveBy(Point ptShift);
    virtual Point getPosition();
    virtual Rect getDrawArea();

    void setFrameW(int fw) { m_iFrameW = fw; }
    void setFrameH(int fh) { m_iFrameH = fh; }
    void setRepsW(int rw)  { m_iRepsW = rw; }
    void setRepsH(int rh)  { m_iRepsH = rh; }

    int getFrameW() { return m_iFrameW; }
    int getFrameH() { return m_iFrameH; }
    int getRepsW()  { return m_iRepsW; }
    int getRepsH()  { return m_iRepsH; }

    void setImageColor(const Color &cr) { m_crImageColor = cr; }
    Color &getImageColor() { return m_crImageColor; }

    void updateDrawArea(const Rect &rc);
    void updateText(const std::string &data, float textSize = -1.f);
    std::string getText() { return m_sData; }

    void centerVertically(bool bCenter);
    void centerHorizontally(bool bCenter);

private:
    void renderImage(Image *pImage);
    void renderText();

    uint m_uiImageId;

    std::string m_sData;
    float m_fTextSize;

    Rect m_rcDrawArea;
    Point m_ptTextPos;

    bool m_bVertCenter;
    bool m_bHorizCenter;

    int m_iFrameW, m_iFrameH;
    int m_iRepsW, m_iRepsH;

    Color m_crImageColor;
};

#endif
