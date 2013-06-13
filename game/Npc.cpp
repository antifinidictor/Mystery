/*
 * Npc.cpp
 * First try at an Npc class
 */

#include "Npc.h"
#include "game/gameDefs.h"
#include "ore/OrderedRenderEngine.h"
#include "tpe/TimePhysicsEngine.h"
#include "pwe/PartitionedWorldEngine.h"

#define NPC_SPEED 0.5f
#define TIMER_MAX 200

Npc::Npc(uint uiID, Image *pImage, Point pos) {
    m_uiID = uiID;
    m_uiFlags = 0;
    int iw = pImage->w / pImage->m_iNumFramesW,
        ih = pImage->h / pImage->m_iNumFramesH;
    Rect rcRenderArea(pos.x - iw / 2, pos.y - 3 * ih / 4, iw, ih);
    Box bxCollisionArea(pos.x - 3 * iw / 8, pos.y - ih / 4, pos.z, 3 * iw / 4, ih / 2, ih);
    m_pPhysicsModel = new TimePhysicsModel(bxCollisionArea);
    m_pRenderModel = new OrderedRenderModel(pImage, rcRenderArea, pos.z, ORE_LAYER_OBJECTS);
    m_iTimer = TIMER_MAX;
    m_iFrame = 0;
    m_iDirection = SOUTH;
    m_eState = NPC_WALK;
    m_uiLastUpdated = 0;

    m_pRenderModel->setFrameW(m_iDirection);
    setFlag(GAM_SPELLABLE, true);
    printf("NPC given id %d\n", uiID);
}

Npc::~Npc() {
}


bool Npc::update(uint time) {
    m_uiLastUpdated = time;

    --m_iTimer;
    Point p;
    float fx = 0, fy = 0;
    switch(m_eState) {
    case NPC_WALK:
        if((m_iTimer % 20) == 0) {
            m_iFrame = (m_iFrame + 1) % 4;
            m_pRenderModel->setFrameH(m_iFrame + 1);
        }
        switch(m_iDirection) {
        case NORTH:
            fy = -NPC_SPEED;
            break;
        case SOUTH:
            fy = NPC_SPEED;
            break;
        case EAST:
            fx = NPC_SPEED;
            break;
        case WEST:
            fx = -NPC_SPEED;
            break;
        }
        p = m_pRenderModel->getPosition();
        //printf(__FILE__": %d p = (%2.2f, %2.2f, %2.2f); f = (%2.2f,%2.2f)\n", __LINE__, p.x, p.y, p.z,fx,fy);
        if(!getFlag(TPE_FLOATING)) {
            m_pPhysicsModel->applyForce(Point(fx, fy, 0));
        }
        if(m_iTimer <= 0) {
            m_iTimer += TIMER_MAX;
            m_iDirection = (m_iDirection + 1) % 4;
            m_pRenderModel->setFrameW(m_iDirection);
        }
        break;
    case NPC_STAND:
        m_iFrame = 0;
        m_pRenderModel->setFrameH(m_iFrame);
        break;
    case NPC_TURN:
        break;
    default:
        break;
    }
    return false;
}

void Npc::callBack(uint cID, void *data, EventID id) {
}
