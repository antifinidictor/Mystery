
#ifndef SPELL_H
#define SPELL_H


//#include "SourceSinkSpell.h"
//#include "FlowSpell.h"
//#include "DivideSpell.h"

enum SpellType {
    SPELL_TYPE_SOURCE_SINK,
    SPELL_TYPE_FLOW,
    SPELL_TYPE_DIVIDE,
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
    virtual void addPoint(ElementalVolume *ev, const Point &pt) = 0;
    virtual bool activate() = 0;    //Returns true if successful, false if not
    virtual SpellState getStatus() = 0;       //Returns true if ready for activation
    virtual void update() = 0;      //Returns true if the spell has finished and can be deleted
};

#endif // SPELL_H
