#ifndef VORTON_H
#define VORTON_H

#include "mgeMath.h"
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
    virtual ~Vorton();

    uint getId() { return m_uiId; }
    Box  getBounds() { return m_bxBounds; }
    virtual Point getPosition() { return bxCenter(m_bxBounds); }
    virtual void moveBy(const Point &ptShift) { m_bxBounds += ptShift; }

    /** Performs stretching/tilting and advection operations */
    void update(float fTimeQuantum, const Matrix<3,3> &matJacobian, const Point &ptVelocity);

    /** Exchanges particle velocity with the specified vorton */
    void exchangeVorticityWith(float fViscocity, Vorton *v);

    /** Determine the velocity due to this vorton at some position */
    Point velocityAt(const Point &pos);

    Point getVorticity() { return m_ptVorticity; }

protected:
private:
    uint m_uiId;
    Box m_bxBounds;
    //float m_fRadius;
    //Point m_ptPosition;
    Point m_ptVelocity;
    Point m_ptVorticity;
    Point m_ptDeltaVorticity;

    //vector<Vorton*> m_vNearestNeighbors;
};

#endif // VORTON_H
