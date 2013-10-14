/*
 * WaterElementalVolume.cpp
 */

#include "WaterElementalVolume.h"
#include "pwe/PartitionedWorldEngine.h"
#include "tpe/tpe.h"
#include "ForceField.h"
#include "game/GameManager.h"

WaterElementalVolume::WaterElementalVolume(uint id, uint texId, Box bxVolume, float fDensity) :
    ElementalVolume(id)
{

    Box bxRelativeVol =Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                            bxVolume.w,      bxVolume.h,      bxVolume.l);
    Image *img = D3RE::get()->getImage(texId);
    m_pRenderModel = new D3PrismRenderModel(this, bxRelativeVol);
    //Hidden faces not rendered
    m_pRenderModel->setTexture(NORTH, IMG_NONE);//img->m_uiID);
    m_pRenderModel->setTexture(SOUTH, img->m_uiID);
    m_pRenderModel->setTexture(EAST,  img->m_uiID);
    m_pRenderModel->setTexture(WEST,  img->m_uiID);
    m_pRenderModel->setTexture(UP,    img->m_uiID);
    m_pRenderModel->setTexture(DOWN,  IMG_NONE);//img->m_uiID);

    m_pPhysicsModel = new TimePhysicsModel(bxCenter(bxVolume), fDensity);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxRelativeVol));
    m_pPhysicsModel->setListener(this);

    setFlag(TPE_LIQUID, true);
    setFlag(TPE_STATIC, true);

    //Force Field test
    #if 0
    Point ptCenter = bxCenter(bxVolume);
    ForceField *field0 = new SourceForceField(ptCenter, 10000.0f);
#define FSIZE (20)
    ForceField *fieldOut = new LineForceField(ptCenter + Point(-FSIZE,0,0), ptCenter + Point(FSIZE,0,0), 100.0f);
    ForceField *fieldA0 = new LineForceField(ptCenter + Point(FSIZE,0,0), ptCenter + Point(FSIZE,0,FSIZE), 100.0f);
    ForceField *fieldA1 = new LineForceField(ptCenter + Point(FSIZE,0,FSIZE), ptCenter + Point(-FSIZE,0,FSIZE), 100.0f);
    ForceField *fieldA2 = new LineForceField(ptCenter + Point(-FSIZE,0,FSIZE), ptCenter + Point(-FSIZE,0,0), 100.0f);

    ForceField *fieldB0 = new LineForceField(ptCenter + Point(FSIZE,0,0), ptCenter + Point(FSIZE,0,FSIZE), 100.0f);
    ForceField *fieldB1 = new LineForceField(ptCenter + Point(FSIZE,0,-FSIZE), ptCenter + Point(-FSIZE,0,-FSIZE), 100.0f);
    ForceField *fieldB2 = new LineForceField(ptCenter + Point(-FSIZE,0,-FSIZE), ptCenter + Point(-FSIZE,0,0), 100.0f);
    addForceField(fieldOut);
    addForceField(fieldA0);
    addForceField(fieldA1);
    addForceField(fieldA2);
    addForceField(fieldB0);
    addForceField(fieldB1);
    addForceField(fieldB2);
    #endif
}

WaterElementalVolume::~WaterElementalVolume() {
    delete m_pPhysicsModel;
    delete m_pRenderModel;
}

GameObject*
WaterElementalVolume::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint uiId = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    uint uiTexId = pt.get(keyBase + ".tex", 0);
    Box bxVolume;
    bxVolume.x = pt.get(keyBase + ".vol.x", 0.f);
    bxVolume.y = pt.get(keyBase + ".vol.y", 0.f);
    bxVolume.z = pt.get(keyBase + ".vol.z", 0.f);
    bxVolume.w = pt.get(keyBase + ".vol.w", 0);
    bxVolume.h = pt.get(keyBase + ".vol.h", 0);
    bxVolume.l = pt.get(keyBase + ".vol.l", 0);
    float fDensity = pt.get(keyBase + ".density", DENSITY_WATER);
    WaterElementalVolume *obj = new WaterElementalVolume(uiId, uiTexId, bxVolume, fDensity);
    Color cr;
    cr.r = pt.get(keyBase + ".cr.r", 0x0);
    cr.g = pt.get(keyBase + ".cr.g", 0x0);
    cr.b = pt.get(keyBase + ".cr.b", 0xFF);
    obj->m_pRenderModel->setColor(cr);

    return obj;
}

void
WaterElementalVolume::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    pt.put(keyBase + ".id", getId());
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
    pt.put(keyBase + ".density", m_pPhysicsModel->getDensity());
}

bool
WaterElementalVolume::update(uint time) {
    Point ptMousePos = D3RE::get()->getMousePos();
    if(ptInXZRect(ptMousePos, m_pPhysicsModel->getCollisionVolume())) {
        m_pRenderModel->setColor(Color(0x00,0xFF,0xFF));
        GameManager::get()->addActiveVolume(this);
    } else {
        m_pRenderModel->setColor(Color(0x00,0x00,0xFF));
        GameManager::get()->removeActiveVolume(getId());
    }
    return false;
}
