/*
 * The D3RenderEngine
 */

#ifndef D3_RENDER_ENGINE_H
#define D3_RENDER_ENGINE_H

#include <list>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

#include "mge/RenderEngine.h"
#include "mge/Image.h"
#include "mge/defs.h"
#include "mge/Event.h"

#include "ContainerRenderModel.h"

class D3HudRenderModel;
class D3SpriteRenderModel;

enum RenderFlags {
    D3RE_ON_SCREEN = RENDER_FLAGS_BEGIN,
    D3RE_INVISIBLE,
    D3RE_TRANSPARENT,
    D3RE_NUM_FLAGS
};

class D3RenderEngine : public RenderEngine, public Listener {
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

    Image *createImage(uint id, const std::string &imageName, const std::string &fileName, int numFramesH = 1, int numFramesW = 1, bool bLinearInterp = false);
    Image *createImage(uint id, const std::string &fileName, int numFramesH = 1, int numFramesW = 1, bool bLinearInterp = false);
    Image *getImage(uint id);
    Image *getImage(const std::string &imageName);
    uint getImageId(const std::string &imageName);
    void   freeImage(uint id);

    const Color& getWorldColor() const { return m_crWorld; }
    float getColorWeight() { return m_fColorWeight; }
    void setWorldColor(const Color &cr) { m_crWorld = cr; }
    void setColorWeight(float fWeight) { m_fColorWeight = fWeight; }
    void setBackgroundColor(const Color &cr);

    void adjustCamDist(float delta);
    void adjustCamAngle(float delta);
    void setLookAngle(float fLookAngle) { m_fDesiredLookAngle = fLookAngle; }
    void adjustLookAngle(float fLookAngle) { m_fDesiredLookAngle += fLookAngle; }
    float getLookAngle() { return m_fLookAngle; }
    float getDesiredLookAngle() { return m_fDesiredLookAngle; }

    void prepCamera();
    void prepHud();
    void resize(uint width, uint height);
    Point getCameraPosition() { return m_ptCamPos; }

    void drawBox(const Box &bxVolume, const Color &cr = Color(0xFF, 0xFF, 0xFF));

    void drawCircle(const Point &ptCenter, float radius, const Color &cr = Color(0xFF, 0xFF, 0xFF));


    ContainerRenderModel *getHudContainer() { return m_pHudContainer; }

    uint getScreenWidth() { return m_uiWidth; }
    uint getScreenHeight() { return m_uiHeight; }

    void write(boost::property_tree::ptree &pt, const std::string &keyBase);
    void read(boost::property_tree::ptree &pt, const std::string &keyBase);

    uint getNumImages() { return m_vImages.size(); }

	virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId);
	virtual uint getId() { return ID_RENDER_ENGINE; }

	Point getMousePos() { return m_ptMouseInWorld; }

	void setDrawCollisions(bool enable) { m_bDrawCollisions = enable; }
	bool getDrawCollisions() { return m_bDrawCollisions; }

    void showRealMouse();
    void hideRealMouse();
    GameObject *getMouseOverObject() { return m_pMouseOverObject; }
    Point getMouseRay() { return m_v3MouseRay; }

private:
    D3RenderEngine();
    virtual ~D3RenderEngine();

    void enableCameraMode();
    void enableGuiMode();

    void updateCamPos();
    void addInOrder(GameObject *obj);
    void resort(GameObject *obj);

    bool comesBefore(GameObject *obj1, GameObject *obj2);
    void updateMousePos(int x, int y);


    void drawBoxNow(const Box &bxVolume, const Color &cr = Color(0xFF, 0xFF, 0xFF));
    void drawCircleNow(const Point &ptCenter, float radius, const Color &cr = Color(0xFF, 0xFF, 0xFF));

    static D3RenderEngine *re;
    Point m_ptPos, m_ptCamPos;
    float m_fCamDist, m_fCamAngle;
    float m_fLookAngle, m_fDesiredLookAngle;    //Angle at which the camera looks
    Color m_crWorld;
    float m_fColorWeight;
//    uint m_uiMouseFrame, m_uiMouseTimer;
    uint m_uiWidth, m_uiHeight;

    struct DrawBoxInfo {
        Box m_bx;
        Color m_cr;
        DrawBoxInfo(const Box &bx, const Color &cr) : m_bx(bx), m_cr(cr) {}
    };

    struct DrawCircleInfo {
        float m_rad;
        Point m_pt;
        Color m_cr;
        DrawCircleInfo(const Point &pt, float rad, const Color &cr) : m_rad(rad), m_pt(pt), m_cr(cr) {}
    };

    std::list<GameObject *> m_lsObjsOnScreen;
    std::list<GameObject *> m_lsTransparentObjs;
    std::list<DrawBoxInfo> m_lsBoxesToDraw;
    std::list<DrawCircleInfo> m_lsCirclesToDraw;
    std::vector<Image*> m_vImages;
    std::map<std::string, uint> m_mImageNameToId;
    SDL_mutex *m_mxDrawPrimitive;
    ContainerRenderModel *m_pHudContainer;
    bool m_bGuiMode;
    bool m_bDrawCollisions;
    bool m_bDrawRealMouse;

    //Mouse animations
    GameObject *m_pMouseOverObject;

    int m_iMouseX, m_iMouseY;
    Point m_ptMouseInWorld;
    Vec3f m_v3MouseRay;
    SDL_Window *m_sdlWindow;
    SDL_GLContext m_glContext;
};

typedef D3RenderEngine D3RE;

#endif
