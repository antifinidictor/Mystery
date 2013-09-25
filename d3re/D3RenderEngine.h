/*
 * The D3RenderEngine
 */

#ifndef D3_RENDER_ENGINE_H
#define D3_RENDER_ENGINE_H

#include <map>
#include <vector>

#include "mge/RenderEngine.h"
#include "mge/Image.h"
#include "mge/defs.h"

class D3HudRenderModel;

enum RenderFlags {
    D3RE_ON_SCREEN = RENDER_FLAGS_BEGIN,
    D3RE_INVISIBLE,
    D3RE_NUM_FLAGS
};

class D3RenderEngine : public RenderEngine {
public:
    static void init()  { re = new D3RenderEngine(); }
    static void clean() { delete re; }
    static D3RenderEngine *get() { return re; }

    virtual void  render();
    virtual Point getRenderOffset() { return m_ptPos; }
    virtual bool  screenHasMoved() { return true; }
    virtual void  manageObjOnScreen(GameObject *obj);
    virtual void  remove(GameObject *obj);
    virtual void  clearScreen();
    virtual void  moveScreenTo(Point pt);
    void moveScreenBy(Point pt);

    Image *createImage(uint id, const char *name, int numFramesH = 1, int numFramesW = 1);
    Image *getImage(uint id);
    void   freeImage(uint id);

    const Color& getWorldColor() const { return m_crWorld; }
    void setWorldColor(const Color &cr) { m_crWorld = cr; }

    void adjustCamDist(float delta);
    void adjustCamAngle(float delta);

    void prepCamera();
    void prepHud();
    void resize(uint width, uint height);
    Point getCameraPosition() { return m_ptCamPos; }

    void drawBox(const Box &bxVolume, const Color &cr = Color(0xFF, 0xFF, 0xFF));

    void setBackgroundColor(const Color &cr);
    void addHudElement(uint uiHudId, D3HudRenderModel *hud) { m_mHudElements[uiHudId] = hud; }
    void clearHud();

private:
    D3RenderEngine();
    virtual ~D3RenderEngine();

    void enableCameraMode();
    void enableGuiMode();

    void updateCamPos();
    void addInOrder(GameObject *obj);
    void resort(GameObject *obj);

    static D3RenderEngine *re;
    Point m_ptPos, m_ptCamPos;
    Color m_crWorld;
    float m_fCamDist, m_fCamAngle;
    uint m_uiWidth, m_uiHeight;

    std::map<float, GameObject *> m_mObjsOnScreen;
    std::map<uint, D3HudRenderModel *> m_mHudElements;
    std::vector<Image*> m_vImages;
    bool m_bGuiMode;
};

typedef D3RenderEngine D3RE;

#endif
