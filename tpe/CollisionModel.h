/*
 * CollisionModel.h
 * Defines the collision model interface and implementations
 */
#ifndef COLLISION_MODEL_H
#define COLLISION_MODEL_H

//convert a volume from px to meters
#define PX3_TO_M3(vol) (vol / 4096.f)   //16^3 = 4096, 16px to a meter

enum CollisionModelType {
    CM_BOX,
    CM_HEIGHTMAP,
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

    virtual float getVolume() { return PX3_TO_M3(m_bxBounds.w * m_bxBounds.h * m_bxBounds.l); }

    Box m_bxBounds;
};

#endif //COLLISION_MODEL_H
