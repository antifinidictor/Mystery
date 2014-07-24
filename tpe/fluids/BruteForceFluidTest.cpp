#include "BruteForceFluidTest.h"
#include "pwe/PartitionedWorldEngine.h"
#include "d3re/d3re.h"
#include "tpe/tpe.h"

using namespace std;

BruteForceFluidTest::BruteForceFluidTest(PixelMap *pxHmap, Box bxBounds, int numVortons, float fCellSize, float fViscocity)
    :   m_pRenderModel(new ContainerRenderModel(NULL, Rect())),
        m_pPhysicsModel(new NullTimePhysicsModel(Point())),
        m_vVortons(numVortons, Vorton(0,Point(),0.f,Point())),
        m_cgVelocities(m_pPhysicsModel, bxBounds, fCellSize),
        m_cgJacobians(m_pPhysicsModel, bxBounds, fCellSize)
{
    m_uiId = PWE::get()->genId();
    m_uiFlags = 0;
    m_pxHmap = pxHmap;
    m_fViscocity = fViscocity;
    int w = bxBounds.w;
    int h = bxBounds.h;
    int l = bxBounds.l;

    for(int i = 0; i < numVortons; ++i) {
        Point ptPos = Point(
            (rand() % (int)w) + bxBounds.x,
            (rand() % (int)h) + bxBounds.y,
            (rand() % (int)l) + bxBounds.z
        );
        Point ptInitVorticity = Point(
            ((rand() % 2) - 1.f) * 0.1f,
            ((rand() % 2) - 1.f) * 0.1f,
            ((rand() % 2) - 1.f) * 0.1f
        );
        float fVortonRadius = 0.01f;
        m_vVortons[i] = (Vorton(i, ptPos,fVortonRadius,ptInitVorticity));

        D3SpriteRenderModel *pSpriteModel = new D3SpriteRenderModel(
            &m_vVortons[i],
            D3RE::get()->getImageId("mouse"),
            Rect(-0.2,-0.2,0.4,0.4)
        );
        m_pRenderModel->add(i, pSpriteModel);
    }
    setFlag(TPE_STATIC, true);
    setFlag(TPE_PASSABLE,true);
}

BruteForceFluidTest::~BruteForceFluidTest() {
    delete m_pPhysicsModel;
    delete m_pRenderModel;
}


GameObject*
BruteForceFluidTest::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    return NULL;
}

void
BruteForceFluidTest::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
}

bool
BruteForceFluidTest::update(float fDeltaTime) {
    #define iter_t vector<Vorton>::iterator

    //Create an interpolatable velocity grid
    uint compVelTime, compJacTime, updateVortTime, shareVelTime;
    compVelTime = SDL_GetTicks();
    computeVelocities();
    compVelTime = SDL_GetTicks() - compVelTime;

    //Create an interpolatable Jacobian grid
    compJacTime = SDL_GetTicks();
    computeJacobians();
    compJacTime = SDL_GetTicks() - compJacTime;

    shareVelTime = SDL_GetTicks();
    shareVorticities(fDeltaTime);
    shareVelTime = SDL_GetTicks() - shareVelTime;

    updateVortTime = SDL_GetTicks();
    updateVortons(fDeltaTime);
    updateVortTime = SDL_GetTicks() - updateVortTime;

    printf("Vel: %d Jac: %d Share: %d Update: %d\n", compVelTime, compJacTime, shareVelTime, updateVortTime);

    //Update tracer particles
//printf(__FILE__" %d (press enter to take a step)\n",__LINE__);
//cin.get();
    return false;
}


int
BruteForceFluidTest::callBack(uint cID, void *data, uint uiEventId) {
    return EVENT_DROPPED;
}

void
BruteForceFluidTest::computeJacobians() {
    //Jacobians can't be calculated entirely locally because they require information from surrounding grid cells
    Box bxBounds = m_cgJacobians.getBounds();
    int iSizeX = m_cgJacobians.getSizeX();
    int iSizeY = m_cgJacobians.getSizeY();
    int iSizeZ = m_cgJacobians.getSizeZ();
    float fCellW = bxBounds.w / iSizeX;
    float fCellH = bxBounds.h / iSizeY;
    float fCellL = bxBounds.l / iSizeZ;

    for(int x = 0; x < iSizeX; ++x) {
        Point ptPosition;
        ptPosition.x = (x * bxBounds.w / iSizeX) + bxBounds.x;

        //This changes at the cell boundaries
        float fCellWDivisor;
        if(0 < x && x < iSizeX - 1) {
            fCellWDivisor = 2 * fCellW;
        } else {
            fCellWDivisor = fCellW;
        }

        for(int y = 0; y < iSizeY; ++y) {
            ptPosition.y = (y * bxBounds.h / iSizeY) + bxBounds.y;

            //This changes at the cell boundaries
            float fCellHDivisor;
            if(0 < y && y < iSizeY - 1) {
                fCellHDivisor = 2 * fCellH;
            } else {
                fCellHDivisor = fCellH;
            }

            for(int z = 0; z < iSizeZ; ++z) {
                ptPosition.z = (z * bxBounds.l / iSizeZ) + bxBounds.z;

                //This changes at the cell boundaries
                float fCellLDivisor;
                if(0 < z && z < iSizeZ - 1) {
                    fCellLDivisor = 2 * fCellL;
                } else {
                    fCellLDivisor = fCellL;
                }

                //Calculate the jacobian at this point
                Mat33 &matJacobian = m_cgJacobians.at(x,y,z);
                const Vec3f &v3VelocityXMY0Z0 = m_cgVelocities.at(x-1,y,  z);
                const Vec3f &v3VelocityXPY0Z0 = m_cgVelocities.at(x+1,y,  z);

                const Vec3f &v3VelocityX0YMZ0 = m_cgVelocities.at(x,  y-1,z);
                const Vec3f &v3VelocityX0YPZ0 = m_cgVelocities.at(x,  y+1,z);

                const Vec3f &v3VelocityX0Y0ZM = m_cgVelocities.at(x,  y,  z-1);
                const Vec3f &v3VelocityX0Y0ZP = m_cgVelocities.at(x,  y,  z+1);

                //Each row is made by combining x, y, z velocities
                #define X 0
                #define Y 1
                #define Z 2
                matJacobian[X][X] = (v3VelocityXPY0Z0.x - v3VelocityXMY0Z0.x) / fCellWDivisor;
                matJacobian[X][Y] = (v3VelocityXPY0Z0.y - v3VelocityXMY0Z0.y) / fCellWDivisor;
                matJacobian[X][Z] = (v3VelocityXPY0Z0.z - v3VelocityXMY0Z0.z) / fCellWDivisor;

                matJacobian[Y][X] = (v3VelocityX0YPZ0.x - v3VelocityX0YMZ0.x) / fCellHDivisor;
                matJacobian[Y][Y] = (v3VelocityX0YPZ0.y - v3VelocityX0YMZ0.y) / fCellHDivisor;
                matJacobian[Y][Z] = (v3VelocityX0YPZ0.z - v3VelocityX0YMZ0.z) / fCellHDivisor;

                matJacobian[Z][X] = (v3VelocityX0Y0ZP.x - v3VelocityX0Y0ZM.x) / fCellLDivisor;
                matJacobian[Z][Y] = (v3VelocityX0Y0ZP.y - v3VelocityX0Y0ZM.y) / fCellLDivisor;
                matJacobian[Z][Z] = (v3VelocityX0Y0ZP.z - v3VelocityX0Y0ZM.z) / fCellLDivisor;
            }
        }
    }
}

void
BruteForceFluidTest::computeVelocities() {
    Box bxBounds = m_cgVelocities.getBounds();
    int iSizeX = m_cgVelocities.getSizeX();
    int iSizeY = m_cgVelocities.getSizeY();
    int iSizeZ = m_cgVelocities.getSizeZ();
    for(int x = 0; x < iSizeX; ++x) {
        Point ptPosition;
        ptPosition.x = (x * bxBounds.w / iSizeX) + bxBounds.x;
        for(int y = 0; y < iSizeY; ++y) {
            ptPosition.y = (y * bxBounds.h / iSizeY) + bxBounds.y;
            for(int z = 0; z < iSizeZ; ++z) {
                ptPosition.z = (z * bxBounds.l / iSizeZ) + bxBounds.z;

                //Calculate the velocity at this point
                Vec3f &v3Velocity = m_cgVelocities.at(x,y,z);
                v3Velocity = Point();
                for(iter_t v1 = m_vVortons.begin(); v1 != m_vVortons.end(); ++v1) {
                    v3Velocity += v1->velocityAt(ptPosition);
                }
            }
        }
    }
}

void
BruteForceFluidTest::shareVorticities(float fDeltaTime) {

    //Share vorticities among themselves
    for(iter_t v0 = m_vVortons.begin(); v0 != m_vVortons.end(); ++v0) {
        iter_t v1 = v0;
        v1++;
        for(; v1 != m_vVortons.end(); ++v1) {
            v0->exchangeVorticityWith(m_fViscocity, &*v1);
        }
    }
}

void
BruteForceFluidTest::updateVortons(float fDeltaTime) {

    //Update vortons: Stretch & Tilt, advection
    for(iter_t v0 = m_vVortons.begin(); v0 != m_vVortons.end(); ++v0) {
        Point ptPosition = v0->getPosition();

        Vec3f v3Velocity = m_cgVelocities.getAt(ptPosition);
        Mat33 matJacobian = m_cgJacobians.getAt(ptPosition);

        v0->update(fDeltaTime, matJacobian, v3Velocity);

        Point ptVorticity = v0->getVorticity();
        Point ptDiff = v0->getPosition() - ptPosition;
        printf("v%d's vorticity: (%2.2f,%2.2f,%2.2f), move by (%2.2f,%2.2f,%2.2f)\n",
               v0->getId(),
               ptVorticity.x, ptVorticity.y, ptVorticity.z,
               ptDiff.x, ptDiff.y, ptDiff.z);
    }
}
