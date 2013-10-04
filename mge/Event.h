/*
 * Event.h
 * Defines the Listener and EventHandler abstract classes; used for game-event
 * handling.
 */
#ifndef EVENT_H
#define EVENT_H

#include "defs.h"

class Listener {
private:
	uint m_uiEventTypes;

public:
	//Constructor(s)/Destructor
	Listener() { m_uiEventTypes = 0; }
	virtual ~Listener() {}

	//Abstract methods
	virtual void callBack(uint uiID, void *data, uint id) = 0;
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
