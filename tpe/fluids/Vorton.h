#ifndef VORTON_H
#define VORTON_H

#include "mge/mgeMath.h"
#include "SpacePartitionedContainer.h"

#include "mge/defs.h"
#include "mge/Positionable.h"

/*
 * Design thanks to Intel's fluid physics for games article
 */
class Vorton : public Positionable
{
public:
    Vorton(uint uiId, const Point &ptPos, float fRadius, const Point &ptInitVorticity);
    Vorton(const Vorton &v);
    virtual ~Vorton();

    uint getId() { return m_uiId; }
    Box  getBounds() { return Box(m_ptPosition.x, m_ptPosition.y, m_ptPosition.z, 0.f, 0.f, 0.f); }
    virtual Point getPosition() { return m_ptPosition; }
    virtual void moveBy(const Point &ptShift) { m_ptPosition += ptShift; }

    /** Performs stretching/tilting and advection operations */
    void update(float fTimeQuantum, const Matrix<3,3> &matJacobian, const Point &ptVelocity);

    /** Exchanges particle velocity with the specified vorton */
    void exchangeVorticityWith(float fViscocity, Vorton *v);

    /** Determine the velocity due to this vorton at some position */
    Point velocityAt(const Point &pos);

    /** Access the vorticity of this vorton */
    Point getVorticity() { return m_ptVorticity; }

    /* Used when aggregating vortons */
    void dividePosition(float divisor) { m_ptPosition /= divisor; }
    void accumVorticity(const Vec3f &vort) { m_ptVorticity += vort; }

protected:
private:
    uint m_uiId;
    float m_fRadius;
    Point m_ptPosition;
    //Point m_ptVelocity;
    Point m_ptVorticity;
    Point m_ptDeltaVorticity;

    //vector<Vorton*> m_vNearestNeighbors;
};

#endif // VORTON_H
