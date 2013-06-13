/*
 * FXSprite.cpp
 * Contains definitions for the FXSprite class
 */

#include "FXSprite.h"
#include "tpe/TimePhysicsEngine.h"
#include "ore/OrderedRenderEngine.h"
#include "game/gameDefs.h"

#define TIME_START 3

FXSprite::FXSprite(uint id, Point pos, FXType type, int iLayer) {
    m_uiID = id;
    Image *imgFX = ORE::get()->getMappedImage(IMG_FX);
    int iWidth = imgFX->w / imgFX->m_iNumFramesW,
        iLength = imgFX->h / imgFX->m_iNumFramesH;
    Box bxArea = Box(pos.x - iWidth / 2, pos.y - iLength / 2, pos.z, iWidth, iLength, 1);
    m_pPhysicsModel = new TimePhysicsModel(bxArea);
    m_pRenderModel = new OrderedRenderModel(imgFX, bxArea, pos.z, iLayer);
    m_pRenderModel->setFrameW(type);
    m_uiFlags = 0;
    setFlag(TPE_STATIC, true);
    setFlag(TPE_PASSABLE, true);
    m_iCurFrame = 0;
    m_iTimer = TIME_START;
    m_iMaxTime = TIME_START;
}

FXSprite::~FXSprite() {
    delete m_pPhysicsModel;
    delete m_pRenderModel;
}

bool FXSprite::update(uint time) {
    if(m_iCurFrame > 4) {
        return true;
    }

    if(m_iTimer < 0) {
        m_iTimer = m_iMaxTime;
        m_pRenderModel->setFrameH(++m_iCurFrame);
    } else {
        --m_iTimer;
    }
    return false;
}
