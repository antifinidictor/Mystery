/*
 * Render model for sprite objects
 */
#ifndef D3_HEIGHTMAP_RENDER_MODEL_H
#define D3_HEIGHTMAP_RENDER_MODEL_H

#include "mge/RenderModel.h"
#include "mge/defs.h"
#include "mge/Image.h"
#include "mge/PixelMap.h"
#include "mge/PhysicsModel.h"

class D3HeightmapRenderModel : public RenderModel {
public:
    D3HeightmapRenderModel(PhysicsModel *parent, uint uiTexture, const PixelMap *pxMap, Box bxVolume);
    virtual ~D3HeightmapRenderModel();

    virtual void render(RenderEngine *re);
    /*
    virtual void moveBy(const Point &ptShift) {}
    virtual Point getPosition();
    virtual Rect getDrawArea();
    */

    uint getTexture() { return m_uiTexture; }
    void setColor(const Color &cr) { m_crColor = cr; }
    Color &getColor() { return m_crColor; }

private:
    uint m_uiTexture;
    const PixelMap *m_pxMap;

    Color m_crColor;

    PhysicsModel *m_pParent;
};

#endif //D3_HEIGHTMAP_RENDER_MODEL_H
