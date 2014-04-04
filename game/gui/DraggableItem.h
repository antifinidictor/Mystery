#ifndef DRAGGABLEITEM_H
#define DRAGGABLEITEM_H

#include "game/gui/Draggable.h"
#include "d3re/D3HudRenderModel.h"
#include <vector>

class DraggableItem : public Draggable
{
public:
    static void addValidDropLocation(const Point &pt);

    DraggableItem(uint uiObjId, uint uiItemId, uint uiIndex, const Rect &rcArea, Listener *pDropListener);
    virtual ~DraggableItem();

    //models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }

    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase) {}

    //Draggable functions
    virtual void onFollow(const Point &diff);
    virtual void onStartDragging();
    virtual void onEndDragging();
    virtual void onMouseIn();
    virtual void onMouseOut();

private:
    static std::vector<Point> s_vDropPoints;

    D3HudRenderModel *m_pRenderModel;
    Point m_ptSnapPosition;
    Listener *m_pDropListener;
    uint m_uiIndex;
};

#endif // DRAGGABLEITEM_H
