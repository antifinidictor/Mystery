#include "SourceSinkSpell.h"
#include "ElementalVolume.h"
#include "game/spells/ForceField.h"
#include "game/FxSprite.h"
#include "pwe/PartitionedWorldEngine.h"
#include "bae/BasicAudioEngine.h"

#define FX_TIMER_MAX 16
SourceSinkSpell::SourceSinkSpell(int duration, float magnitude)
{
    m_ev1 = m_ev2 = NULL;
    m_iTimer = duration;
    m_iFxTimer = 0;
    m_fMagnitude = magnitude;
    m_eState = SSS_STATE_FIRST_POINT;
    m_bWasActivated = false;
}

SourceSinkSpell::~SourceSinkSpell() {
    if(m_bWasActivated) {
        //TODO: May cause cleanup issues at the end of the game
        m_ev1->beginRestore();
        m_ev1->interpRestore(1.f);
        m_ev1->endRestore();

        if(m_ev2->getId() != m_ev1->getId()) {
            m_ev2->beginRestore();
            m_ev2->interpRestore(1.f);
            m_ev2->endRestore();
        }
    }
    clean();
}

void
SourceSinkSpell::clean() {
    if(m_bWasActivated) {
        m_ev1->removeForceField(m_uiSourceId);
        m_ev2->removeForceField(m_uiSinkId);
        m_bWasActivated = false;
    }
}

void
SourceSinkSpell::addPoint(ElementalVolume *ev, const Point &pt, bool bLeftClick) {
    switch(m_eState) {
    case SSS_STATE_FIRST_POINT:
        m_ev1 = ev;
        m_eState = SSS_STATE_SECOND_POINT;
        m_ptSink = pt;
        break;
    case SSS_STATE_SECOND_POINT:
        m_ev2 = ev;
        m_eState = SSS_STATE_READY;
        m_ptSource = pt;
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
    if(m_eState != SSS_STATE_INVALID) {
        BAE::get()->playSound(AUD_SPELL_POINT);
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
        //m_uiSourceId = m_ev2->addForceField(new SourceForceField(m_ptSource, m_fMagnitude));
        //m_uiSinkId = m_ev1->addForceField(new SinkForceField(m_ptSink, m_fMagnitude));
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
    case SSS_STATE_RESTORING:
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
    Color crPoint = Color(255, 0, 0);
    switch(m_eState) {
    case SSS_STATE_ACTIVATED: {
        if(--m_iTimer < 0) {
            m_eState = SSS_STATE_INVALID;   //SSS_STATE_RESTORING;
            /*
            m_iTimer = RESTORE_TIMER_MAX;
            m_ev1->beginRestore();

            if(m_ev2->getId() != m_ev1->getId()) {
                m_ev2->beginRestore();
            }
            */
            clean();
        }

        //Determine the amount of volume to transfer
        if(m_ev1->getVolume() > 1.f) {
            float volProp = 0.1f;
            m_ev2->addVolumeAt(volProp, m_ptSource);
            m_ev1->addVolumeAt(-volProp, m_ptSink);
        }
        //Do not break
    }
    case SSS_STATE_READY:
        crPoint = Color(0, 255, 0);

        if(m_iFxTimer < 0) {
            FxSprite *sprite = new FxSprite(
                PWE::get()->genId(),
                D3RE::get()->getImageId("spellPoints"),
                FX_TIMER_MAX,
                m_ptSource + Point(0.f, m_ev2->getHeightAt(m_ptSource) - m_ptSource.y + 0.05f, 0.f),
                0
            );
            sprite->setColor(crPoint);
            PWE::get()->add(sprite);
        }
        //Do not break
    case SSS_STATE_SECOND_POINT:
        if(m_iFxTimer < 0) {
            m_iFxTimer = FX_TIMER_MAX;
            FxSprite *sprite = new FxSprite(
                PWE::get()->genId(),
                D3RE::get()->getImageId("spellPoints"),
                FX_TIMER_MAX,
                m_ptSink + Point(0.f, m_ev1->getHeightAt(m_ptSink) - m_ptSink.y + 0.01f, 0.f),
                1
            );
            sprite->setColor(crPoint);
            PWE::get()->add(sprite);
        }
        m_iFxTimer--;
        break;
    case SSS_STATE_RESTORING:
        if(--m_iTimer < 0) {
            m_eState = SSS_STATE_INVALID;
            m_ev1->endRestore();
            if(m_ev2->getId() != m_ev1->getId()) {
                m_ev2->endRestore();
            }
        } else {
            m_ev1->interpRestore(1.f - m_iTimer * 1.f / RESTORE_TIMER_MAX);

            if(m_ev2->getId() != m_ev1->getId()) {
                m_ev2->interpRestore(1.f - m_iTimer * 1.f / RESTORE_TIMER_MAX);
            }
        }
        break;
    default:
        break;
    }
}

void
SourceSinkSpell::deactivate() {
    switch(m_eState) {
    case SSS_STATE_RESTORING:
        //Already restoring, do nothing
        break;
    case SSS_STATE_ACTIVATED:
        m_eState = SSS_STATE_INVALID;   //SSS_STATE_RESTORING;
        /*
        m_iTimer = RESTORE_TIMER_MAX;
        m_ev1->beginRestore();
        if(m_ev2->getId() != m_ev1->getId()) {
            m_ev2->beginRestore();
        }
        */
        clean();
        break;
    default:
        m_eState = SSS_STATE_INVALID;
        break;
    }
    clean();
}


