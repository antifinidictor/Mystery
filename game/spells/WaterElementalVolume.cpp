/*
 * WaterElementalVolume.cpp
 */

#include "WaterElementalVolume.h"
#include "pwe/PartitionedWorldEngine.h"
#include "tpe/tpe.h"
#include "ForceField.h"
#include "game/GameManager.h"

WaterElementalVolume::WaterElementalVolume(uint id, uint texId, Box bxVolume, float fSwellRes, float fDensity) :
    ElementalVolume(id)
{

    Box bxRelativeVol =Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                            bxVolume.w,      bxVolume.h,      bxVolume.l);
    m_fSwellRes = fSwellRes;
    m_pxMap = new PixelMap(bxVolume.w * m_fSwellRes, bxVolume.l * m_fSwellRes,0);
    m_pRenderModel = new D3HeightmapRenderModel(this, texId, m_pxMap, bxRelativeVol);

    //Default swell values
    m_fSwellSize = MAX_COLOR_VAL;
    m_fSwellSpacingX = bxVolume.w / (m_pxMap->m_uiW - 1) * M_PI / 5.f;
    m_fSwellSpacingZ = bxVolume.l / (m_pxMap->m_uiH - 1) * M_PI / 5.f;

    m_pPhysicsModel = new TimePhysicsModel(bxCenter(bxVolume), fDensity);
    m_pPhysicsModel->addCollisionModel(new PixelMapCollisionModel(bxRelativeVol, m_pxMap));
    m_pPhysicsModel->setListener(this);

    setFlag(TPE_LIQUID, true);
    setFlag(TPE_STATIC, true);
    setFlag(D3RE_TRANSPARENT, true);
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
    bxVolume.w = pt.get(keyBase + ".vol.w", 0.f);
    bxVolume.h = pt.get(keyBase + ".vol.h", 0.f);
    bxVolume.l = pt.get(keyBase + ".vol.l", 0.f);
    float fDensity = pt.get(keyBase + ".density", DENSITY_WATER);
    WaterElementalVolume *obj = new WaterElementalVolume(uiId, uiTexId, bxVolume, 1.f, fDensity);
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
    pt.put(keyBase + ".tex", m_pRenderModel->getTexture());
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

    //Update swells
    float fTime = time / 1000.f;
    for(uint x = 0; x < m_pxMap->m_uiW; ++x) {
        for(uint z = 0; z < m_pxMap->m_uiH; ++z) {
            m_pxMap->m_pData[x][z] = uint((m_fSwellSize / 2.f)
                * sin((x + fTime) * M_PI / 5.f)
                * sin((z + fTime) * M_PI / 5.f)
                + m_fSwellSize / 2);
        }
    }

    return false;
}


void
WaterElementalVolume::setVolume(float fVolume) {
}

float
WaterElementalVolume::getVolume() {
    return m_pPhysicsModel->getVolume();
}
