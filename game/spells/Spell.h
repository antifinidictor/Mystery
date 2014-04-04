
#ifndef SPELL_H
#define SPELL_H


//#include "SourceSinkSpell.h"
//#include "FlowSpell.h"
//#include "DivideSpell.h"

#define RESTORE_TIMER_MAX 100

enum SpellType {
    SPELL_TYPE_CYCLIC,
    SPELL_TYPE_FLOW,
    //SPELL_TYPE_DIVIDE,
    NUM_SPELL_TYPES
};


class ElementalVolume;

enum SpellState {
    SPELL_WAITING,
    SPELL_INVALID,
    SPELL_READY,
    SPELL_ACTIVE
};

class Spell {
public:
    virtual ~Spell() {}
    virtual void addPoint(ElementalVolume *ev, const Point &pt, bool bLeftClick) = 0;
    virtual bool activate() = 0;    //Returns true if successful, false if not
    virtual SpellState getStatus() = 0;       //Returns true if ready for activation
    virtual void update() = 0;      //Returns true if the spell has finished and can be deleted
    virtual void deactivate() = 0;
};

#endif // SPELL_H
