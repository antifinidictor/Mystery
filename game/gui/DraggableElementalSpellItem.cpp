#include "DraggableElementalSpellItem.h"
#include "d3re/d3re.h"
#include "mge/ModularEngine.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/game_defs.h"
#include "game/gui/DraggableHud.h"
using namespace std;

#define VALID_DROP_RADIUS (128)

Point DraggableElementalSpellItem::s_ptDropPoint;
Point DraggableElementalSpellItem::s_ptMakeCurrentPoint;

DraggableElementalSpellItem::DraggableElementalSpellItem(Item *pItem, const Rect &rcArea, Listener *pDropListener)
    : Draggable(this, Rect(-rcArea.w / 2.f, -rcArea.h / 2.f, rcArea.w, rcArea.h)),
      D3HudRenderModel(D3RE::get()->getImageId("items"), rcArea),
        m_pItem(pItem),
        m_uiId(pItem->getId()),
        m_ptSnapPosition(),
        m_pDropListener(pDropListener),
        m_fTotalDragDistance(0.f)
{
    setPriority(1); //Higher priority than the draggable HUD
    setFrameH(pItem->getItemId());

    MGE::get()->addListener(this, ON_MOUSE_MOVE);
    MGE::get()->addListener(this, ON_BUTTON_INPUT);
}

DraggableElementalSpellItem::~DraggableElementalSpellItem()
{
    MGE::get()->removeListener(this->getId(), ON_MOUSE_MOVE);
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);

    if(m_pItem) {
        delete m_pItem;
    }
    //The model should be deleted by its container
    //delete m_pRenderModel;
}

void
DraggableElementalSpellItem::onFollow(const Point &diff) {
    Draggable::onFollow(diff);
    m_fTotalDragDistance += diff.magnitude();
}

void
DraggableElementalSpellItem::onStartDragging() {
    m_ptSnapPosition = m_pParent->getPosition();
    m_fTotalDragDistance = 0.f;
}

void
DraggableElementalSpellItem::onEndDragging() {
    //Check: If close to a viable position, then react to that viable position.  Otherwise, snap back.
    Point ptCurPos = m_pParent->getPosition();

    //Send the appropriate drop event
    if(dist(s_ptMakeCurrentPoint, ptCurPos) < VALID_DROP_RADIUS) {
        sendItemDropEvent(CUR_SPELL_ITEM_INDEX);
    } else if(dist(s_ptDropPoint, ptCurPos) < VALID_DROP_RADIUS) {
        sendItemDropEvent(DROP_SPELL_ITEM_INDEX);
    } else if(m_fTotalDragDistance < 5.f) {
        sendItemDropEvent(CUR_SPELL_ITEM_INDEX);
    }

    //Move the item back to the correct position
//    printf("Dropped at (%f,%f) (snap was (%f,%f))\n", ptCurPos.x, ptCurPos.y, m_ptSnapPosition.x, m_ptSnapPosition.y);
    onFollow(m_ptSnapPosition - ptCurPos);
}

void
DraggableElementalSpellItem::sendItemDropEvent(int newIndex) {
        //Tell the listener was in the region
        ItemDropEvent event;
        event.item = m_pItem;
        event.itemOldIndex = m_pItem->getItemId();
        event.itemNewIndex = newIndex;
        event.distance = m_fTotalDragDistance;

        //Tell the listener about the event.  Its reply does not matter
        m_pDropListener->callBack(getId(), &event, ON_ITEM_DROPPED);
}

void
DraggableElementalSpellItem::onMouseIn() {
    ContainerRenderModel *panel =
        D3RE::get()->getHudContainer()->
        get<ContainerRenderModel*>(HUD_TOPBAR)->
        get<ContainerRenderModel*>(MGHUD_SIDEBUTTON_CONTAINER);
    panel->get<D3HudRenderModel*>(MGHUD_SIDEBUTTON_ITEMNAME)->updateText(m_pItem->getItemName());
    panel->get<D3HudRenderModel*>(MGHUD_SIDEBUTTON_ITEMDESC)->updateText(m_pItem->getItemInfo());
}

void
DraggableElementalSpellItem::onMouseOut() {
    ContainerRenderModel *panel =
        D3RE::get()->getHudContainer()->
        get<ContainerRenderModel*>(HUD_TOPBAR)->
        get<ContainerRenderModel*>(MGHUD_SIDEBUTTON_CONTAINER);
    panel->get<D3HudRenderModel*>(MGHUD_SIDEBUTTON_ITEMNAME)->updateText("");
    panel->get<D3HudRenderModel*>(MGHUD_SIDEBUTTON_ITEMDESC)->updateText("");
}
