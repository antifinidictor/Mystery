#include "DraggableElementalSpellItem.h"
#include "d3re/d3re.h"
#include "mge/ModularEngine.h"
#include "game/items/Inventory.h"
using namespace std;

#define VALID_DROP_RADIUS (128)

Point DraggableElementalSpellItem::s_ptDropPoint;
Point DraggableElementalSpellItem::s_ptMakeCurrentPoint;

DraggableElementalSpellItem::DraggableElementalSpellItem(uint uiObjId, uint uiItemId, uint uiIndex, const Rect &rcArea, Listener *pDropListener)
    : Draggable(uiObjId, rcArea)
{
    setPriority(1); //Higher priority than the draggable HUD
    m_pRenderModel = new D3HudRenderModel(D3RE::get()->getImageId("items"), rcArea);
    m_pRenderModel->setFrameH(uiItemId);
    MGE::get()->addListener(this, ON_MOUSE_MOVE);
    MGE::get()->addListener(this, ON_BUTTON_INPUT);

    m_pDropListener = pDropListener;

    m_uiIndex = uiIndex;

    printf("Element/Spell %d has obj id %d\n", uiItemId, uiObjId);
}

DraggableElementalSpellItem::~DraggableElementalSpellItem()
{
    MGE::get()->removeListener(this->getId(), ON_MOUSE_MOVE);
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);

    delete m_pRenderModel;
}

void
DraggableElementalSpellItem::onFollow(const Point &ptShift) {
    Draggable::onFollow(ptShift);
    m_pRenderModel->moveBy(ptShift);
}


void
DraggableElementalSpellItem::onStartDragging() {
    m_ptSnapPosition = m_pPhysicsModel->getPosition();
}

void
DraggableElementalSpellItem::onEndDragging() {
    //Check: If close to a viable position, then react to that viable position.  Otherwise, snap back.
    Point ptCurPos = m_pPhysicsModel->getPosition();

    //Send the appropriate drop event
    if(dist(s_ptMakeCurrentPoint, ptCurPos) < VALID_DROP_RADIUS) {
        sendItemDropEvent(CUR_SPELL_ITEM_INDEX);
    } else if(dist(s_ptDropPoint, ptCurPos) < VALID_DROP_RADIUS) {
        sendItemDropEvent(DROP_SPELL_ITEM_INDEX);
    }

    //Move the item back to the correct position
    printf("Dropped at (%f,%f) (snap was (%f,%f))\n", ptCurPos.x, ptCurPos.y, m_ptSnapPosition.x, m_ptSnapPosition.y);
    onFollow(m_ptSnapPosition - ptCurPos);
}

void
DraggableElementalSpellItem::onMouseIn() {
}

void
DraggableElementalSpellItem::onMouseOut() {
}

void
DraggableElementalSpellItem::sendItemDropEvent(int newIndex) {
        //Tell the listener was in the region
        ItemDropEvent event;
        event.itemId = m_pRenderModel->getFrameH();
        event.itemOldIndex = m_uiIndex;
        event.itemNewIndex = newIndex;

        //Tell the listener about the event.  Its reply does not matter
        m_pDropListener->callBack(getId(), &event, ON_ITEM_DROPPED);
}
