/*
 * ElementalVolume
 * Defines an object that basically stands in and listens for events.  It has an element type, a bx volume, and a float volume.
 */

#ifndef ELEMENTAL_VOLUME
#define ELEMENTAL_VOLUME

#include "mge/GameObject.h"
#include "game/game_defs.h"

class ForceField;

class ElementalVolume : public GameObject {
public:
    ElementalVolume();
    virtual ~ElementalVolume();

    virtual uint getId() { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual bool update(uint time);
    virtual uint getType() { return TYPE_ELEMENTAL_VOLUME; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "ElementalVolume"; }

    //Listener
	virtual void callBack(uint uiEventHandlerId, void *data, uint uiEventId) = 0;

    //Elemental-specific
    uint addForceField(ForceField *field);
    void removeForceField(uint id);

private:

    uint m_uiId, m_uiFlags;
    std::map<uint,ForceField*> m_mForceFields;
};

#endif //ELEMENTAL_VOLUME
