#ifndef FLUIDOCTREE3D_H
#define FLUIDOCTREE3D_H

//#include "mge/Octree3d.h"
#include "Vorton.h"
#include <map>

/*
 * Design thanks to Intel's fluid physics for games article
 */

class FluidOctree3d
{
public:

    FluidOctree3d(const Box &bxBounds, float fMinResolution = 1.f);
    virtual ~FluidOctree3d();

    //Adds object to the appropriate list
    bool add(Vorton *obj, bool bForce = true);

    //Removes from the list but does not call delete
    bool remove(uint uiObjId);

    //Removes from the list and calls delete
    bool erase(uint uiObjId);

    //Returns a reference to the appropriate object
    Vorton *find(uint uiObjId);

    //The Octree is essentially incomplete, you need to implement your own searching functions & other ops

protected:
//private:
    enum QuadrantNames {
        QUAD_FIRST = 0,                     //Used for iterating
        QUAD_POSX_POSY_POSZ = QUAD_FIRST,
        QUAD_POSX_POSY_NEGZ,
        QUAD_POSX_NEGY_POSZ,
        QUAD_POSX_NEGY_NEGZ,
        QUAD_NEGX_POSY_POSZ,
        QUAD_NEGX_POSY_NEGZ,
        QUAD_NEGX_NEGY_POSZ,
        QUAD_NEGX_NEGY_NEGZ,
        QUAD_NUM_QUADS      //Used for iterating
    };
    #define QUAD_X_MASK 0x4
    #define QUAD_Y_MASK 0x2
    #define QUAD_Z_MASK 0x1

    bool getChildBounds(int iQuadName, Box &bx);
    void updateEmptiness();
    bool empty() { return m_bEmpty; }

    //Fellow octree information
    //Octree3d *m_pParent;
    FluidOctree3d *m_aChildren[QUAD_NUM_QUADS];

    //Object information
    typedef std::map<uint,Vorton*>::iterator iter_t;
    std::map<uint, Vorton*> m_mObjs;
    bool m_bEmpty;

    //Boundary information
    Box m_bxBounds;
    float m_fMinResolution;
};

#endif // FLUIDOCTREE3D_H
