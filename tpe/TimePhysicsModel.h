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
class CollisionModel;

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
    virtual void update(float fDeltaTime) = 0;
    virtual void handleCollisionEvent(HandleCollisionData *dat) = 0;
    virtual float getDensity() = 0;
    virtual float getVolume() = 0;

    virtual void addSurfaceObj(AbstractTimePhysicsModel *mdl) = 0;
    virtual void removeSurfaceObj(AbstractTimePhysicsModel *mdl) = 0;
    virtual void setSurface(PhysicsModel *mdl) = 0;
    virtual AbstractTimePhysicsModel* getSurface() = 0;
    virtual uint addCollisionModel(CollisionModel* mdl) = 0;
    virtual void removeCollisionModel(uint id) = 0;
    virtual CollisionModel *getCollisionModel(uint id) = 0;
    virtual uint getNumModels() = 0;
    virtual bool getPhysicsChanged() = 0;
    virtual void setPhysicsChanged(bool val) = 0;
};

/*
 * TimePhysicsModel
 * The basic time physics model
 */
class TimePhysicsModel : public AbstractTimePhysicsModel {
public:
    TimePhysicsModel(Point ptPos, float fDensity = 1000.f);   //Density in kg/m^3
    virtual ~TimePhysicsModel();

    virtual Point getPosition() { return m_ptPos; }//Point(m_bxVolume); }
    virtual Point getCenter() { return bxCenter(m_bxVolume) + m_ptPos; }
    virtual Point getLastVelocity() { return m_ptLastMotion; }
    virtual Box   getCollisionVolume() { return m_bxVolume + m_ptPos; }
    virtual void  moveBy(const Point &ptShift);
    virtual void  applyForce(const Point &ptForce);

    virtual void  handleCollisionEvent(HandleCollisionData *dat);
    virtual void  update(float fDeltaTime);

    //Time physics model properties
    virtual void mulTimeDivisor(float fTimeEffect) { m_fTimeDivisor *= fTimeEffect; }
    virtual float getMass() { return m_fMass; }
    virtual void setListener(Listener *pListener) { m_pListener = pListener; }
    virtual void clearVerticalVelocity() { m_ptVelocity.y = 0.f; }
    virtual bool wasPushed() { return m_bWasPushed; }
    virtual void setWasPushed(bool pushed) { m_bWasPushed = pushed; }

    virtual float getDensity() { return m_fDensity; }
    virtual float getVolume()  { return m_fMass / m_fDensity; }

    virtual void addSurfaceObj(AbstractTimePhysicsModel *mdl);
    virtual void removeSurfaceObj(AbstractTimePhysicsModel *mdl);
    virtual void setSurface(PhysicsModel *mdl);
    virtual AbstractTimePhysicsModel* getSurface() { return m_pObjImOn; }

    virtual uint addCollisionModel(CollisionModel* mdl);
    virtual void removeCollisionModel(uint id);
    virtual CollisionModel* getCollisionModel(uint id);
    virtual uint getNumModels() { return m_vCollisions.size(); }
    virtual void resetVolume();

    virtual bool getPhysicsChanged() { return m_bPhysicsChanged; }
    virtual void setPhysicsChanged(bool hasChanged) { m_bPhysicsChanged = hasChanged; }

private:
    //Time physics model
    Point m_ptAcceleration,
          m_ptVelocity;
    Point m_ptLastMotion;
    Box   m_bxVolume;
    Point m_ptPos;
    float m_fFrictionEffect,
          m_fFrictionAffect;
    float m_fTimeDivisor;
    float m_fMass, m_fDensity;
    std::vector<CollisionModel*> m_vCollisions;

    //Listener
    Listener *m_pListener;
    bool m_bWasPushed;
    bool m_bIsCleaning; //Don't do some ops when cleaning up
    bool m_bPhysicsChanged;
    std::list<AbstractTimePhysicsModel*> m_lsObjsOnMe;
    AbstractTimePhysicsModel *m_pObjImOn;
};

/*
 * NullTimePhysicsModel
 * Only tracks position.  No volume to speak of.
 */
class NullTimePhysicsModel : public AbstractTimePhysicsModel {
public:
    NullTimePhysicsModel(const Point &ptPos) { m_ptPosition = ptPos; }
    virtual ~NullTimePhysicsModel() {}

    virtual Point getPosition() { return m_ptPosition; }
    virtual Point getCenter() { return m_ptPosition; }
    virtual Point getLastVelocity() { return Point(); }
    virtual Box   getCollisionVolume() { return Box(m_ptPosition.x, m_ptPosition.y, m_ptPosition.z, 0, 0, 0); }
    virtual void  moveBy(const Point &ptShift) { m_ptPosition += ptShift; }
    virtual void  applyForce(const Point &ptForce) {}

    virtual void  handleCollisionEvent(HandleCollisionData *dat) {}
    virtual void  update(float fDeltaTime) {}

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
    virtual void setSurface(PhysicsModel *mdl) {}
    virtual AbstractTimePhysicsModel* getSurface() { return NULL; }

    virtual uint addCollisionModel(CollisionModel* mdl) { return 0; }
    virtual void removeCollisionModel(uint id) {}
    virtual CollisionModel* getCollisionModel(uint id) { return NULL; }
    virtual uint getNumModels() { return 0; }
    virtual bool getPhysicsChanged() { return false; }
    virtual void setPhysicsChanged(bool hasChanged) { }

private:
    Point m_ptPosition;
};

#endif
