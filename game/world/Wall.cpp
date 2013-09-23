#include "Wall.h"

Wall::Wall(uint uiId, uint texTopId, uint texBottomId, uint texSideId, Box bxVolume, uint visibleFaces) {
    m_uiID = uiId;
    m_uiFlags = 0;

    m_pRenderModel = new D3PrismRenderModel(this, Box(-bxVolume.w / 2, -bxVolume.l / 2, -bxVolume.h / 2,
                                                       bxVolume.w,      bxVolume.l,      bxVolume.h));

    m_pRenderModel->setTexture(NORTH, ((visibleFaces & WALL_NORTH) ? texSideId : IMG_NONE));
    m_pRenderModel->setTexture(SOUTH, ((visibleFaces & WALL_SOUTH) ? texSideId : IMG_NONE));
    m_pRenderModel->setTexture(EAST,  ((visibleFaces & WALL_EAST)  ? texSideId : IMG_NONE));
    m_pRenderModel->setTexture(WEST,  ((visibleFaces & WALL_WEST)  ? texSideId : IMG_NONE));
    m_pRenderModel->setTexture(UP,    ((visibleFaces & WALL_UP)    ? texTopId  : IMG_NONE));
    m_pRenderModel->setTexture(DOWN,  ((visibleFaces & WALL_DOWN)  ? texBottomId : IMG_NONE));

    m_pPhysicsModel = new TimePhysicsModel(bxVolume);

    setFlag(TPE_STATIC, true);
}

Wall::~Wall() {
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}
