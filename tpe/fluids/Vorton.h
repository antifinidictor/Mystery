#ifndef VORTON_H
#define VORTON_H

#include "mge/defs.h"

class Vorton
{
public:
    Vorton();
    virtual ~Vorton();

    uint getId() { return m_uiId; }
    Box  getBounds() { return m_bxBounds; }

protected:
private:
    uint m_uiId;
    Box m_bxBounds;
    float m_fRadius;
    Point m_ptPosition;
    Point m_ptVorticity;

    //vector<Vorton*> m_vNearestNeighbors;
};

#endif // VORTON_H
