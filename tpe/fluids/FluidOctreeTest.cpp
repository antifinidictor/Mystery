#include "FluidOctreeTest.h"
#include "mge/ConfigManager.h"
#include "pwe/PartitionedWorldEngine.h"

FluidOctreeTest::FluidOctreeTest()
    :   m_pRenderModel(new ContainerRenderModel(NULL, Rect())),
        m_pPhysicsModel(new NullTimePhysicsModel(Point())),
        m_fCurDeltaTime(1.f)
{
#if DEBUG_VORTONS
printf(__FILE__" %d: Created new fluid octree test\n", __LINE__);
#endif
    m_uiId = PWE::get()->reserveId(100);

    ConfigManager *config = ConfigManager::get();
    int numVorts = config->get("test.fluid.numVortons", 5);
    Box vol      = config->get("test.fluid.volume", Box());
    float visc   = config->get("test.fluid.viscocity", 0.1f);
    float resOct = config->get("test.fluid.resolution.octree", 2.f);
    float resVel = config->get("test.fluid.resolution.velocity", 2.f);
    float resJac = config->get("test.fluid.resolution.jacobian", 2.f);
    float rad    = config->get("test.fluid.radius", 0.1f);
    m_pRoot = new FluidOctree(NULL, 0xA, vol, resOct, resVel, resJac);
#if DEBUG_VORTONS
printf(__FILE__" %d: %d vortons\n", __LINE__, numVorts);
#endif
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
//printf(__FILE__" %d: Created new fluid render model thing\n", __LINE__);
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
    return new FluidOctreeTest();
}

void
FluidOctreeTest::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
}

bool
FluidOctreeTest::update(float fDeltaTime) {
    m_fCurDeltaTime = fDeltaTime;

    //TimePhysicsEngine::get()->updateFluid(m_pRoot);
    //Schedule fluid updates
    int numUpdatesPerThread = ConfigManager::get()->get("test.fluid.numIndicesPerThread", 10);

    //Calculate the total number of updates we are waiting for
    const InterpGrid<Vec3f> *velGrid = m_pRoot->getVelocityGrid();
    uint uiVelSize = velGrid->getSizeX() * velGrid->getSizeY() * velGrid->getSizeZ();
    uint numVelBlocks = uiVelSize / numUpdatesPerThread;
    uint remVelUpdates = uiVelSize % numUpdatesPerThread;

    for(uint i = 0; i < numVelBlocks; ++i) {
        int minIndex = i * numUpdatesPerThread;
        WorklistItem *item = new FluidVelocityWorklistItem(m_pRoot, minIndex, minIndex + numUpdatesPerThread);
        MGE::get()->addItemToWorklist(item);
    }

    //The number of updates may not be evenly divisible by the number of indices per thread
    if(remVelUpdates > 0) {
        int minIndex = numVelBlocks * numUpdatesPerThread;
        WorklistItem *item = new FluidVelocityWorklistItem(m_pRoot, minIndex, minIndex + remVelUpdates);
        MGE::get()->addItemToWorklist(item);
    }

    //Calculate the total number of updates we are waiting for
    const InterpGrid<Matrix<3,3> > *jacGrid = m_pRoot->getJacobianGrid();
    uint uiJacSize = jacGrid->getSizeX() * jacGrid->getSizeY() * jacGrid->getSizeZ();
    uint numJacBlocks = uiJacSize / numUpdatesPerThread;
    uint remJacUpdates = uiJacSize % numUpdatesPerThread;

    for(uint i = 0; i < numJacBlocks; ++i) {
        int minIndex = i * numUpdatesPerThread;
        WorklistItem *item = new FluidJacobianWorklistItem(m_pRoot, minIndex, minIndex + numUpdatesPerThread);
        MGE::get()->addItemToWorklist(item);
    }

    //The number of updates may not be evenly divisible by the number of indices per thread
    if(remJacUpdates > 0) {
        int minIndex = numJacBlocks * numUpdatesPerThread;
        WorklistItem *item = new FluidJacobianWorklistItem(m_pRoot, minIndex, minIndex + remJacUpdates);
        MGE::get()->addItemToWorklist(item);
    }

    //Schedule the vorton updates/aggregation
    m_pRoot->scheduleUpdates(this);

    //Help update the worklist?
    D3RE::get()->drawBox(m_pPhysicsModel->getCollisionVolume(), Color(0, 255, 0));

    return false;
}

void
FluidOctreeTest::scheduleUpdate(Octree3dNode<Vorton> *node) {
    WorklistItem *item = new FluidOctreeWorklistItem(m_pRoot, (FluidOctreeNode*)node, m_fCurDeltaTime);
    MGE::get()->addItemToWorklist(item);
}


int
FluidOctreeTest::callBack(uint cID, void *data, uint uiEventId) {
    return EVENT_DROPPED;
}
