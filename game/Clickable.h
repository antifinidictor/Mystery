/*
 * Clickable.h
 */

#ifndef CLICKABLE_H
#define CLICKABLE_H

#include "mge/defs.h"
#include "mge/GameObject.h"
#include "mge/Event.h"
#include "ore/OrderedRenderModel.h"
#include "tpe/TimePhysicsModel.h"
#include "game/GameDefs.h"

enum ClickableState {
    CLICK_INACTIVE,
    CLICK_SELECTED,
    CLICK_PRESSED
};

class Clickable : public GameObject, public EventHandler, public Listener {
public:
    //Constructor(s)/Destructor
    Clickable(uint uiID, bool bFreeListener = false);
    virtual ~Clickable();

    //General
    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType() { return OBJ_GUI; }

//    virtual bool update(uint time) = 0;

    //Models
//    virtual RenderModel  *getRenderModel() = 0;
//    virtual PhysicsModel *getPhysicsModel() = 0;

    //Listener
	virtual void callBack(uint cID, void *data, EventID id);

    //Event handler
    virtual void addListener(void (*fpCallBack)(uint,uint));
	virtual void addListener(Listener *pListener, EventID id, char* triggerData = 0);
	virtual bool removeListener(uint uiListenerID, EventID eventID);

protected:
    ClickableState m_eState;

    //Abstract helper methods
    virtual Rect getResponseArea() = 0;
private:
    uint m_uiID;
    uint m_uiFlags;
    bool m_bFreeListener;

    Listener *m_pListener;    //Only going to allow one for now
    void (*m_fpCallBack)(uint,uint);

    //private helper methods
    void handleInput(InputData* data);
    void handleMouseMove(InputData* data);
    void informListeners(EventID id);
};

#endif
