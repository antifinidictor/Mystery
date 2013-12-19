/*
 * ElementalVolume.cpp
 * Specifies info for elemental volumes
 */
#include "ElementalVolume.h"
#include "ForceField.h"
#include "tpe/tpe.h"
#include "pwe/PartitionedWorldEngine.h"

using namespace std;

ElementalVolume::ElementalVolume(uint uiId) {
    m_uiId = uiId;
    m_uiFlags = 0;
    m_uiNextField = 0;
}

ElementalVolume::~ElementalVolume() {
    map<uint,ForceField*>::iterator iter;
    for(iter = m_mForceFields.begin(); iter != m_mForceFields.end(); ++iter) {
        delete iter->second;
    }
    m_mForceFields.clear();

    PWE::get()->freeId(getId());
}

void
ElementalVolume::callBack(uint uiEventHandlerId, void *data, uint uiEventId) {
    switch(uiEventId) {
    case TPE_ON_COLLISION: {
        handleCollision((HandleCollisionData*)data);
        break;
      }
    default:
        break;
    }
}

void
ElementalVolume::handleCollision(HandleCollisionData *data) {
    //We'll deal with multiple collision objects later
    Point ptPos = data->obj->getPhysicsModel()->getPosition();
    Point ptForce = Point();
    map<uint,ForceField*>::iterator iter;
    for(iter = m_mForceFields.begin(); iter != m_mForceFields.end(); ++iter) {
        ptForce += iter->second->getForceAt(ptPos);
        ptForce.y = 0.f;
    }
    data->obj->getPhysicsModel()->applyForce(ptForce);
}


uint
ElementalVolume::addForceField(ForceField *field) {
    uint id = m_uiNextField++;
    m_mForceFields[id] = field;
    return id;
}

void
ElementalVolume::removeForceField(uint id) {
    map<uint,ForceField*>::iterator iter = m_mForceFields.find(id);
    if(iter != m_mForceFields.end()) {
        delete iter->second;
        m_mForceFields.erase(iter);
    }
}
