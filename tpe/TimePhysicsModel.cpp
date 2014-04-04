
#include "tpe.h"

#define DEFAULT_TIME_DIVISOR 10.f
#define DEFAULT_FRICTION_COEFFICIENT 0.5f

using namespace std;

TimePhysicsModel::TimePhysicsModel(Point ptPos, float fDensity) {
    m_bxVolume = Box();
    m_ptPos = ptPos;
    m_pListener = NULL;
    m_ptAcceleration = Point();
    m_ptVelocity = Point();
    m_ptLastMotion = Point();
    m_fFrictionEffect = DEFAULT_FRICTION_COEFFICIENT;
    m_fFrictionAffect = DEFAULT_FRICTION_COEFFICIENT;
    m_fTimeDivisor = DEFAULT_TIME_DIVISOR;
    m_fDensity = fDensity;
    m_fMass = 0.f;
    m_bWasPushed = false;
    m_pObjImOn = NULL;
    m_bIsCleaning = false;
}

TimePhysicsModel::~TimePhysicsModel() {
    list<AbstractTimePhysicsModel*>::iterator itSurf;
    m_bIsCleaning = true;
    for(itSurf = m_lsObjsOnMe.begin(); itSurf != m_lsObjsOnMe.end(); ++itSurf) {
        (*itSurf)->setSurface(NULL);
    }
    m_lsObjsOnMe.clear();
    if(m_pObjImOn != NULL) {
        m_pObjImOn->removeSurfaceObj(this);
    }

    vector<CollisionModel*>::iterator itColl;
    for(itColl = m_vCollisions.begin(); itColl != m_vCollisions.end(); ++itColl) {
        if((*itColl) != NULL) {
            delete *itColl;
        }
    }
    m_vCollisions.clear();
}

void TimePhysicsModel::moveBy(Point ptShift) {
    m_ptPos += ptShift;
    m_ptLastMotion += ptShift;

    list<AbstractTimePhysicsModel*>::iterator iter;
    for(iter = m_lsObjsOnMe.begin(); iter != m_lsObjsOnMe.end(); ++iter) {
        (*iter)->moveBy(ptShift);    //TODO: Why doesn't this work?
    }
}

//F = ma
//a_new = F / m
void TimePhysicsModel::applyForce(Point ptForce) {
    m_ptAcceleration += ptForce / m_fMass;
}


void TimePhysicsModel::update(uint uiDeltaTime) {
    //The time divisor can be affected by other objects
    float dt = uiDeltaTime / m_fTimeDivisor;

    //Move the physics model
    m_ptLastMotion = Point();
    float fx = 0.5 * m_ptAcceleration.x * dt * dt + m_ptVelocity.x * dt;
    float fy = 0.5 * m_ptAcceleration.y * dt * dt + m_ptVelocity.y * dt;
    float fz = 0.5 * m_ptAcceleration.z * dt * dt + m_ptVelocity.z * dt;
    m_ptVelocity.x = m_ptAcceleration.x * dt + m_ptVelocity.x * m_fFrictionAffect;
    m_ptVelocity.y = m_ptAcceleration.y * dt + m_ptVelocity.y * m_fFrictionAffect;
    m_ptVelocity.z = m_ptAcceleration.z * dt + m_ptVelocity.z * m_fFrictionAffect;
    moveBy(Point(fx, fy, fz));

    //Reset values changed each turn
    m_ptAcceleration = Point();
}

void TimePhysicsModel::handleCollisionEvent(HandleCollisionData *dat) {
    if(m_pListener) {
        m_pListener->callBack(0, dat, TPE_ON_COLLISION);
    }
}

void
TimePhysicsModel::addSurfaceObj(AbstractTimePhysicsModel *mdl) {
    if(m_bIsCleaning) return;

    m_lsObjsOnMe.push_back(mdl);
}

void
TimePhysicsModel::removeSurfaceObj(AbstractTimePhysicsModel *mdl) {
    if(m_bIsCleaning) return;

    list<AbstractTimePhysicsModel*>::iterator iter;
    for(iter = m_lsObjsOnMe.begin(); iter != m_lsObjsOnMe.end(); ++iter) {
        if(*iter == mdl) {
            m_lsObjsOnMe.erase(iter);
            break;
        }
    }
}

void
TimePhysicsModel::setSurface(PhysicsModel *mdl) {
    AbstractTimePhysicsModel *amdl = (AbstractTimePhysicsModel*)mdl;
    if(m_pObjImOn != amdl) {
        if(m_pObjImOn != NULL) {
            m_pObjImOn->removeSurfaceObj(this);
        }
        m_pObjImOn = amdl;
        if(m_pObjImOn != NULL) {
            m_pObjImOn->addSurfaceObj(this);
        }
    }
}


uint
TimePhysicsModel::addCollisionModel(CollisionModel* mdl) {
    m_vCollisions.push_back(mdl);
    m_bxVolume += mdl->getBounds();
    m_fMass += m_fDensity * mdl->getVolume();
    return m_vCollisions.size() - 1;
}

void
TimePhysicsModel::removeCollisionModel(uint id) {
    if(id < m_vCollisions.size()) {
        delete m_vCollisions[id];
        m_vCollisions[id] = NULL;
    }
    resetVolume();
}
CollisionModel*
TimePhysicsModel::getCollisionModel(uint id) {
    if(id < m_vCollisions.size()) {
        return m_vCollisions[id];
    }
    return NULL;
}

void
TimePhysicsModel::resetVolume() {
    //Recalculate total volume
    m_bxVolume = Box();
    m_fMass = 0.f;
    vector<CollisionModel*>::iterator iter;
    for(iter = m_vCollisions.begin(); iter != m_vCollisions.end(); ++iter) {
        if((*iter) != NULL) {
            m_bxVolume += (*iter)->getBounds();
            m_fMass += m_fDensity * (*iter)->getVolume();
        }
    }
}
