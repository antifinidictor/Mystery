#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include "game/game_defs.h"
#include "mge/GameObject.h"
#include "tpe/TimePhysicsModel.h"
#include "d3re/D3HudRenderModel.h"
#include "d3re/ContainerRenderModel.h"

enum DraggableState {
    DRAG_MOUSE_OUT,
    DRAG_MOUSE_IN,
    DRAG_DRAGGING
};

class Draggable : public GameObject
{
public:
    Draggable(uint uiId, const Rect &rcArea);
    virtual ~Draggable();

    //General
    virtual bool update(uint time);

    //General
    virtual uint getId()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return TYPE_GUI; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "Draggable"; }

    //Models
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Listener
	virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId);
    virtual void onFollow(const Point &diff);
    virtual void onStartDragging() {};
    virtual void onEndDragging() {};
    virtual void onMouseIn() {};
    virtual void onMouseOut() {};

protected:
    NullTimePhysicsModel *m_pPhysicsModel;
    Rect getClickArea();
private:
    int onMouseMove(InputData *data);
    int onButtonPress(InputData *data);

    uint m_uiId;
    uint m_uiFlags;

    Point m_ptMouseOffset;
    DraggableState m_eState;
    Rect m_rcClickArea;
    //float m_fRadius;

};

#endif // DRAGGABLE_H
