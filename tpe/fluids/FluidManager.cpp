
#include "FluidManager.h"
#include "tpe/CollisionModel.h"

FluidManager::FluidManager(Positionable *pParent, Box bxRelativeBounds, float fCellSize)
    :   m_igVelocities(pParent, bxRelativeBounds, fCellSize),
        m_igJacobians(pParent, bxRelativeBounds, fCellSize)
{
    m_mutex = SDL_CreateMutex();
}

FluidManager::~FluidManager() {
    SDL_DestroyMutex(m_mutex);
}

void
FluidManager::onVortonUpdated() {
    SDL_LockMutex(m_mutex);
    if(++numVortonsUpdated == m_vVortons.size()) {
        //Reset prior to next update
        numVortonsUpdated = 0;

        aggregateVortons();

        updateVelocityGrid();

        updateJacobianGrid();
    }
    SDL_UnlockMutex(m_mutex);
}

void
FluidManager::aggregateVortons() {
}

void
FluidManager::updateVelocityGrid() {
    Box bxBounds = m_igVelocities.getBounds();
    int iSizeX = m_igVelocities.getSizeX();
    int iSizeY = m_igVelocities.getSizeY();
    int iSizeZ = m_igVelocities.getSizeZ();
    for(int x = 0; x < iSizeX; ++x) {
        Point ptPosition;
        ptPosition.x = (x * bxBounds.w / iSizeX) + bxBounds.x;
        for(int y = 0; y < iSizeY; ++y) {
            ptPosition.y = (y * bxBounds.h / iSizeY) + bxBounds.y;
            for(int z = 0; z < iSizeZ; ++z) {
                ptPosition.z = (z * bxBounds.l / iSizeZ) + bxBounds.z;

                //Calculate the velocity at this point
                //TODO: Create some sort of nested grid or octree here
                Vec3f &v3Velocity = m_igVelocities.at(x,y,z);
                v3Velocity = Point();
                for(std::vector<VortonCollisionModel*>::iterator v1 = m_vVortons.begin(); v1 != m_vVortons.end(); ++v1) {
                    v3Velocity += (*v1)->velocityAt(ptPosition);
                }
            }
        }
    }
}

void
FluidManager::updateJacobianGrid() {
    //Jacobians can't be calculated entirely locally because they require information from surrounding grid cells
    Box bxBounds = m_igJacobians.getBounds();
    int iSizeX = m_igJacobians.getSizeX();
    int iSizeY = m_igJacobians.getSizeY();
    int iSizeZ = m_igJacobians.getSizeZ();
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
                Mat33 &matJacobian = m_igJacobians.at(x,y,z);
                const Vec3f &v3VelocityXMY0Z0 = m_igVelocities.at(x-1,y,  z);
                const Vec3f &v3VelocityXPY0Z0 = m_igVelocities.at(x+1,y,  z);

                const Vec3f &v3VelocityX0YMZ0 = m_igVelocities.at(x,  y-1,z);
                const Vec3f &v3VelocityX0YPZ0 = m_igVelocities.at(x,  y+1,z);

                const Vec3f &v3VelocityX0Y0ZM = m_igVelocities.at(x,  y,  z-1);
                const Vec3f &v3VelocityX0Y0ZP = m_igVelocities.at(x,  y,  z+1);

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
