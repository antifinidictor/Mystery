#include "HmapSurface.h"
#include "pwe/PartitionedWorldEngine.h"
HmapSurface::HmapSurface(uint id, uint texId, const std::string &sMapFile, const Box &bxVolume) {
    m_uiId = id;
    m_uiFlags = 0;
    m_sMapFile = sMapFile;
    m_pxMap = new PixelMap(sMapFile, 0);    //not going to worry about id for now
    Box bxRelativeVol = Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                            bxVolume.w, bxVolume.h, bxVolume.l);
    m_pPhysicsModel = new TimePhysicsModel(this, bxCenter(bxVolume), 10000.f);
    m_pPhysicsModel->addCollisionModel(new PixelMapCollisionModel(bxRelativeVol, m_pxMap));
    m_pPhysicsModel->setListener(this);
    m_pRenderModel = new D3HeightmapRenderModel(m_pPhysicsModel, texId, m_pxMap, bxRelativeVol);

    setFlag(TPE_STATIC, true);
}

HmapSurface::~HmapSurface()
{
    PWE::get()->freeId(getId());
    delete m_pxMap;
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

bool
HmapSurface::update(float fDeltaTime) {
    return false;
}

int
HmapSurface::callBack(uint uiEventHandlerId, void *data, uint uiEventId) {
    return EVENT_DROPPED;
}

GameObject*
HmapSurface::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint uiId = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    uint uiTexId = pt.get(keyBase + ".tex", 0);
    std::string file = pt.get(keyBase + ".map", "res/world/defaultMap.bmp");
    Box bxVolume;
    bxVolume.x = pt.get(keyBase + ".vol.x", 0.f);
    bxVolume.y = pt.get(keyBase + ".vol.y", 0.f);
    bxVolume.z = pt.get(keyBase + ".vol.z", 0.f);
    bxVolume.w = pt.get(keyBase + ".vol.w", 0.f);
    bxVolume.h = pt.get(keyBase + ".vol.h", 0.f);
    bxVolume.l = pt.get(keyBase + ".vol.l", 0.f);
    HmapSurface *obj = new HmapSurface(uiId, uiTexId, file, bxVolume);
    Color cr;
    cr.r = pt.get(keyBase + ".cr.r", 0xFF);
    cr.g = pt.get(keyBase + ".cr.g", 0xFF);
    cr.b = pt.get(keyBase + ".cr.b", 0xFF);
    obj->m_pRenderModel->setColor(cr);
    return obj;
}

void
HmapSurface::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    pt.put(keyBase + ".id", getId());
    pt.put(keyBase + ".tex", m_pRenderModel->getTexture());
    pt.put(keyBase + ".map", m_sMapFile);
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
