#ifndef WALL_H
#define WALL_H

#include "mge/GameObject.h"
#include "d3re/d3re.h"
#include "tpe/tpe.h"
#include "game/game_defs.h"

enum WallVisibleFaces {
    WALL_NONE  =  0x0,
    WALL_NORTH =  0x1,
    WALL_SOUTH =  0x2,
    WALL_EAST  =  0x4,
    WALL_WEST  =  0x8,
    WALL_UP    = 0x10,
    WALL_DOWN  = 0x20,
    WALL_DEFAULT = 0x1E,
    WALL_ALL   = 0x3F
};

class Wall : public GameObject
{
public:
    Wall(uint uiId, uint texTopId, uint texBottomId, uint texSideId, Box bxVolume, uint visibleFaces = WALL_DEFAULT);
    virtual ~Wall();

    //General
    virtual uint getID() { return m_uiID; }
    virtual bool update(uint time)              { return false; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType() { return TYPE_GENERAL; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Misc
    void setColor(const Color &cr) { m_pRenderModel->setColor(cr); }
    Color &getColor() { return m_pRenderModel->getColor(); }

private:
    uint m_uiID;
    uint m_uiFlags;

    D3PrismRenderModel *m_pRenderModel;
    TimePhysicsModel   *m_pPhysicsModel;
};

#endif // WALL_H
