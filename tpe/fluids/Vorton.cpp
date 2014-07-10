#include "Vorton.h"
#include "d3re/d3re.h"
#include "mge/ConfigManager.h"
static bool s_bDisplayBounds;
using namespace std;
/*
 * Vorticity equations
 * Change in vorticity = stretching/tilting + Viscous diffusion + Buoyancy + External torque
 * dw/dt =                (w dot grad) * v  + v * grad^2 * w  + (p x p) / p^2 + T
 */

Vorton::Vorton(uint uiId, const Point &ptPos, float fRadius, const Point &ptInitVorticity)
    :   m_uiId(uiId),
        m_fRadius(fRadius),
        m_ptPosition(ptPos),
        m_ptVorticity(ptInitVorticity),
        m_ptDeltaVorticity(Vec3f())
{
    s_bDisplayBounds = ConfigManager::get()->get("test.fluids.drawBounds", false);
}

Vorton::Vorton(const Vorton &v)
    :   m_uiId(v.m_uiId),
        m_fRadius(v.m_fRadius),
        m_ptPosition(v.m_ptPosition),
        m_ptDeltaVorticity(v.m_ptDeltaVorticity)
{
}

Vorton::~Vorton()
{
    //dtor
}

void
Vorton::update(float fTimeQuantum, const Matrix<3,3> &matJacobian, const Point &ptVelocity) {

#if 1
printf(__FILE__" %d: Vorton %d being updated @ pos (%f,%f,%f), vort (%f,%f,%f), delta vort (%f,%f,%f), vel mag = %f\n",
    __LINE__, getId(),
    m_ptPosition.x, m_ptPosition.y, m_ptPosition.z,
    m_ptVorticity.x, m_ptVorticity.y, m_ptVorticity.z,
    m_ptDeltaVorticity.x, m_ptDeltaVorticity.y, m_ptDeltaVorticity.z,
    ptVelocity.magnitude()
);
#endif
    //Stretching and tilting (halve this to preserve stability)
    m_ptDeltaVorticity += matMult(m_ptVorticity, matJacobian);

#if DEBUG_VORTONS
printf(__FILE__" %d: Vorton %d being updated @ pos (%f,%f,%f), vort (%f,%f,%f), delta vort (%f,%f,%f)\n",
    __LINE__, getId(),
    m_ptPosition.x, m_ptPosition.y, m_ptPosition.z,
    m_ptVorticity.x, m_ptVorticity.y, m_ptVorticity.z,
    m_ptDeltaVorticity.x, m_ptDeltaVorticity.y, m_ptDeltaVorticity.z
);
#endif
    //Mix in adjusted vorticity
    m_ptVorticity += m_ptDeltaVorticity * fTimeQuantum * 0.05f;
    m_ptDeltaVorticity = Point();
#if DEBUG_VORTONS
printf(__FILE__" %d: Vorton %d being updated @ pos (%f,%f,%f), vort (%f,%f,%f), delta vort (%f,%f,%f)\n",
    __LINE__, getId(),
    m_ptPosition.x, m_ptPosition.y, m_ptPosition.z,
    m_ptVorticity.x, m_ptVorticity.y, m_ptVorticity.z,
    m_ptDeltaVorticity.x, m_ptDeltaVorticity.y, m_ptDeltaVorticity.z
);
#endif

    //Advect each vorton
#if DEBUG_VORTONS
printf(__FILE__" %d: Vorton %d being updated @ pos (%f,%f,%f), vort (%f,%f,%f), delta vort (%f,%f,%f)\n",
    __LINE__, getId(),
    m_ptPosition.x, m_ptPosition.y, m_ptPosition.z,
    m_ptVorticity.x, m_ptVorticity.y, m_ptVorticity.z,
    m_ptDeltaVorticity.x, m_ptDeltaVorticity.y, m_ptDeltaVorticity.z
);
#endif
    m_ptPosition += ptVelocity * fTimeQuantum;
    //m_ptVelocity = ptVelocity;
#if DEBUG_VORTONS
printf(__FILE__" %d: Vorton %d being updated @ pos (%f,%f,%f), vort (%f,%f,%f), delta vort (%f,%f,%f)\n",
    __LINE__, getId(),
    m_ptPosition.x, m_ptPosition.y, m_ptPosition.z,
    m_ptVorticity.x, m_ptVorticity.y, m_ptVorticity.z,
    m_ptDeltaVorticity.x, m_ptDeltaVorticity.y, m_ptDeltaVorticity.z
);
#endif


    if(s_bDisplayBounds) {
        D3RE::get()->drawCircle(m_ptPosition, m_ptVorticity.magnitude(), Color(0, 0, 255));
    }
}

void
Vorton::exchangeVorticityWith(float fViscocity, Vorton *v) {
    Point ptVorticityExchange = (m_ptVorticity - v->m_ptVorticity) *  fViscocity;
    float fRemainingVorticity = 1.f - fViscocity;

#if DEBUG_VORTONS
printf(__FILE__" %d: Vorton %d being mixed with vorton %d @ pos (%f,%f,%f), vort (%f,%f,%f)\n",__LINE__, getId(), v->getId(), m_ptPosition.x, m_ptPosition.y, m_ptPosition.z, m_ptVorticity.x, m_ptVorticity.y, m_ptVorticity.z);
#endif
    //Adjust delta vorticity of me
    m_ptVorticity = m_ptVorticity * fRemainingVorticity + ptVorticityExchange;
//float meBefore, meAfter, themBefore, themAfter;
//meBefore = m_ptDeltaVorticity.magnitude();
//themBefore = v->m_ptDeltaVorticity.magnitude();
//    m_ptDeltaVorticity += ptVorticityExchange;
#if DEBUG_VORTONS
printf(__FILE__" %d: Vorton %d being mixed with vorton %d @ pos (%f,%f,%f), vort (%f,%f,%f)\n",__LINE__, getId(), v->getId(), m_ptPosition.x, m_ptPosition.y, m_ptPosition.z, m_ptVorticity.x, m_ptVorticity.y, m_ptVorticity.z);
#endif
    //Adjust delta vorticity of them
    v->m_ptVorticity = v->m_ptVorticity * fRemainingVorticity - ptVorticityExchange;
#if DEBUG_VORTONS
printf(__FILE__" %d: Vorton %d being mixed with vorton %d @ pos (%f,%f,%f), vort (%f,%f,%f)\n",__LINE__, getId(), v->getId(), m_ptPosition.x, m_ptPosition.y, m_ptPosition.z, m_ptVorticity.x, m_ptVorticity.y, m_ptVorticity.z);
#endif
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
    float radius2 = m_fRadius * m_fRadius;
    float dist2 = diff.magSq();
    float oneOverDist = 1.f / sqrt(dist2);
    float distLaw = (dist2 < radius2) ? (1.f / m_fRadius / radius2) : (oneOverDist / dist2);
    //float distLaw = (dist2 < radius2) ? (oneOverDist / radius2) : (oneOverDist / dist2);
    float oneOverFourPi = 1 / M_PI / 4;
#if DEBUG_VORTONS
printf(__FILE__" %d: diff = (%f,%f,%f), rad2 = %f, dist2 = %f, oneOverDist = %f, distLaw = %f, oneOver4pi = %f\n",__LINE__,
    diff.x, diff.y, diff.z,
    radius2, dist2, oneOverDist, distLaw, oneOverFourPi
);
#endif
    return cross(m_ptVorticity, diff) * oneOverFourPi * ( 8.0f * radius2 * m_fRadius ) * distLaw;
}
