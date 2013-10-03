
#include "TimePhysicsModel.h"
#include "TimePhysicsEngine.h"

#define DEFAULT_TIME_DIVISOR 10.f
#define DEFAULT_FRICTION_COEFFICIENT 0.5f

TimePhysicsModel::TimePhysicsModel(Box bxVolume, float fMass) {
    m_bxVolume = bxVolume;
    m_pListener = NULL;
    m_ptAcceleration = Point();
    m_ptVelocity = Point();
    m_ptLastMotion = Point();
    m_fFrictionEffect = DEFAULT_FRICTION_COEFFICIENT;
    m_fFrictionAffect = DEFAULT_FRICTION_COEFFICIENT;
    m_fTimeDivisor = DEFAULT_TIME_DIVISOR;
    m_fMass = fMass;
    m_bWasPushed = false;
}

void TimePhysicsModel::moveBy(Point ptShift) {
    m_bxVolume += ptShift;
}

//F = ma
//a_new = F / m
void TimePhysicsModel::applyForce(Point ptAcceleration) {
    m_ptAcceleration += ptAcceleration;
}


void TimePhysicsModel::update(uint uiDeltaTime) {
    //The time divisor can be affected by other objects
    float dt = uiDeltaTime / m_fTimeDivisor;

    //Move the physics model
    m_ptLastMotion.x = 0.5 * m_ptAcceleration.x * dt * dt + m_ptVelocity.x * dt,
    m_ptLastMotion.y = 0.5 * m_ptAcceleration.y * dt * dt + m_ptVelocity.y * dt,
    m_ptLastMotion.z = 0.5 * m_ptAcceleration.z * dt * dt + m_ptVelocity.z * dt;
    m_ptVelocity.x = m_ptAcceleration.x * dt + m_ptVelocity.x * m_fFrictionAffect;
    m_ptVelocity.y = m_ptAcceleration.y * dt + m_ptVelocity.y * m_fFrictionAffect;
    m_ptVelocity.z = m_ptAcceleration.z * dt + m_ptVelocity.z * m_fFrictionAffect;
    moveBy(m_ptLastMotion);

    //Reset values changed each turn
    m_ptAcceleration = Point();
}

void TimePhysicsModel::handleCollisionEvent(HandleCollisionData *dat) {
    if(m_pListener) {
        m_pListener->callBack(0, dat, TPE_ON_COLLISION);
    }
}
