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
Vorton::update(float fTimeQuantum, const Matrix<3,3> &matJacobian, const Point &ptVelocity) {
    //Stretching and tilting (halve this to preserve stability)
    m_ptDeltaVorticity += matMult(m_ptVorticity, matJacobian);

    //Mix in adjusted vorticity
    m_ptVorticity += m_ptDeltaVorticity * fTimeQuantum * 0.05f;
    m_ptDeltaVorticity = Point();

    //Advect each vorton
    m_bxBounds += ptVelocity * fTimeQuantum;
    m_ptVelocity = ptVelocity;
}

void
Vorton::exchangeVorticityWith(float fViscocity, Vorton *v) {
    Point ptVorticityExchange = (m_ptVorticity - v->m_ptVorticity) *  fViscocity;
    float fRemainingVorticity = 1.f - fViscocity;
    //Adjust delta vorticity of me
    m_ptVorticity = m_ptVorticity * fRemainingVorticity + ptVorticityExchange;
//float meBefore, meAfter, themBefore, themAfter;
//meBefore = m_ptDeltaVorticity.magnitude();
//themBefore = v->m_ptDeltaVorticity.magnitude();
//    m_ptDeltaVorticity += ptVorticityExchange;
    //Adjust delta vorticity of them
    v->m_ptVorticity = v->m_ptVorticity * fRemainingVorticity - ptVorticityExchange;
//    v->m_ptDeltaVorticity -= ptVorticityExchange;
//meAfter = m_ptDeltaVorticity.magnitude();
//themAfter = v->m_ptDeltaVorticity.magnitude();
//printf("Mag = %f, me: %f/%f, them: %f/%f\n", m_ptDeltaVorticity.magnitude(), meBefore, meAfter, themBefore, themAfter);

}
#if 0
#define VORTON_ACCUMULATE_VELOCITY_private( vVelocity , vPosQuery , mPosition , mVorticity , mRadius )      \
{                                                                                                           \
    const Vec3          vNeighborToSelf     = vPosQuery - mPosition ;                                       \
    const float         radius2             = mRadius * mRadius ;                                           \
    const float         dist2               = vNeighborToSelf.Mag2() + sAvoidSingularity ;                  \
    const float         oneOverDist         = finvsqrtf( dist2 ) ;                                          \
    const Vec3          vNeighborToSelfDir  = vNeighborToSelf * oneOverDist ;                               \
    /* If the reciprocal law is used everywhere then when 2 vortices get close, they tend to jettison. */   \
    /* Mitigate this by using a linear law when 2 vortices get close to each other. */                      \
    const float         distLaw             = ( dist2 < radius2 )                                           \
                                                ?   /* Inside vortex core */                                \
                                                ( oneOverDist / radius2 )                                   \
                                                :   /* Outside vortex core */                               \
                                                ( oneOverDist / dist2 ) ;                                   \
    vVelocity +=  OneOverFourPi * ( 8.0f * radius2 * mRadius ) * mVorticity ^ vNeighborToSelf * distLaw ;   \
}
#endif

Point
Vorton::velocityAt(const Point &pos) {
    //Taken directly from intel code
    Point diff = pos - getPosition();
    float radius = m_bxBounds.w / 2.f;
    float radius2 = radius * radius;
    float dist2 = diff.magSq();
    float oneOverDist = 1.f / sqrt(dist2);
    float distLaw = (dist2 < radius2) ? (oneOverDist / radius2) : (oneOverDist / dist2);
    float oneOverFourPi = 1 / M_PI / 4;
    return cross(m_ptVorticity, diff) * oneOverFourPi * ( 8.0f * radius2 * radius ) * distLaw;
}
