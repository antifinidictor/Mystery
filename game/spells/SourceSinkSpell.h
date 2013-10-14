#ifndef SPELLSOURCESINK_H
#define SPELLSOURCESINK_H

#include "mge/defs.h"
#include "Spell.h"

class ElementalVolume;

enum SSSpellState {
    SSS_STATE_FIRST_POINT,
    SSS_STATE_SECOND_POINT,
    SSS_STATE_READY,
    SSS_STATE_ACTIVATED,
    SSS_STATE_INVALID,
    NUM_SSS_STATES
};

class SourceSinkSpell : public Spell {
public:
    SourceSinkSpell(int duration, float magnitude = 100.f);
    virtual ~SourceSinkSpell();

    virtual void addPoint(ElementalVolume *ev, const Point &pt);
    virtual bool activate();    //Returns true if successful, false if not
    virtual SpellState getStatus();       //Returns true if getStatus for activation
    virtual void update();      //Returns true if the spell has finished and can be deleted

private:
    ElementalVolume *m_ev;
    int m_iTimer;
    Point m_ptSource;
    Point m_ptSink;
    uint m_uiSourceId;
    uint m_uiSinkId;
    float m_fMagnitude;
    bool m_bWasActivated;

    SSSpellState m_eState;
};

#endif // SPELLSOURCESINK_H
