#ifndef DRAGGABLE_ELEMENTAL_SPELL_ITEM_H
#define DRAGGABLE_ELEMENTAL_SPELL_ITEM_H

#include "game/gui/Draggable.h"
#include "d3re/D3HudRenderModel.h"
#include <vector>

class DraggableElementalSpellItem : public Draggable
{
public:
    static void setDropPoint(const Point &pt) { s_ptDropPoint = pt; }
    static void setMakeCurrentPoint(const Point &pt) { s_ptMakeCurrentPoint = pt; }

    DraggableElementalSpellItem(uint uiObjId, uint uiItemId, uint uiIndex, const Rect &rcArea, Listener *pDropListener);
    virtual ~DraggableElementalSpellItem();

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
    static Point s_ptDropPoint;         //Point where the element or spell can be put down
    static Point s_ptMakeCurrentPoint;  //Point where the element or spell can be made current
    
    void sendItemDropEvent(int newIndex);

    D3HudRenderModel *m_pRenderModel;
    Point m_ptSnapPosition;
    Listener *m_pDropListener;
    uint m_uiIndex;
};

#endif // DRAGGABLE_ELEMENTAL_SPELL_ITEM_H
