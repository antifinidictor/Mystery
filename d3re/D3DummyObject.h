/*
 * D3DummyObject
 * Used for HUD render models and such
 */

#ifndef D3_DUMMY_OBJECT_H
#define D3_DUMMY_OBJECT_H

#include "mge/GameObject.h"
#include "mge/PhysicsModel.h"

class D3DummyObject : public GameObject, public PhysicsModel {
public:
    D3DummyObject(Point ptPos) {
        m_ptPos = ptPos;
    }
    virtual ~D3DummyObject() {
    }

    //Listener
    virtual uint getId() { return 0; }
    virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId) { return EVENT_DROPPED; }

    //GameObject
    virtual bool getFlag(uint flag) { return false; }
    virtual void setFlag(uint flag, bool value) {}
    virtual bool update(uint time) { return false; }
    virtual uint getType() { return 0; }
    virtual const std::string getClass() { return "D3DummyObject"; }
    virtual RenderModel  *getRenderModel() { return NULL; }
    virtual PhysicsModel *getPhysicsModel() { return this; }
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase) {}

    //Physics model
    virtual Point getPosition() { return m_ptPos; }
    virtual Point getLastVelocity() { return Point(); }
    virtual void  moveBy(Point ptShift) { m_ptPos += ptShift; }
    virtual void  applyForce(Point force) { }
    virtual Box   getCollisionVolume() { return Box(); }
private:
    Point m_ptPos;
};

#endif //D3_DUMMY_OBJECT_H
