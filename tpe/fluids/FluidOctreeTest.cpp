#include "FluidOctreeTest.h"
#include "mge/ConfigManager.h"

FluidOctreeTest::FluidOctreeTest()
    :   m_pRenderModel(new ContainerRenderModel(Rect())),
        m_pPhysicsModel(new NullTimePhysicsModel(Point()))
{
    ConfigManager *config = ConfigManager::get();
    int numVorts = config->get("test.fluid.numVortons", 0);
    Box vol      = config->get("test.fluid.volume", Box());
    float visc   = config->get("test.fluid.viscocity", 0.1f);
    float resOct = config->get("test.fluid.resolution.octree", 2.f);
    float resVel = config->get("test.fluid.resolution.velocity", 2.f);
    float resJac = config->get("test.fluid.resolution.jacobian", 2.f);
    float rad    = config->get("test.fluid.radius", 0.1f);
    m_pRoot = new FluidOctree(NULL, 0xA, vol, resOct, resVel, resJac);
    for(int i = 0; i < numVorts; ++i) {
        Point ptPos = Point(
            (rand() % (int)vol.w) + vol.x,
            (rand() % (int)vol.h) + vol.y,
            (rand() % (int)vol.l) + vol.z
        );
        Point ptInitVorticity = Point(
            ((rand() % 2) - 1.f) * 0.1f,
            ((rand() % 2) - 1.f) * 0.1f,
            ((rand() % 2) - 1.f) * 0.1f
        );
        Vorton *vort = new Vorton(i, ptPos,rad,ptInitVorticity);

        D3SpriteRenderModel *pSpriteModel = new D3SpriteRenderModel(
            vort,
            D3RE::get()->getImageId("mouse"),
            Rect(-0.2,-0.2,0.4,0.4)
        );
        m_pRenderModel->add(i, pSpriteModel);
        m_pRoot->add(vort);
    }
}

FluidOctreeTest::~FluidOctreeTest()
{
    delete m_pRoot;
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}


GameObject*
FluidOctreeTest::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
}

void
FluidOctreeTest::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
}

bool
FluidOctreeTest::update(float fDeltaTime) {
    TimePhysicsEngine::get()->updateFluid(m_pRoot);
}


int
FluidOctreeTest::callBack(uint cID, void *data, uint uiEventId) {
    return EVENT_DROPPED;
}
