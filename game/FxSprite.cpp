#include "FxSprite.h"
#define MAX_ANIM_TIMER 10
FxSprite::FxSprite(uint id, uint texId, int duration, const Point &ptPos, uint frameW) {
    m_uiId = id;
    m_uiFlags = 0;
    float size = 0.25f;
    Rect rcRelativeArea = Rect(-size / 2, -size / 2, size, size);
    m_pPhysicsModel = new NullTimePhysicsModel(ptPos);
    m_pRenderModel = new D3XZSpriteRenderModel(this, texId, rcRelativeArea);
    setFlag(TPE_STATIC, true);
    setFlag(TPE_PASSABLE, true);

    Image *img = D3RE::get()->getImage(texId);
    if(img != NULL) {
        m_uiMaxFramesH = img->m_iNumFramesH;
    } else {
        m_uiMaxFramesH = 0;
    }
    m_pRenderModel->setFrameW(frameW);
    m_iMaxTimeToLive = m_iTimeToLive = duration;
}

FxSprite::~FxSprite() {
    delete m_pPhysicsModel;
    delete m_pRenderModel;
}

bool
FxSprite::update(uint time) {
    //Insert animation stuff here
    uint curFrame = (m_iMaxTimeToLive - m_iTimeToLive) * m_uiMaxFramesH / m_iMaxTimeToLive;
    m_pRenderModel->setFrameH(curFrame);
    return m_iTimeToLive-- < 0;
}

int
FxSprite::callBack(uint cID, void *data, uint uiEventId) {
    return EVENT_DROPPED;
}
