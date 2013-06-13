/*
 * LiftingSurface.cpp
 * A magically-clickable-and-raisable surface
 */
#include "LiftingSurface.h"
#include "tpe/TimePhysicsEngine.h"
using namespace std;

LiftingSurface::LiftingSurface(uint id, Image *img, Box bxVolume) {
    m_uiID = id;
    m_uiFlags = 0;
    Rect rcDrawArea = Rect(bxVolume.x, bxVolume.y + bxVolume.l - img->h / img->m_iNumFramesH,
                           img->w / img->m_iNumFramesW, img->h / img->m_iNumFramesH);
    //m_pRenderModel = new EdgeRenderModel(img, rcDrawArea, bxVolume.z, ORE_LAYER_OBJECTS);
    m_pPhysicsModel = new TimePhysicsModel(bxVolume);
    this->setFlag(TPE_STATIC, true);
    m_iCount = 0;
}

bool LiftingSurface::LiftingSurface::update(uint time) {
    return false;
}


void LiftingSurface::LiftingSurface::callBack(uint cID, void *data, EventID id) {
}

bool LiftingSurface::hasNext() {
    return m_iCount > 0;
}

Box LiftingSurface::nextVolume() {
    m_itrCurList++;
    m_iCount--;
    return (*m_itrCurList)->getVolume();
}

void LiftingSurface::setList(int iDir) {
    switch(iDir) {
    case NORTH:
        m_itrCurList = m_vNorth.begin();
        m_iCount = m_vNorth.size() - 1;
        break;
    case EAST:
        m_itrCurList = m_vEast.begin();
        m_iCount = m_vEast.size() - 1;
        break;
    case SOUTH:
        m_itrCurList = m_vSouth.begin();
        m_iCount = m_vSouth.size() - 1;
        break;
    default: //WEST
        m_itrCurList = m_vWest.begin();
        m_iCount = m_vWest.size() - 1;
        break;
    }
}
