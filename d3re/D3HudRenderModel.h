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
    D3HudRenderModel(Image *img, const Rect &rcArea);
    D3HudRenderModel(const std::string &data, const Rect &rcArea, float textSize = 1.f);
    D3HudRenderModel(Image *img, const Rect &rcArea, const std::string &data, const Point &ptTextOffset, float textSize = 1.f);
    virtual ~D3HudRenderModel();

    virtual void render(RenderEngine *re);
    virtual void moveBy(Point ptShift) {}
    virtual Point getPosition();
    virtual Rect getDrawArea();

    void setFrameW(int fw) { m_iFrameW = fw; }
    void setFrameH(int fh) { m_iFrameH = fh; }
    void setRepsW(int rw)  { m_iRepsW = rw; }
    void setRepsH(int rh)  { m_iRepsH = rh; }

    void setImageColor(const Color &cr) { m_crImageColor = cr; }
    Color &getImageColor() { return m_crImageColor; }

    void updateText(const std::string &data, float textSize = -1.f);

private:
    void renderImage();
    void renderText();

    Image *m_pImage;

    std::string m_sData;
    float m_fTextSize;

    Rect m_rcDrawArea;
    Point m_ptTextPos;

    int m_iFrameW, m_iFrameH;
    int m_iRepsW, m_iRepsH;

    Color m_crImageColor;
};

#endif
