#include "Vorton.h"

/*
 * Vorticity equations
 * Change in vorticity = stretching/tilting + Viscous diffusion + Buoyancy + External torque
 * dw/dt =                (w dot grad) * v  + v * grad^2 * w  + (p x p) / p^2 + T
 */

Vorton::Vorton(uint uiId, const Point &ptPos, float fRadius, const Point &ptInitVorticity)
{
    m_uiId = uiId;
    m_bxBounds = Box(
        ptPos.x - fRadius,
        ptPos.y - fRadius,
        ptPos.z - fRadius,
        fRadius * 2,
        fRadius * 2,
        fRadius * 2
    );
    m_ptVorticity = ptInitVorticity;
}

Vorton::~Vorton()
{
    //dtor
}

void
Vorton::update(float fTimeQuantum) {
    //Mix in adjusted vorticity
    m_ptVorticity += m_ptDeltaVorticity;
    m_ptDeltaVorticity = Point();

    SpacePartitionedContainer<Matrix<3,3> > *jacobians;

    //Stretching and tilting

    //Advect each vorton
}

void
Vorton::exchangeVorticityWith(float fViscocity, Vorton *v) {
    //Adjust delta vorticity of me
    m_ptDeltaVorticity += (m_ptVorticity - v->m_ptVorticity) *  fViscocity;

    //Adjust delta vorticity of them
}
