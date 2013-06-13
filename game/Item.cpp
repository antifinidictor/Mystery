/*
 * Item.cpp
 * Specifies the item class.
 */
#include "Item.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderEngine.h"

ItemObject::ItemObject(uint uiID, Image *pImage, Point pos) {
    m_uiID = uiID;
    m_uiFlags = 0;
    m_bDead = false;
    int iw = pImage->w / pImage->m_iNumFramesW,
        ih = pImage->h / pImage->m_iNumFramesH;
    Box bxVolume(pos.x - iw / 2, pos.y - ih / 2, pos.z, iw, ih, 3);
    m_pPhysicsModel = new TimePhysicsModel(bxVolume);
    m_pRenderModel = new OrderedRenderModel(pImage, bxVolume, pos.z, ORE_LAYER_LOW_FX);
}

ItemObject::~ItemObject() {
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

bool ItemObject::update(uint time) {
    return m_bDead;
}


void ItemObject::callBack(uint cID, void *data, EventID id) {
}
