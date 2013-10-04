/*
 * SimplePhysicsObject
 */

#include "SimplePhysicsObject.h"
#include "pwe/PartitionedWorldEngine.h"

SimplePhysicsObject::SimplePhysicsObject(uint id, uint texId, Box bxVolume) {
    m_uiID = id;
    m_uiFlags = 0;

    Image *img = D3RE::get()->getImage(texId);
    m_pRenderModel = new D3PrismRenderModel(this, Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                                                       bxVolume.w,      bxVolume.h,      bxVolume.l));
    //Hidden faces not rendered
    m_pRenderModel->setTexture(NORTH, IMG_NONE);//img->m_uiID);
    m_pRenderModel->setTexture(SOUTH, img->m_uiID);
    m_pRenderModel->setTexture(EAST,  img->m_uiID);
    m_pRenderModel->setTexture(WEST,  img->m_uiID);
    m_pRenderModel->setTexture(UP,    img->m_uiID);
    m_pRenderModel->setTexture(DOWN,  IMG_NONE);//img->m_uiID);

    m_pPhysicsModel = new TimePhysicsModel(bxVolume);

}

SimplePhysicsObject::~SimplePhysicsObject() {
    PWE::get()->freeID(getID());
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

GameObject*
SimplePhysicsObject::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint uiId = PWE::get()->reserveID(pt.get(keyBase + ".id", 0));
    uint uiTexId = pt.get(keyBase + ".tex", 0);
    Box bxVolume;
    bxVolume.x = pt.get(keyBase + ".vol.x", 0.f);
    bxVolume.y = pt.get(keyBase + ".vol.y", 0.f);
    bxVolume.z = pt.get(keyBase + ".vol.z", 0.f);
    bxVolume.w = pt.get(keyBase + ".vol.w", 0);
    bxVolume.h = pt.get(keyBase + ".vol.h", 0);
    bxVolume.l = pt.get(keyBase + ".vol.l", 0);
    SimplePhysicsObject *obj = new SimplePhysicsObject(uiId, uiTexId, bxVolume);
    Color cr;
    cr.r = pt.get(keyBase + ".cr.r", 0xFF);
    cr.g = pt.get(keyBase + ".cr.g", 0xFF);
    cr.b = pt.get(keyBase + ".cr.b", 0xFF);
    obj->m_pRenderModel->setColor(cr);
    return obj;
}

void
SimplePhysicsObject::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    pt.put(keyBase + ".id", getID());
    pt.put(keyBase + ".tex", m_pRenderModel->getTexture(SOUTH));
    Box bxVolume = m_pPhysicsModel->getCollisionVolume();
    pt.put(keyBase + ".vol.x", bxVolume.x);
    pt.put(keyBase + ".vol.y", bxVolume.y);
    pt.put(keyBase + ".vol.z", bxVolume.z);
    pt.put(keyBase + ".vol.w", bxVolume.w);
    pt.put(keyBase + ".vol.h", bxVolume.h);
    pt.put(keyBase + ".vol.l", bxVolume.l);
    Color cr = m_pRenderModel->getColor();
    pt.put(keyBase + ".cr.r", cr.r);
    pt.put(keyBase + ".cr.g", cr.g);
    pt.put(keyBase + ".cr.b", cr.b);
}

