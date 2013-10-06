/*
 * TimePhysicsModel.h
 * Defines the basic physics model structure
 */

#ifndef TIME_PHYSICS_MODEL_H
#define TIME_PHYSICS_MODEL_H

#include "mge/PhysicsModel.h"
#include "mge/Event.h"
#include <list>

struct HandleCollisionData;

/*
 * AbstractTimePhysicsModel
 * Defines the basic time physics model interface
 */
class AbstractTimePhysicsModel : public PhysicsModel {
public:
    //new methods
    virtual ~AbstractTimePhysicsModel() {}
    virtual void mulTimeDivisor(float fTimeEffect) = 0;
    virtual float getMass() = 0;
    virtual void setListener(Listener *pListener) = 0;
    virtual void clearVerticalVelocity() = 0;
    virtual bool wasPushed() = 0;
    virtual void setWasPushed(bool pushed) = 0;
    virtual void update(uint time) = 0;
    virtual void handleCollisionEvent(HandleCollisionData *dat) = 0;
    virtual float getDensity() = 0;
    virtual float getVolume() = 0;

    virtual void addSurfaceObj(AbstractTimePhysicsModel *mdl) = 0;
    virtual void removeSurfaceObj(AbstractTimePhysicsModel *mdl) = 0;
    virtual void setSurface(AbstractTimePhysicsModel *mdl) = 0;
    virtual AbstractTimePhysicsModel* getSurface() = 0;
};

/*
 * TimePhysicsModel
 * The basic time physics model
 */
class TimePhysicsModel : public AbstractTimePhysicsModel {
public:
    TimePhysicsModel(Box bxVolume, float fDensity = 1000.f);   //Density in kg/m^3
    virtual ~TimePhysicsModel();

    virtual Point getPosition() { return bxCenter(m_bxVolume); }//Point(m_bxVolume); }
    virtual Point getCenter() { return bxCenter(m_bxVolume); }
    virtual Point getLastVelocity() { return m_ptLastMotion; }
    virtual Box   getCollisionVolume() { return m_bxVolume; }
    virtual void  moveBy(Point ptShift);
    virtual void  applyForce(Point ptForce);

    virtual void  handleCollisionEvent(HandleCollisionData *dat);
    virtual void  update(uint time);

    //Time physics model properties
    virtual void mulTimeDivisor(float fTimeEffect) { m_fTimeDivisor *= fTimeEffect; }
    virtual float getMass() { return m_fMass; }
    virtual void setListener(Listener *pListener) { m_pListener = pListener; }
    virtual void clearVerticalVelocity() { m_ptVelocity.z = 0.f; }
    virtual bool wasPushed() { return m_bWasPushed; }
    virtual void setWasPushed(bool pushed) { m_bWasPushed = pushed; }

    virtual float getDensity() { return m_fMass / m_fVolume; }
    virtual float getVolume()  { return m_fVolume; }

    virtual void addSurfaceObj(AbstractTimePhysicsModel *mdl);
    virtual void removeSurfaceObj(AbstractTimePhysicsModel *mdl);
    virtual void setSurface(AbstractTimePhysicsModel *mdl) { m_pObjImOn = mdl; }
    virtual AbstractTimePhysicsModel* getSurface() { return m_pObjImOn; }

private:
    //Time physics model
    Point m_ptAcceleration,
          m_ptVelocity;
    Point m_ptLastMotion;
    Box   m_bxVolume;
    float m_fFrictionEffect,
          m_fFrictionAffect;
    float m_fTimeDivisor;
    float m_fMass, m_fVolume;

    //Listener
    Listener *m_pListener;
    bool m_bWasPushed;

    std::list<AbstractTimePhysicsModel*> m_lsObjsOnMe;
    AbstractTimePhysicsModel *m_pObjImOn;
};

/*
 * NullTimePhysicsModel
 * Only tracks position.  No volume to speak of.
 */
class NullTimePhysicsModel : public AbstractTimePhysicsModel {
public:
    NullTimePhysicsModel(Box bxVolume) { m_ptPosition = bxCenter(bxVolume); }
    virtual ~NullTimePhysicsModel() {}

    virtual Point getPosition() { return m_ptPosition; }
    virtual Point getCenter() { return m_ptPosition; }
    virtual Point getLastVelocity() { return Point(); }
    virtual Box   getCollisionVolume() { return Box(m_ptPosition.x, m_ptPosition.y, m_ptPosition.z, 0, 0, 0); }
    virtual void  moveBy(Point ptShift) { m_ptPosition += ptShift; }
    virtual void  applyForce(Point ptForce) {}

    virtual void  handleCollisionEvent(HandleCollisionData *dat) {}
    virtual void  update(uint time) {}

    //Time physics model properties
    virtual void mulTimeDivisor(float fTimeEffect) { }
    virtual float getMass() { return 1.f; }
    virtual void setListener(Listener *pListener) { }
    virtual void clearVerticalVelocity() { }
    virtual bool wasPushed() { return false; }
    virtual void setWasPushed(bool pushed) { }
    virtual float getDensity() { return 1000.f; }
    virtual float getVolume()  { return 1.f; }

    virtual void addSurfaceObj(AbstractTimePhysicsModel *mdl) {}
    virtual void removeSurfaceObj(AbstractTimePhysicsModel *mdl) {}
    virtual void setSurface(AbstractTimePhysicsModel *mdl) {}
    virtual AbstractTimePhysicsModel* getSurface() { return NULL; }

private:
    Point m_ptPosition;
};

#endif
