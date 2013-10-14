#include "GameManager.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/TimePhysicsEngine.h"
#include "game/spells/ElementalVolume.h"

using namespace std;

GameManager *GameManager::m_pInstance;

GameManager::GameManager(uint uiId) {
    m_uiId = uiId;
    m_uiFlags = 0;
    m_skState.push(GM_START);
    m_fFadeTimer = 0.f;
    m_uiNextArea = 0;
    m_crWorld = Color(0xFF,0xFF,0xFF);
    m_crBackground = Color(0x9a,0xd7,0xfb);

    D3RE::get()->setWorldColor(m_crWorld);
    D3RE::get()->setBackgroundColor(m_crBackground);
    D3RE::get()->setColorWeight(DEFAULT_WEIGHT);
}

GameManager::~GameManager() {
    PWE::get()->freeId(getId());
}

GameObject*
GameManager::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint id = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));

    //Put state information here
    return new GameManager(id);
}

void
GameManager::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    pt.put(keyBase + ".id", getId());
    //Read state information here
}

bool
GameManager::update(uint time) {
    switch(m_skState.top()) {
    case GM_START:
        //Any initialization here
        m_skState.push(GM_NORMAL);
        break;
    case GM_FADE_OUT:
        if(m_fFadeTimer < 1.f) {
            fadeArea();
            m_fFadeTimer += FADE_TIME_STEP;
        } else {
            m_fFadeTimer = 1.f;
            m_skState.pop();
            m_skState.push(GM_FADE_IN);
            PWE::get()->setCurrentArea(m_uiNextArea);
        }
        break;
    case GM_FADE_IN:
        if(m_fFadeTimer > 0.f) {
            fadeArea();
            m_fFadeTimer -= FADE_TIME_STEP;
        } else {
            m_fFadeTimer = 0.f;
            m_skState.pop();
        }
        break;
    default:
        break;
    }
    return false;
}

void
GameManager::fadeArea() {
    Color black = Color(0x0,0x0,0x0);
    Color world = Color(
        m_crWorld.r * (1 - m_fFadeTimer) + black.r * m_fFadeTimer,
        m_crWorld.g * (1 - m_fFadeTimer) + black.r * m_fFadeTimer,
        m_crWorld.b * (1 - m_fFadeTimer) + black.r * m_fFadeTimer
    );
    Color background = Color(
        m_crBackground.r * (1 - m_fFadeTimer) + black.r * m_fFadeTimer,
        m_crBackground.g * (1 - m_fFadeTimer) + black.r * m_fFadeTimer,
        m_crBackground.b * (1 - m_fFadeTimer) + black.r * m_fFadeTimer
    );
    float fWeight = DEFAULT_WEIGHT * (1 - m_fFadeTimer) + FADE_WEIGHT * m_fFadeTimer;
    D3RE::get()->setWorldColor(world);
    D3RE::get()->setBackgroundColor(background);
    D3RE::get()->setColorWeight(fWeight);
}

void
GameManager::callBack(uint uiId, void *data, uint eventId) {
    switch(eventId) {
    case ON_AREA_FADE_IN: {
        m_uiNextArea = *((uint*)data);
        if(m_skState.top() == GM_NORMAL) {
            m_skState.push(GM_FADE_OUT);
        }
        break;
      }
    default:
        break;
    }
}

void
GameManager::addActiveVolume(ElementalVolume *ev) {
    m_mActiveVolumes[ev->getId()] = ev;
}

void
GameManager::removeActiveVolume(uint id) {
    m_mActiveVolumes.erase(id);
}

ElementalVolume*
GameManager::getTopVolume() {
    if(m_mActiveVolumes.size() < 1) {
        return NULL;
    }

    map<uint, ElementalVolume*>::iterator iter;
    ElementalVolume *ev = m_mActiveVolumes.begin()->second;
    Box bxVol = ev->getPhysicsModel()->getCollisionVolume();
    float maxY = bxVol.y + bxVol.h;
    for(iter = m_mActiveVolumes.begin(); iter != m_mActiveVolumes.end(); ++iter) {
        bxVol = iter->second->getPhysicsModel()->getCollisionVolume();
        if(bxVol.y + bxVol.h > maxY) {
            maxY = bxVol.y + bxVol.h;
            ev = iter->second;
        }
    }
    return ev;
}

