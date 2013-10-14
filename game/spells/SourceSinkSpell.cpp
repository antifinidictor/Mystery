#include "SourceSinkSpell.h"
#include "ElementalVolume.h"
#include "game/spells/ForceField.h"

SourceSinkSpell::SourceSinkSpell(int duration, float magnitude)
{
    m_ev = NULL;
    m_iTimer = duration;
    m_fMagnitude = magnitude;
    m_eState = SSS_STATE_FIRST_POINT;
    m_bWasActivated = false;
}

SourceSinkSpell::~SourceSinkSpell()
{
    if(m_bWasActivated) {
        m_ev->removeForceField(m_uiSourceId);
        m_ev->removeForceField(m_uiSinkId);
    }
}

void
SourceSinkSpell::addPoint(ElementalVolume *ev, const Point &pt) {
    switch(m_eState) {
    case SSS_STATE_FIRST_POINT:
        m_ev = ev;
        m_eState = SSS_STATE_SECOND_POINT;
        m_ptSource = pt;
        break;
    case SSS_STATE_SECOND_POINT:
        if(m_ev != ev && m_ev != NULL) {
            m_eState = SSS_STATE_INVALID;
        } else {
            m_eState = SSS_STATE_READY;
        }
        m_ptSink = pt;
        break;
    case SSS_STATE_ACTIVATED:
    case SSS_STATE_READY:
        m_eState = SSS_STATE_INVALID; //Tried to add too many points!
        break;
    case SSS_STATE_INVALID:
        break;
    default:
        break;
    }
}

bool
SourceSinkSpell::activate() {
    if(m_eState == SSS_STATE_READY) {
        m_eState = SSS_STATE_ACTIVATED;
        m_bWasActivated = true;
        m_uiSourceId = m_ev->addForceField(new SourceForceField(m_ptSource, m_fMagnitude));
        m_uiSinkId = m_ev->addForceField(new SinkForceField(m_ptSink, m_fMagnitude));
        return true;
    }
    return false;
}

SpellState
SourceSinkSpell::getStatus() {
    switch(m_eState) {
    case SSS_STATE_FIRST_POINT:
    case SSS_STATE_SECOND_POINT:
        return SPELL_WAITING;
    case SSS_STATE_ACTIVATED:
        return SPELL_ACTIVE;
    case SSS_STATE_READY:
        return SPELL_READY;
    case SSS_STATE_INVALID:
    default:
        return SPELL_INVALID;
    }
}

void
SourceSinkSpell::update() {
    if(m_eState == SSS_STATE_ACTIVATED) {
        if(--m_iTimer < 0) {
            m_eState = SSS_STATE_INVALID;
        }
    }
}
