/*
 * CollisionModel.h
 * Defines the collision model interface and implementations
 */
#ifndef COLLISION_MODEL_H
#define COLLISION_MODEL_H

#include "mge/PixelMap.h"
//convert a volume from px to meters

enum CollisionModelType {
    CM_BOX,
    CM_Y_HEIGHTMAP,
    CM_NUM_COLLISION_MODEL_TYPES
};

class CollisionModel {
public:
    virtual Box getBounds() = 0;    //Returns a volume that bounds this object
    virtual CollisionModelType getType() = 0;
    virtual float getVolume() = 0;
};

class BoxCollisionModel : public CollisionModel {
public:
    BoxCollisionModel(Box bxBounds) {
        m_bxBounds = bxBounds;
    }
    virtual Box getBounds() { return m_bxBounds; }
    virtual CollisionModelType getType() { return CM_BOX; }

    virtual float getVolume() { return (m_bxBounds.w * m_bxBounds.h * m_bxBounds.l); }

    Box m_bxBounds;
};

class PixelMapCollisionModel : public CollisionModel {
public:
    PixelMapCollisionModel(const Box &bxBounds, const PixelMap *pxMap)
        : m_pxMap(pxMap),
          m_bxBounds(bxBounds)
    {
    }
    virtual Box getBounds() { return m_bxBounds; }
    virtual CollisionModelType getType() { return CM_Y_HEIGHTMAP; }

    virtual float getVolume() { return (m_bxBounds.w * m_bxBounds.h * m_bxBounds.l); }

    float getHeightAtPoint(const Point &ptPos) {
        //Scale ptPos to a set of four indices
        float x = (ptPos.x - m_bxBounds.x) * (m_pxMap->m_uiW - 1) / m_bxBounds.w;
        float z = (ptPos.z - m_bxBounds.z) * (m_pxMap->m_uiH - 1) / m_bxBounds.l;
        float fx = floor(x);
        float fz = floor(z);
        float cx = ceil(x);
        float cz = ceil(z);

        if(fx < 0 || cz > m_pxMap->m_uiW || fz < 0 || cz > m_pxMap->m_uiH) {
            return 0.f;
        }

        //Get heights at four neighboring indices
        float yff = (float)m_pxMap->m_pData[(int)fx][(int)fz].toUint();
        float yfc = (float)m_pxMap->m_pData[(int)fx][(int)cz].toUint();
        float ycc = (float)m_pxMap->m_pData[(int)cx][(int)cz].toUint();
        float ycf = (float)m_pxMap->m_pData[(int)cx][(int)fz].toUint();

        //Interpolate
        float xDiff = (cx - x) / (cx - fx);
        float xInterpFz = (xDiff) * yff + (1.f - xDiff) * ycf;
        float xInterpCz = (xDiff) * yfc + (1.f - xDiff) * ycc;
        float zDiff = (cz - z) / (cz - fz);
        float interp = zDiff * xInterpFz + (1 - zDiff) * xInterpCz;
        interp = interp / MAX_COLOR_VAL * (m_bxBounds.h) + m_bxBounds.y;
        return interp;
    }

    const PixelMap *m_pxMap;
    Box m_bxBounds;
};

#endif //COLLISION_MODEL_H
