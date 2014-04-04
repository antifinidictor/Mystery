#include "Draggable.h"
#include "pwe/PartitionedWorldEngine.h"

Draggable::Draggable(uint uiId, const Rect &rcArea)
{
    m_uiFlags = 0;
    m_uiId = uiId;
    m_rcClickArea = rcArea;
    Rect rcBackdropArea = Rect(0.f, 0.f, rcArea.w, rcArea.h);

    m_pPhysicsModel = new NullTimePhysicsModel(
        Point(rcArea.x + rcArea.w / 2, rcArea.y + rcArea.h / 2, 0)
    );
    //m_fRadius = rcArea.w / 2;
    m_eState = DRAG_MOUSE_OUT;
    m_ptMouseOffset = Point();
}

Draggable::~Draggable()
{
    PWE::get()->freeId(m_uiId);
}

int
Draggable::callBack(uint uiEventHandlerId, void *data, uint uiEventId) {
    int status = EVENT_DROPPED;
    switch(uiEventId) {
    case ON_MOUSE_MOVE:
        status = onMouseMove((InputData*)data);
        break;
    case ON_BUTTON_INPUT:
        status = onButtonPress((InputData*)data);
        break;
    default:
        break;
    }
    return status;
}

bool
Draggable::update(uint time) {
    return false;
}

void
Draggable::onFollow(const Point &diff) {
    m_pPhysicsModel->moveBy(diff);
    m_rcClickArea.x += diff.x;
    m_rcClickArea.y += diff.y;
}

int
Draggable::onMouseMove(InputData *data) {
    Point ptMouse = Point(
        data->getInputState(MIN_MOUSE_X),
        data->getInputState(MIN_MOUSE_Y),
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
        Point ptPos = m_pPhysicsModel->getPosition();
        onFollow(ptMouse + m_ptMouseOffset - ptPos);
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
            m_ptMouseOffset = m_pPhysicsModel->getPosition() - ptMouse;
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
