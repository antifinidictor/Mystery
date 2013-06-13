#ifndef ORDERED_RENDER_ENGINE_H
#define ORDERED_RENDER_ENGINE_H

#include <vector>
#include <list>
#include <map>

#include "mge/RenderEngine.h"
#include "mge/defs.h"
#include "mge/RenderModel.h"
#include "mge/Image.h"

//Image layer interpretation.  FX layers are reserved for FX that need to be above/below all objects in a layer, such as fog/shadows.
#define ORE_LAYER_SURFACE 0
#define ORE_LAYER_LOW_FX  1
#define ORE_LAYER_OBJECTS 2
#define ORE_LAYER_HIGH_FX 3

enum RenderFlags {
    ORE_ON_SCREEN = RENDER_FLAGS_BEGIN,
    ORE_INVISIBLE,
    ORE_NUM_FLAGS
};

class OrderedRenderEngine : public RenderEngine {
public:
    static void init()  { bre = new OrderedRenderEngine(); }
    static void clean() { delete bre; }
    static OrderedRenderEngine *get() { return bre; }

    virtual void  render();
    virtual Point getRenderOffset() { return Point(m_rcScreenArea); }
    virtual void  manageObjOnScreen(GameObject *obj);
    virtual bool  screenHasMoved() { return true; }
    virtual void  moveScreenTo(Point pt);
    virtual void  remove(GameObject *obj);
    virtual void  clearScreen();

    Image *createImage(const char *name, int numFramesH = 1, int numFramesW = 1);
    Image *getImage(uint id);
    Image *getMappedImage(uint mapValue);
    void   freeImage(uint id);
    void   mapStandardImage(uint mapValue, uint id);
    GameObject *getObjAtPos(float x, float y);  //get topmost object that could be clicked on

protected:
private:
    OrderedRenderEngine();
    virtual ~OrderedRenderEngine();

    static OrderedRenderEngine *bre;
    
    void addInOrder(GameObject *obj);
    void resort(GameObject *obj);
    bool comesBefore(GameObject *obj1, GameObject *obj2);

    std::list<GameObject*> m_lsObjsOnScreen;
    std::vector<Image*> m_vImages;
    std::map<uint,uint> m_mStdImgs;
    Rect m_rcScreenArea;
};

typedef OrderedRenderEngine ORE;

#endif // ORDERED_RENDER_ENGINE_H
