#ifndef VORTEX_SPELL_H
#define VORTEX_SPELL_H

#include "mge/defs.h"
#include "Spell.h"
#include <vector>

class ElementalVolume;

enum VortexSpellState {
    VS_STATE_AWAITING_POINTS,
    VS_STATE_READY,
    VS_STATE_ACTIVATED,
    VS_STATE_INVALID,
    VS_STATE_RESTORING,
    NUM_VS_STATES
};

class VortexSpell : public Spell {
public:
    VortexSpell(int duration, float magnitude = 0.8f);
    virtual ~VortexSpell();

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

    VortexSpellState m_eState;
};

#endif // VORTEX_SPELL_H
