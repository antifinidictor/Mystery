/*
 * Wall.cpp
 * Defines functions for the wall class.
 */

#include "Wall.h"
#include <stdio.h>
#include "tpe/TimePhysicsEngine.h"
#include "pwe/PartitionedWorldEngine.h"
#include "ore/OrderedRenderEngine.h"

Wall::Wall(uint uiID, Image *pStraightImage, Image *pCornerImage, Box bxArea, int iDirection) {
    m_uiID = uiID;
    m_iDirection = iDirection;
    m_uiFlags = 0;

    int iWidth  = pStraightImage->w,
        iHeight = pStraightImage->h,
        iLength = 0;

    Box bxMainArea, bxNextEdge;
    switch(iDirection) {
    case WALL_NORTH:
        iLength = bxArea.w;
        bxMainArea = Box(bxArea.x, bxArea.y, bxArea.z, bxArea.w - iWidth, bxArea.l, bxArea.h);
        bxNextEdge = Box(bxArea.x + iLength - iWidth, bxArea.y, bxArea.z, iWidth, iHeight, bxArea.h);
        break;
    case WALL_EAST:
        iLength = bxArea.l;
        bxMainArea = Box(bxArea.x, bxArea.y, bxArea.z, bxArea.w, bxArea.l - iWidth, bxArea.h);
        bxNextEdge = Box(bxArea.x, bxArea.y + iLength - iWidth, bxArea.z, iWidth, iHeight, bxArea.h);
        break;
    case WALL_SOUTH:
        iLength = bxArea.w;
        bxMainArea = Box(bxArea.x + iWidth, bxArea.y, bxArea.z, bxArea.w - iWidth, bxArea.l, bxArea.h);
        bxNextEdge = Box(bxArea.x, bxArea.y, bxArea.z, iWidth, iHeight, bxArea.h);
        break;
    case WALL_WEST:
        iLength = bxArea.l;
        bxMainArea = Box(bxArea.x, bxArea.y + iWidth, bxArea.z, bxArea.w, bxArea.l - iWidth, bxArea.h);
        bxNextEdge = Box(bxArea.x, bxArea.y, bxArea.z, iWidth, iHeight, bxArea.h);
        break;
    }

    //Initialize render models
    m_pRenderModel   = new CompositeRenderModel(bxArea);
    m_pNextEdgeModel = new OrderedRenderModel(pCornerImage, bxNextEdge, bxNextEdge.z, ORE_LAYER_OBJECTS);
    m_pPrimaryModel = new WallRenderModel(pStraightImage, bxMainArea, iLength - iWidth, iDirection);
    m_pRenderModel->add(m_pPrimaryModel);
    m_pRenderModel->add(m_pNextEdgeModel);

    //Initialize physics model
    m_pPhysicsModel = new TimePhysicsModel(bxArea);

    //Initialize corner lookup table:
    m_luCorner[WALL_NORTH][WALL_NORTH] = WALL_NORTH;
    m_luCorner[WALL_NORTH][WALL_EAST]  = WALL_OUT_NE;
    m_luCorner[WALL_NORTH][WALL_SOUTH] = WALL_IN_NW;
    m_luCorner[WALL_NORTH][WALL_WEST]  = WALL_IN_NW;
    m_luCorner[WALL_EAST][WALL_NORTH]  = WALL_IN_NE;
    m_luCorner[WALL_EAST][WALL_EAST]   = WALL_EAST;
    m_luCorner[WALL_EAST][WALL_SOUTH]  = WALL_OUT_SE;
    m_luCorner[WALL_EAST][WALL_WEST]   = WALL_IN_NE;
    m_luCorner[WALL_SOUTH][WALL_NORTH] = WALL_IN_SE;
    m_luCorner[WALL_SOUTH][WALL_EAST]  = WALL_IN_SE;
    m_luCorner[WALL_SOUTH][WALL_SOUTH] = WALL_SOUTH;
    m_luCorner[WALL_SOUTH][WALL_WEST]  = WALL_OUT_SW;
    m_luCorner[WALL_WEST][WALL_NORTH]  = WALL_OUT_NW;
    m_luCorner[WALL_WEST][WALL_EAST]   = WALL_IN_SW;
    m_luCorner[WALL_WEST][WALL_SOUTH]  = WALL_IN_SW;
    m_luCorner[WALL_WEST][WALL_WEST]   = WALL_WEST;

    //Initialize NULL walls: iDirection corresponds directly to the frame index
    m_pNextEdgeModel->setFrameW(iDirection & 1);
    m_pNextEdgeModel->setFrameH(iDirection >> 2);

    setFlag(TPE_STATIC, true);
}

Wall::~Wall() {
}

void Wall::setNextWall(Wall *pNext) {
    m_pNext = pNext;

    //Set the corner direction
    if(pNext != NULL) {
        setDirection(pNext->getDirection());
    }
}

void Wall::setDirection(int iNextDirection) {
    int iDirection = m_luCorner[m_iDirection][iNextDirection];
//    printf("id: %d my dir: %d his dir: %d res: %d (%d,%d)\n", m_uiID, m_iDirection, iNextDirection, iDirection, iDirection & 1, iDirection >> 1);
    m_pNextEdgeModel->setFrameW(iDirection & 1);
    m_pNextEdgeModel->setFrameH(iDirection >> 1);
}


Wall *Wall::read(FileManager *mgr) {
    PartitionedWorldEngine *we = PWE::get();
    fstream *fin = mgr->getFileHandle();
    uint uiImgStraightID, uiImgCornerID;
    Box bxArea;
    int iDirection;

    fin->read((char*)(&uiImgStraightID), sizeof(uint));
    fin->read((char*)(&uiImgCornerID),   sizeof(uint));
    fin->read((char*)(&bxArea),          sizeof(Box));
    fin->read((char*)(&iDirection),      sizeof(int));

    return new Wall(we->genID(), mgr->getImageRes(uiImgStraightID), mgr->getImageRes(uiImgCornerID), bxArea, iDirection);
}

void Wall::write(FileManager *mgr) {
    fstream *fout = mgr->getFileHandle();
    uint uiImgStraightID = m_pPrimaryModel->getImage()->m_uiID,
         uiImgCornerID = m_pNextEdgeModel->getImage()->m_uiID;
    Box bxArea = m_pPhysicsModel->getCollisionVolume();
    ObjType eType = OT_WALL;

    fout->write((char*)(&eType),           sizeof(ObjType));
    fout->write((char*)(&uiImgStraightID), sizeof(uint));
    fout->write((char*)(&uiImgCornerID),   sizeof(uint));
    fout->write((char*)(&bxArea),          sizeof(Box));
    fout->write((char*)(&m_iDirection),    sizeof(int));
}
