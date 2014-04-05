#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include "mge/Event.h"
#include "mge/Positionable.h"

enum DraggableState {
    DRAG_MOUSE_OUT,
    DRAG_MOUSE_IN,
    DRAG_DRAGGING
};

class Draggable : public Listener
{
public:
    Draggable(Positionable *parent, const Rect &rcRelativeClickArea);
    virtual ~Draggable();


    //Listener
    //virtual uint getId() = 0;
	virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId);

protected:
	//For use by the subclasses
    virtual void onFollow(const Point &ptShift);
    virtual void onStartDragging() {};
    virtual void onEndDragging() {};
    virtual void onMouseIn() {};
    virtual void onMouseOut() {};

    Positionable *m_pParent;

private:
    int onMouseMove(InputData *data);
    int onButtonPress(InputData *data);

    Point m_ptMouseOffset;
    DraggableState m_eState;

    Rect m_rcClickArea;
};

#endif // DRAGGABLE_H
