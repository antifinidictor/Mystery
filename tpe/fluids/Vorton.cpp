#include "Vorton.h"

/*
 * Vorticity equations
 * Change in vorticity = stretching/tilting + Viscous diffusion + Buoyancy + External torque
 * dw/dt =                (w dot grad) * v  + v * grad^2 * w  + (p x p) / p^2 + T
 */

Vorton::Vorton()
{
    //ctor
}

Vorton::~Vorton()
{
    //dtor
}
