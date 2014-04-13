#include "DraggableItem.h"
#include "d3re/d3re.h"
#include "mge/ModularEngine.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/game_defs.h"
using namespace std;

#define VALID_DROP_RADIUS (32)

vector<Point> DraggableItem::s_vDropPoints;
void
DraggableItem::addValidDropLocation(const Point &pt) {
    s_vDropPoints.push_back(pt);
}

DraggableItem::DraggableItem(Item *item, uint uiIndex, const Rect &rcArea, Listener *pDropListener)
    : D3HudRenderModel(D3RE::get()->getImageId("items"), rcArea),
      Draggable(this, Rect(-rcArea.w / 2.f, -rcArea.h / 2.f, rcArea.w, rcArea.h))
{
printf(__FILE__" %d\n",__LINE__);

    setPriority(1); //Higher priority than the draggable HUD

    m_pItem = item;
    setFrameH(item->getItemId());

    MGE::get()->addListener(this, ON_MOUSE_MOVE);
    MGE::get()->addListener(this, ON_BUTTON_INPUT);

    m_pDropListener = pDropListener;

    m_uiIndex = uiIndex;
}

DraggableItem::~DraggableItem()
{
    MGE::get()->removeListener(this->getId(), ON_MOUSE_MOVE);
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);
    //The render model should be deleted by its container
    //delete m_pRenderModel;
}

void
DraggableItem::onStartDragging() {
    m_ptSnapPosition = m_pParent->getPosition();
    m_fTotalDragDistance = 0.f;
    //printf("Picked up at (%f,%f)\n", m_ptSnapPosition.x, m_ptSnapPosition.y);
}

void
DraggableItem::onEndDragging() {
    //Check: If close to a viable position, then react to that viable position.  Otherwise, snap back.
    Point ptCurPos = m_pParent->getPosition();

    //If the position is invalid
    int index = 0;
    for(vector<Point>::iterator pt = s_vDropPoints.begin(); pt != s_vDropPoints.end(); ++pt, ++index) {
        if(dist(*pt, ptCurPos) < VALID_DROP_RADIUS) {
            //Prepare to ask the listener for permission to drop the item here
            ItemDropEvent event;
            event.item = m_pItem;
            event.itemOldIndex = m_uiIndex;
            event.itemNewIndex = index;
            event.distance = m_fTotalDragDistance;

            //Ask the listener for permission
            int retCode = m_pDropListener->callBack(getId(), &event, ON_ITEM_DROPPED);

            //Did we get permission?
            if(retCode == EVENT_ITEM_CAN_DROP) {
                m_ptSnapPosition = *pt;
                m_uiIndex = index;
                break;
            } else if(retCode == EVENT_ITEM_CANNOT_DROP) {
                //The item should not be checked against other positions,
                // but it still cannot be dropped here
                break;
            }   //Otherwise, keep searching
        }
    }
    //printf("Dropped at (%f,%f) (snap was (%f,%f))\n", ptCurPos.x, ptCurPos.y, m_ptSnapPosition.x, m_ptSnapPosition.y);
    onFollow(m_ptSnapPosition - ptCurPos);
}


void
DraggableItem::onFollow(const Point &diff) {
    Draggable::onFollow(diff);
    m_fTotalDragDistance += diff.magnitude();
}


void
DraggableItem::onMouseIn() {
    setImageColor(Color(200, 255, 200));
}

void
DraggableItem::onMouseOut() {
    setImageColor(Color(255, 255, 255));
}

void
DraggableItem::snapToIndex(uint index) {
    Point ptShift = s_vDropPoints[index] - m_pParent->getPosition();
    onFollow(ptShift);
    m_uiIndex = index;
}
