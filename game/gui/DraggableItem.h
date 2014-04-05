#ifndef DRAGGABLEITEM_H
#define DRAGGABLEITEM_H

#include "game/gui/Draggable.h"
#include "d3re/D3HudRenderModel.h"
#include "game/items/Item.h"
#include <vector>

class DraggableItem : public Draggable, public D3HudRenderModel
{
public:
    static void addValidDropLocation(const Point &pt);

    DraggableItem(Item *item, uint uiIndex, const Rect &rcArea, Listener *pDropListener);
    virtual ~DraggableItem();

    //Listener
    virtual uint getId() { return m_pItem->getId(); }

    //Draggable functions
    //virtual void onFollow(const Point &diff);
    virtual void onStartDragging();
    virtual void onEndDragging();
    virtual void onMouseIn();
    virtual void onMouseOut();

    //General
    Item *getItem() { return m_pItem; }

    void snapToIndex(uint index);

private:
    static std::vector<Point> s_vDropPoints;

    Item *m_pItem;
    Point m_ptSnapPosition;
    Listener *m_pDropListener;
    uint m_uiIndex;
};

#endif // DRAGGABLEITEM_H
