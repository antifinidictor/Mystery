#ifndef VORTON_H
#define VORTON_H

#include "mgeMath.h"
#include "SpacePartitionedContainer.h"

#include "mge/defs.h"
/*
 * Design thanks to Intel's fluid physics for games article
 */
class Vorton
{
public:
    Vorton(uint uiId, const Point &ptPos, float fRadius, const Point &ptInitVorticity);
    virtual ~Vorton();

    uint getId() { return m_uiId; }
    Box  getBounds() { return m_bxBounds; }

    /** Performs stretching/tilting and advection operations */
    void update(float fTimeQuantum);

    /** Exchanges particle velocity with the specified vorton */
    void exchangeVorticityWith(float fViscocity, Vorton *v);

protected:
private:
    uint m_uiId;
    Box m_bxBounds;
    //float m_fRadius;
    //Point m_ptPosition;
    Point m_ptVorticity;
    Point m_ptDeltaVorticity;

    //vector<Vorton*> m_vNearestNeighbors;
};

#endif // VORTON_H
