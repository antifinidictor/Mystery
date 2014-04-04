#include "DraggableItem.h"
#include "d3re/d3re.h"
#include "mge/ModularEngine.h"
using namespace std;

#define VALID_DROP_RADIUS (32)

vector<Point> DraggableItem::s_vDropPoints;
void
DraggableItem::addValidDropLocation(const Point &pt) {
    s_vDropPoints.push_back(pt);
}

DraggableItem::DraggableItem(uint uiObjId, uint uiItemId, uint uiIndex, const Rect &rcArea, Listener *pDropListener)
    : Draggable(uiObjId, rcArea)
{
    setPriority(1); //Higher priority than the draggable HUD
    m_pRenderModel = new D3HudRenderModel(D3RE::get()->getImageId("items"), rcArea);
    m_pRenderModel->setFrameH(uiItemId);
    MGE::get()->addListener(this, ON_MOUSE_MOVE);
    MGE::get()->addListener(this, ON_BUTTON_INPUT);

    m_pDropListener = pDropListener;

    m_uiIndex = uiIndex;

    printf("Item %d has obj id %d\n", uiItemId, uiObjId);
}

DraggableItem::~DraggableItem()
{
    MGE::get()->removeListener(this->getId(), ON_MOUSE_MOVE);
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);

    delete m_pRenderModel;
}

void
DraggableItem::onFollow(const Point &ptShift) {
    Draggable::onFollow(ptShift);
    m_pRenderModel->moveBy(ptShift);
}


void
DraggableItem::onStartDragging() {
    m_ptSnapPosition = m_pPhysicsModel->getPosition();
}

void
DraggableItem::onEndDragging() {
printf(__FILE__" %d\n",__LINE__);
    //Check: If close to a viable position, then react to that viable position.  Otherwise, snap back.
    Point ptCurPos = m_pPhysicsModel->getPosition();

    //If the position is invalid
    int index = 0;
    for(vector<Point>::iterator pt = s_vDropPoints.begin(); pt != s_vDropPoints.end(); ++pt, ++index) {
        if(dist(*pt, ptCurPos) < VALID_DROP_RADIUS) {
printf(__FILE__" %d\n",__LINE__);
            //Prepare to ask the listener for permission to drop the item here
            ItemDropEvent event;
            event.itemId = m_pRenderModel->getFrameH();
            event.itemOldIndex = m_uiIndex;
            event.itemNewIndex = index;
printf(__FILE__" %d\n",__LINE__);

            //Ask the listener for permission
            int retCode = m_pDropListener->callBack(getId(), &event, ON_ITEM_DROPPED);
printf(__FILE__" %d\n",__LINE__);

            //Did we get permission?
            if(retCode == EVENT_ITEM_CAN_DROP) {
printf(__FILE__" %d\n",__LINE__);
                m_ptSnapPosition = *pt;
                m_uiIndex = index;
                break;
            } else if(retCode == EVENT_ITEM_CANNOT_DROP) {
printf(__FILE__" %d\n",__LINE__);
                //The item should not be checked against other positions,
                // but it still cannot be dropped here
                break;
            }   //Otherwise, keep searching
        }
    }
    printf("Dropped at (%f,%f) (snap was (%f,%f))\n", ptCurPos.x, ptCurPos.y, m_ptSnapPosition.x, m_ptSnapPosition.y);
    onFollow(m_ptSnapPosition - ptCurPos);
}

void
DraggableItem::onMouseIn() {
}

void
DraggableItem::onMouseOut() {
}
