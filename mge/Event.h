/*
 * Event.h
 * Defines the Listener and EventHandler abstract classes; used for game-event
 * handling.
 */
#ifndef EVENT_H
#define EVENT_H

#include "defs.h"

//Standard callback return codes
enum CallbackReturnStatus {
    EVENT_CAUGHT,
    EVENT_DROPPED
};

class Listener {
private:
	uint m_uiEventTypes;

public:
	//Constructor(s)/Destructor
	Listener() { m_uiEventTypes = 0; }
	virtual ~Listener() {}

	//Abstract methods
	virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId) = 0;
	virtual uint getId() = 0;

	//Accessor methods
	bool getEventType(uint id) { return m_uiEventTypes & BIT(id); }
	void addEventType(uint id) { m_uiEventTypes = (m_uiEventTypes | BIT(id)); }
	void resetEventTypes() { m_uiEventTypes = 0; }
};

class EventHandler {
public:
    virtual ~EventHandler() {}
	virtual void addListener(Listener *pListener, uint id, char* triggerData = 0) = 0;
	virtual bool removeListener(uint uiListenerID, uint eventID) = 0;	//Returns true if object found
	virtual uint getId() = 0;
};


#endif
