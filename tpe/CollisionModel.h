/*
 * CollisionModel.h
 * Defines the collision model interface and implementations
 */
#ifndef COLLISION_MODEL_H
#define COLLISION_MODEL_H

#include "mge/PixelMap.h"
#include "tpe/fluids/mgeMath.h"
#include "tpe/fluids/InterpGrid.h"

enum CollisionModelType {
    CM_BOX,
    CM_Y_HEIGHTMAP,
    CM_VORTON,
    CM_NUM_COLLISION_MODEL_TYPES
};

class CollisionModel {
public:
    virtual ~CollisionModel() {}
    virtual Box getBounds() = 0;    //Returns a volume that bounds this object
    virtual CollisionModelType getType() = 0;
    virtual float getVolume() = 0;
};

class BoxCollisionModel : public CollisionModel {
public:
    BoxCollisionModel(Box bxBounds) {
        m_bxBounds = bxBounds;
    }
    virtual ~BoxCollisionModel() {}
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
    virtual ~PixelMapCollisionModel() {}
    virtual Box getBounds() { return m_bxBounds; }
    virtual CollisionModelType getType() { return CM_Y_HEIGHTMAP; }

    virtual float getVolume();

    float getHeightAtPoint(const Point &ptPos);
    Vec3f getNormalAtPoint(const Point &ptPos);

    const PixelMap *m_pxMap;
    Box m_bxBounds;
};

class VortonCollisionModel : public CollisionModel {
public:
    VortonCollisionModel(Positionable *pParent, const Vec3f &v3InitVorticity, float fRadius);
    virtual ~VortonCollisionModel() {}
    virtual Box getBounds() { return Box(); }
    virtual CollisionModelType getType() { return CM_VORTON; }
    virtual float getVolume() { return 0.f; }

    void update(float fTimeQuantum);
    Vec3f velocityAt(const Point &pos);
    void exchangeVorticityWith(float fViscocity, VortonCollisionModel *v);

    //Vorton CMs are responsible for knowing which fluid they are a part of
    InterpGrid< Matrix<3,3> > *m_pJacobianGrid;
    InterpGrid< Vec3f >       *m_pVelocityGrid;

    Positionable *m_pParent;
    Vec3f m_v3Vorticity;
    float m_fRadius;    //Radius after which linear scaling occurs
};

#endif //COLLISION_MODEL_H
