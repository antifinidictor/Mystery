#ifndef FLOW_SPELL_H
#define FLOW_SPELL_H

#include "mge/defs.h"
#include "Spell.h"
#include <vector>

class ElementalVolume;

enum FSpellState {
    FS_STATE_AWAITING_POINTS,
    FS_STATE_READY,
    FS_STATE_ACTIVATED,
    FS_STATE_INVALID,
    FS_STATE_RESTORING,
    NUM_FS_STATES
};

class FlowSpell : public Spell {
public:
    FlowSpell(int duration, float magnitude = 0.8f);
    virtual ~FlowSpell();

    virtual void addPoint(ElementalVolume *ev, const Point &pt, bool bLeftClick);
    virtual bool activate();    //Returns true if successful, false if not
    virtual SpellState getStatus();       //Returns true if ready for activation
    virtual void update();      //Returns true if the spell has finished and can be deleted
    virtual void deactivate();

private:
    struct PointIdPair{
        uint id;
        Point pt;
        bool m_bPositiveNormal;
        PointIdPair(const Point &pt, bool bPositiveNormal) {
            this->pt = pt;
            this->id = 0;
            m_bPositiveNormal = bPositiveNormal;
        }
    };

    void clean();

    ElementalVolume *m_ev;
    int m_iTimer, m_iFxTimer;
    float m_fMagnitude;
    std::vector<PointIdPair> m_vForcePoints;
    bool m_bWasActivated;

    FSpellState m_eState;
};

#endif // FLOW_SPELL_H
