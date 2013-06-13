/*
 * ClickableItem.cpp
 * Defines the ClickableItem class
 */

#include "ClickableItem.h"
#include "game/gameDefs.h"
#include "ore/OrderedRenderEngine.h"
#include "pwe/PartitionedWorldEngine.h"

using namespace std;


ClickableItem::ClickableItem(uint uiID, Image *img, Point pos, bool bFreeListener) :
        Clickable(uiID, bFreeListener) {
    Box bxArea = Box(pos.x, pos.y, pos.z, img->w / img->m_iNumFramesW, img->h / img->m_iNumFramesH, 1);
    m_pRenderModel = new OrderedRenderModel(img, bxArea, pos.z, ORE_LAYER_HIGH_FX);
    m_pPhysicsModel = new TimePhysicsModel(bxArea);
}

ClickableItem::~ClickableItem() {
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

bool ClickableItem::update(uint time) {
    return false;
}
