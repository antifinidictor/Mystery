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

int
ElementalVolume::callBack(uint uiEventHandlerId, void *data, uint uiEventId) {
    int status = EVENT_CAUGHT;
    switch(uiEventId) {
    case TPE_ON_COLLISION: {
        handleCollision((HandleCollisionData*)data);
        break;
      }
    default:
        status = EVENT_DROPPED;
        break;
    }
    return status;
}

void
ElementalVolume::handleCollision(HandleCollisionData *data) {
    //We'll deal with multiple collision objects later
    if(!data->obj->getFlag(TPE_STATIC)) {
        Point ptPos = data->obj->getPhysicsModel()->getPosition();
        Point ptForce = getTotalForceAt(ptPos);
        data->obj->getPhysicsModel()->applyForce(ptForce);
    }
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

Point
ElementalVolume::getTotalForceAt(const Point &ptPos) {
    Point ptForce = Point();
    map<uint,ForceField*>::iterator iter;
    for(iter = m_mForceFields.begin(); iter != m_mForceFields.end(); ++iter) {
        ptForce += iter->second->getForceAt(ptPos);
        ptForce.y = 0.f;
    }
    return ptForce;
}
