/*
 * The D3RenderEngine
 */

#ifndef D3_RENDER_ENGINE_H
#define D3_RENDER_ENGINE_H

#include <list>
#include <vector>

#include "mge/RenderEngine.h"
#include "mge/Image.h"

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

    void resetCamera();
    void resize(uint width, uint height);

private:
    D3RenderEngine();
    virtual ~D3RenderEngine();

    static D3RenderEngine *re;
    Point m_ptPos;
    Color m_crWorld;
    float m_camDist;
    std::list<GameObject*> m_lsObjsOnScreen;
    std::vector<Image*> m_vImages;
};

typedef D3RenderEngine D3RE;

#endif
