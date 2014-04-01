/*
 * ElementalVolume
 * Defines an object that basically stands in and listens for events.  It has an element type, a bx volume, and a float volume.
 */

#ifndef ELEMENTAL_VOLUME
#define ELEMENTAL_VOLUME

#include "mge/GameObject.h"
#include "game/game_defs.h"

class ForceField;
class HandleCollisionData;

class ElementalVolume : public GameObject {
public:
    ElementalVolume(uint uiId);
    virtual ~ElementalVolume();

    virtual uint getId() { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual bool update(uint time) = 0;
    virtual uint getType() { return TYPE_ELEMENTAL_VOLUME; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "ElementalVolume"; }

    //Listener
	virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId);

    //Elemental-specific
    uint addForceField(ForceField *field);
    void removeForceField(uint id);
    virtual void setVolume(float fVolume) = 0;
    virtual void addVolumeAt(float fVolume, const Point &pt) = 0;
    virtual float getVolume() = 0;
    virtual void interpRestore(float fTime) = 0;    //Use to restore to its original state, from 0.f -> 1.f
    virtual void beginRestore() = 0;
    virtual void endRestore() = 0;
protected:
    Point getTotalForceAt(const Point &pos);
private:

    void handleCollision(HandleCollisionData *data);

    uint m_uiId, m_uiFlags;
    uint m_uiNextField;
    std::map<uint,ForceField*> m_mForceFields;
};

#endif //ELEMENTAL_VOLUME
