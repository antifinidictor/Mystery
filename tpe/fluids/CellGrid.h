#ifndef CELLGRID_H
#define CELLGRID_H

#include "mge/defs.h"
#include "mge/Positionable.h"
#include <vector>

//Based on UniformGrid from the Intel fluid simulation article
//Maintains one item at each grid point.  Gets the interpolated value at the specified points.
template<class ItemType>
class CellGrid
{
public:
    CellGrid(Positionable *pParent, Box bxRelativeBounds, float fCellSize)
        :   m_iNumX((int)(bxRelativeBounds.w / fCellSize)),
            m_iNumY((int)(bxRelativeBounds.h / fCellSize)),
            m_iNumZ((int)(bxRelativeBounds.l / fCellSize)),
            m_bxBounds(bxRelativeBounds),
            m_pParent(pParent),
            m_vItems(m_iNumX * m_iNumY * m_iNumZ)
    {
    }

    CellGrid(Positionable *pParent, Box bxRelativeBounds, int numX, int numY, int numZ)
        :   m_iNumX(numX),
            m_iNumY(numY),
            m_iNumZ(numZ),
            m_bxBounds(bxRelativeBounds),
            m_pParent(pParent),
            m_vItems(m_iNumX * m_iNumY * m_iNumZ)
    {
    }

    virtual ~CellGrid() {
        m_vItems.clear();
    }

#define TO_INDEX(x, y, z) (x * m_iNumY * m_iNumZ + y * m_iNumZ + z)
#define BOUND(min, val, max) ((val < min) ? min : ((val < max) ? val : max-1))
    ItemType getAt(const Point &pos) {
        //Get integer x/y/z indices
        Point ipos = pointToIndexFloats(pos);
        int minX = BOUND(0, (int)floor(ipos.x), m_iNumX);
        int minY = BOUND(0, (int)floor(ipos.y), m_iNumY);
        int minZ = BOUND(0, (int)floor(ipos.z), m_iNumZ);
        int maxX = BOUND(0, (int)ceil(ipos.x), m_iNumX);
        int maxY = BOUND(0, (int)ceil(ipos.y), m_iNumY);
        int maxZ = BOUND(0, (int)ceil(ipos.z), m_iNumZ);
        float weightX = ipos.x - minX;
        float weightY = ipos.y - minY;
        float weightZ = ipos.z - minZ;

        //Interpolate linearly
        ItemType itemInterpX_minY_minZ = m_vItems[TO_INDEX(minX, minY, minZ)] * (1.f - weightX) + m_vItems[TO_INDEX(maxX, minY, minZ)] * (weightX);
        ItemType itemInterpX_minY_maxZ = m_vItems[TO_INDEX(minX, minY, maxZ)] * (1.f - weightX) + m_vItems[TO_INDEX(maxX, minY, maxZ)] * (weightX);
        ItemType itemInterpX_maxY_minZ = m_vItems[TO_INDEX(minX, maxY, minZ)] * (1.f - weightX) + m_vItems[TO_INDEX(maxX, maxY, minZ)] * (weightX);
        ItemType itemInterpX_maxY_maxZ = m_vItems[TO_INDEX(minX, maxY, maxZ)] * (1.f - weightX) + m_vItems[TO_INDEX(maxX, maxY, maxZ)] * (weightX);

        ItemType itemInterpXY_minZ = itemInterpX_minY_minZ * (1.f - weightY) + itemInterpX_maxY_minZ * (weightY);
        ItemType itemInterpXY_maxZ = itemInterpX_minY_maxZ * (1.f - weightY) + itemInterpX_maxY_maxZ * (weightY);

        ItemType itemInterpXYZ = itemInterpXY_minZ * (1.f - weightZ) + itemInterpXY_maxZ * (weightZ);
        return itemInterpXYZ;
    }

    const ItemType &at(int x, int y, int z) const {
        int bx = BOUND(0, x, m_iNumX);
        int by = BOUND(0, y, m_iNumY);
        int bz = BOUND(0, z, m_iNumZ);
        return m_vItems[TO_INDEX(bx,by,bz)];
    }

    ItemType &at(int x, int y, int z) {
        int bx = BOUND(0, x, m_iNumX);
        int by = BOUND(0, y, m_iNumY);
        int bz = BOUND(0, z, m_iNumZ);
        return m_vItems[TO_INDEX(bx,by,bz)];
    }

    Point toPosition(int x, int y, int z) {
        Point myPos;
        if(m_pParent != NULL) {
            myPos = m_pParent->getPosition();
        }
        return Point(
            (x * m_bxBounds.w / m_iNumX) + (myPos.x + m_bxBounds.x),
            (y * m_bxBounds.h / m_iNumY) + (myPos.y + m_bxBounds.y),
            (z * m_bxBounds.l / m_iNumZ) + (myPos.z + m_bxBounds.z)
        );
    }

    int getSizeX() { return m_iNumX; }
    int getSizeY() { return m_iNumY; }
    int getSizeZ() { return m_iNumZ; }
    Box getBounds() {
        Point myPos;
        if(m_pParent != NULL) {
            myPos = m_pParent->getPosition();
        }
        return m_bxBounds + myPos;
    }
protected:
private:
    Point pointToIndexFloats(const Point &desiredPos) {
        Point myPos;
        if(m_pParent != NULL) {
            myPos = m_pParent->getPosition();
        }
        Point indexPos = Point(
            (desiredPos.x - (myPos.x + m_bxBounds.x)) * m_iNumX / m_bxBounds.w,
            (desiredPos.y - (myPos.y + m_bxBounds.y)) * m_iNumY / m_bxBounds.h,
            (desiredPos.z - (myPos.z + m_bxBounds.z)) * m_iNumZ / m_bxBounds.l
        );

        //Converting to actual integer indices can be done outside of here
        return indexPos;
    }

    int m_iNumX, m_iNumY, m_iNumZ;
    Box m_bxBounds;
    Positionable *m_pParent;
    std::vector<ItemType> m_vItems;
};

#endif // CELLGRID_H
