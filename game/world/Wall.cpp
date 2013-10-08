#include "Wall.h"
#include "pwe/PartitionedWorldEngine.h"

Wall::Wall(uint uiId, uint texTopId, uint texBottomId, uint texSideId, Box bxVolume, uint visibleFaces) {
    m_uiId = uiId;
    m_uiFlags = 0;

    Box bxRelativeVol =Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                            bxVolume.w,      bxVolume.h,      bxVolume.l);
    m_pRenderModel = new D3PrismRenderModel(this, bxRelativeVol);

    m_pRenderModel->setTexture(NORTH, ((visibleFaces & WALL_NORTH) ? texSideId : IMG_NONE));
    m_pRenderModel->setTexture(SOUTH, ((visibleFaces & WALL_SOUTH) ? texSideId : IMG_NONE));
    m_pRenderModel->setTexture(EAST,  ((visibleFaces & WALL_EAST)  ? texSideId : IMG_NONE));
    m_pRenderModel->setTexture(WEST,  ((visibleFaces & WALL_WEST)  ? texSideId : IMG_NONE));
    m_pRenderModel->setTexture(UP,    ((visibleFaces & WALL_UP)    ? texTopId  : IMG_NONE));
    m_pRenderModel->setTexture(DOWN,  ((visibleFaces & WALL_DOWN)  ? texBottomId : IMG_NONE));

    m_pPhysicsModel = new TimePhysicsModel(bxCenter(bxVolume), DENSITY_STONE);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxRelativeVol));

    setFlag(TPE_STATIC, true);
}

Wall::~Wall() {
    PWE::get()->freeId(getId());
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

GameObject*
Wall::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint uiId = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    Box bxVolume;
    bxVolume.x = pt.get(keyBase + ".vol.x", 0.f);
    bxVolume.y = pt.get(keyBase + ".vol.y", 0.f);
    bxVolume.z = pt.get(keyBase + ".vol.z", 0.f);
    bxVolume.w = pt.get(keyBase + ".vol.w", 0);
    bxVolume.h = pt.get(keyBase + ".vol.h", 0);
    bxVolume.l = pt.get(keyBase + ".vol.l", 0);

    //We will set other parameters once the initial wall has been set up
    Wall *wall = new Wall(uiId, IMG_NONE, IMG_NONE, IMG_NONE, bxVolume, 0);
    wall->m_pRenderModel->setTexture(NORTH, pt.get(keyBase + ".tex.north", (uint)IMG_NONE));
    wall->m_pRenderModel->setTexture(SOUTH, pt.get(keyBase + ".tex.south", (uint)IMG_NONE));
    wall->m_pRenderModel->setTexture(EAST,  pt.get(keyBase + ".tex.east",  (uint)IMG_NONE));
    wall->m_pRenderModel->setTexture(WEST,  pt.get(keyBase + ".tex.west",  (uint)IMG_NONE));
    wall->m_pRenderModel->setTexture(UP,    pt.get(keyBase + ".tex.up",    (uint)IMG_NONE));
    wall->m_pRenderModel->setTexture(DOWN,  pt.get(keyBase + ".tex.down",  (uint)IMG_NONE));
    Color cr;
    cr.r = pt.get(keyBase + ".cr.r", 0xFF);
    cr.g = pt.get(keyBase + ".cr.g", 0xFF);
    cr.b = pt.get(keyBase + ".cr.b", 0xFF);
    wall->m_pRenderModel->setColor(cr);
    return wall;
}

void
Wall::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    pt.put(keyBase + ".id", getId());
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
    pt.put(keyBase + ".tex.north", m_pRenderModel->getTexture(NORTH));
    pt.put(keyBase + ".tex.south", m_pRenderModel->getTexture(SOUTH));
    pt.put(keyBase + ".tex.east",  m_pRenderModel->getTexture(EAST));
    pt.put(keyBase + ".tex.west",  m_pRenderModel->getTexture(WEST));
    pt.put(keyBase + ".tex.up",    m_pRenderModel->getTexture(UP));
    pt.put(keyBase + ".tex.down",  m_pRenderModel->getTexture(DOWN));
}
