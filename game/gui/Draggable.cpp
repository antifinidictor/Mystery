#include "Draggable.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/game_defs.h"

Draggable::Draggable(Positionable *parent, const Rect &rcRelativeClickArea)
{
    m_rcClickArea = rcRelativeClickArea;

    printf("Click area (relative): (%f,%f,%f,%f)\n",
           m_rcClickArea.x, m_rcClickArea.y,
           m_rcClickArea.w, m_rcClickArea.h);

    m_eState = DRAG_MOUSE_OUT;
    m_ptMouseOffset = Point();
    m_pParent = parent;
}

Draggable::~Draggable()
{
}

int
Draggable::callBack(uint uiEventHandlerId, void *data, uint uiEventId) {
    int status = EVENT_DROPPED;
    switch(uiEventId) {
    case ON_MOUSE_MOVE:
        status = onMouseMove((InputData*)data);
        break;
    case ON_BUTTON_INPUT:
        onMouseMove((InputData*)data);
        status = onButtonPress((InputData*)data);
        break;
    default:
        break;
    }
    return status;
}

void
Draggable::onFollow(const Point &diff) {
    m_pParent->moveBy(diff);
}

int
Draggable::onMouseMove(InputData *data) {
    Point ptPos = m_pParent->getPosition();
    Point ptMouse = Point(
        data->getInputState(MIN_MOUSE_X) - (ptPos.x),
        data->getInputState(MIN_MOUSE_Y) - (ptPos.y),
        0.f
    );

    switch(m_eState) {
    case DRAG_MOUSE_OUT: {
        if(ptInRect(ptMouse, m_rcClickArea)) {
            m_eState = DRAG_MOUSE_IN;
            onMouseIn();
        }
        break;
    }
    case DRAG_MOUSE_IN: {
        if(!ptInRect(ptMouse, m_rcClickArea)) {
            m_eState = DRAG_MOUSE_OUT;
            onMouseOut();
        }
        break;
    }
    case DRAG_DRAGGING: {
        onFollow(ptMouse + m_ptMouseOffset);
        break;
    }
    }
    return EVENT_DROPPED;   //Mouse move events should be shared
}

int
Draggable::onButtonPress(InputData *data) {
    int status = EVENT_DROPPED;
    switch(m_eState) {
    case DRAG_MOUSE_IN: {
        if(data->getInputState(IN_SELECT) && data->hasChanged(IN_SELECT)) {
            m_eState = DRAG_DRAGGING;
            Point ptMouse = Point(
                data->getInputState(MIN_MOUSE_X),
                data->getInputState(MIN_MOUSE_Y),
                0.f
            );
            m_ptMouseOffset = m_pParent->getPosition() - ptMouse;
            printf("Clicked on pos (%f,%f)\n", ptMouse.x, ptMouse.y);
            onStartDragging();
            status = EVENT_CAUGHT;
        }
        break;
    }
    case DRAG_DRAGGING: {
        if(!data->getInputState(IN_SELECT) && data->hasChanged(IN_SELECT)) {
            m_eState = DRAG_MOUSE_IN;
            onEndDragging();
            status = EVENT_CAUGHT;
        }
        break;
    }
    default:
        break;
    }
    return status;
}
