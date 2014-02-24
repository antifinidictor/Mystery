#include "SourceSinkSpell.h"
#include "ElementalVolume.h"
#include "game/spells/ForceField.h"
#include "game/FxSprite.h"
#include "pwe/PartitionedWorldEngine.h"

#define FX_TIMER_MAX 60
SourceSinkSpell::SourceSinkSpell(int duration, float magnitude)
{
    m_ev1 = m_ev2 = NULL;
    m_iTimer = duration;
    m_iFxTimer = 0;
    m_fMagnitude = magnitude;
    m_eState = SSS_STATE_FIRST_POINT;
    m_bWasActivated = false;
}

SourceSinkSpell::~SourceSinkSpell()
{
    if(m_bWasActivated) {
        m_ev1->removeForceField(m_uiSourceId);
        m_ev2->removeForceField(m_uiSinkId);
    }
}

void
SourceSinkSpell::addPoint(ElementalVolume *ev, const Point &pt) {
    switch(m_eState) {
    case SSS_STATE_FIRST_POINT:
        m_ev1 = ev;
        m_eState = SSS_STATE_SECOND_POINT;
        m_ptSource = pt;
        break;
    case SSS_STATE_SECOND_POINT:
        m_ev2 = ev;
        m_eState = SSS_STATE_READY;
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
        if(m_ev1 == NULL) {
            m_eState = SSS_STATE_INVALID;
            return false;
        }
        m_eState = SSS_STATE_ACTIVATED;
        m_bWasActivated = true;
        m_uiSourceId = m_ev1->addForceField(new SourceForceField(m_ptSource, m_fMagnitude));
        m_uiSinkId = m_ev2->addForceField(new SinkForceField(m_ptSink, m_fMagnitude));
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
    Color crSource = Color(255, 0, 0);
    Color crSink = Color(255, 0, 0);
    switch(m_eState) {
    case SSS_STATE_ACTIVATED:
        if(--m_iTimer < 0) {
            m_eState = SSS_STATE_INVALID;
        }
        m_ev1->addVolumeAt(0.01f, m_ptSource);
        m_ev2->addVolumeAt(-0.01f, m_ptSink);
        //Do not break
    case SSS_STATE_READY:
        crSource = Color(0, 255, 0);
        crSink = Color(0, 255, 0);

        if(m_iFxTimer < 0) {
            FxSprite *sprite = new FxSprite(
                PWE::get()->genId(),
                D3RE::get()->getImageId("spellPoints"),
                64,
                m_ptSink + Point(0.f, 0.01f, 0.f),
                1
            );
            sprite->setColor(crSink);
            PWE::get()->add(sprite);
        }
        //Do not break
    case SSS_STATE_SECOND_POINT:
        if(m_iFxTimer < 0) {
            m_iFxTimer = FX_TIMER_MAX;
            FxSprite *sprite = new FxSprite(
                PWE::get()->genId(),
                D3RE::get()->getImageId("spellPoints"),
                64,
                m_ptSource + Point(0.f, 0.01f, 0.f),
                0
            );
            sprite->setColor(crSource);
            PWE::get()->add(sprite);
        }
        m_iFxTimer--;
        break;
    default:
        break;
    }
}
