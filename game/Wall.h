/*
 * Wall.h
 * Defines a surface with friction
 */

#ifndef WALL_H
#define WALL_H
#include "mge/GameObject.h"
#include "mge/defs.h"
#include "mge/Image.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderModel.h"
//#include "bre/BasicRenderModel.h"
#include "CompositeRenderModel.h"
#include "WallRenderModel.h"
#include "game/FileManager.h"
#include "game/GameDefs.h"

#define WALL_NORTH  0
#define WALL_EAST   1
#define WALL_SOUTH  2
#define WALL_WEST   3

#define WALL_OUT_NW 4
#define WALL_OUT_NE 5
#define WALL_OUT_SW 6
#define WALL_OUT_SE 7
#define WALL_IN_SE  8
#define WALL_IN_SW  9
#define WALL_IN_NE  10
#define WALL_IN_NW  11

class Wall : public GameObject {
public:
    Wall(uint uiID, Image *pStraightImage, Image *pCornerImage, Box bxArea, int iDirection);
    virtual ~Wall();

    //General
    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual int  getDirection()                 { return m_iDirection; }
    virtual uint getType() { return OBJ_WALL; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }
    virtual bool update(uint time) { return false; } //Nothing to update

    //Altering the wall
    void setNextWall(Wall *pNext);

    //File I/O
    static Wall *read(FileManager *mgr);
    void write(FileManager *mgr);


private:
    Wall *m_pPrev,  //N/S: Left  E/W: Top
         *m_pNext;  //N/S: Right E/W: Bottom
    CompositeRenderModel *m_pRenderModel;
    OrderedRenderModel   *m_pNextEdgeModel;
    WallRenderModel      *m_pPrimaryModel;
    TimePhysicsModel    *m_pPhysicsModel;

    int m_iDirection;
    uint m_uiID, m_uiFlags;
    char m_luCorner[4][4];

    void setDirection(int iNextDirection);
};

#endif
