#ifndef FLUID_MANAGER_H
#define FLUID_MANAGER_H
/*
 * Contains basic info for a fluid, but not information on, say, how to draw it
 */
#include "InterpGrid.h"
#include "mge/mgeMath.h"
#include "SDL.h"
#include <vector>

class VortonCollisionModel;

class FluidManager {
protected:
    typedef Matrix<3,3> Mat33;

    void aggregateVortons();
    void updateVelocityGrid();
    void updateJacobianGrid();

    InterpGrid<Vec3f> m_igVelocities;
    InterpGrid<Mat33> m_igJacobians;
    std::vector<VortonCollisionModel*> m_vVortons;
    uint numVortonsUpdated;

    SDL_mutex *m_mutex;
public:
    FluidManager(Positionable *pParent, Box bxRelativeBounds, float fCellSize);
    virtual ~FluidManager();

    void onVortonUpdated();

    Vec3f getVelocityAt(const Point &pos) { return m_igVelocities.getAt(pos); }
    Mat33 getJacobianAt(const Point &pos) { return m_igJacobians.getAt(pos); }

};

#endif
