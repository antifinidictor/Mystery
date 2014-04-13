#ifndef DRAGGABLE_ELEMENTAL_SPELL_ITEM_H
#define DRAGGABLE_ELEMENTAL_SPELL_ITEM_H

#include "game/gui/Draggable.h"
#include "d3re/D3HudRenderModel.h"
#include "game/items/Item.h"
#include <vector>

class DraggableElementalSpellItem : public Draggable, public D3HudRenderModel
{
public:
    static void setDropPoint(const Point &pt) { s_ptDropPoint = pt; }
    static void setMakeCurrentPoint(const Point &pt) { s_ptMakeCurrentPoint = pt; }

    DraggableElementalSpellItem(Item *pItem, const Rect &rcArea, Listener *pDropListener);
    virtual ~DraggableElementalSpellItem();

    //Listener
    virtual uint getId() { return m_pItem->getId(); }

    //Draggable functions
    virtual void onFollow(const Point &diff);
    virtual void onStartDragging();
    virtual void onEndDragging();
    //virtual void onMouseIn();
    //virtual void onMouseOut();

private:
    static Point s_ptDropPoint;         //Point where the element or spell can be put down
    static Point s_ptMakeCurrentPoint;  //Point where the element or spell can be made current

    void sendItemDropEvent(int newIndex);

    Item *m_pItem;
    Point m_ptSnapPosition;
    Listener *m_pDropListener;
    float m_fTotalDragDistance;
};

#endif // DRAGGABLE_ELEMENTAL_SPELL_ITEM_H
