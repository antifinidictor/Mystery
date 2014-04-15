/*
 * SimplePhysicsObject
 */

#include "SimplePhysicsObject.h"
#include "pwe/PartitionedWorldEngine.h"
#include "bae/BasicAudioEngine.h"

SimplePhysicsObject::SimplePhysicsObject(uint id, uint texId, Box bxVolume, float fDensity) {
    m_uiID = id;
    m_uiFlags = 0;

    Box bxRelativeVol =Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                            bxVolume.w,      bxVolume.h,      bxVolume.l);

    m_pPhysicsModel = new TimePhysicsModel(bxCenter(bxVolume), fDensity);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxRelativeVol));
    m_pPhysicsModel->setListener(this);

    Image *img = D3RE::get()->getImage(texId);
    m_pRenderModel = new D3PrismRenderModel(m_pPhysicsModel, bxRelativeVol);
    //Hidden faces not rendered
    m_pRenderModel->setTexture(NORTH, IMG_NONE);//img->m_uiID);
    m_pRenderModel->setTexture(SOUTH, img->m_uiID);
    m_pRenderModel->setTexture(EAST,  img->m_uiID);
    m_pRenderModel->setTexture(WEST,  img->m_uiID);
    m_pRenderModel->setTexture(UP,    img->m_uiID);
    m_pRenderModel->setTexture(DOWN,  IMG_NONE);//img->m_uiID);
    m_iSoundChannel = -1;
}

SimplePhysicsObject::~SimplePhysicsObject() {
    PWE::get()->freeId(getId());
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

GameObject*
SimplePhysicsObject::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint uiId = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    uint uiTexId = pt.get(keyBase + ".tex", 0);
    Box bxVolume;
    bxVolume.x = pt.get(keyBase + ".vol.x", 0.f);
    bxVolume.y = pt.get(keyBase + ".vol.y", 0.f);
    bxVolume.z = pt.get(keyBase + ".vol.z", 0.f);
    bxVolume.w = pt.get(keyBase + ".vol.w", 0.f);
    bxVolume.h = pt.get(keyBase + ".vol.h", 0.f);
    bxVolume.l = pt.get(keyBase + ".vol.l", 0.f);
    float fDensity = pt.get(keyBase + ".density", DENSITY_WOOD);
    SimplePhysicsObject *obj = new SimplePhysicsObject(uiId, uiTexId, bxVolume, fDensity);
    Color cr;
    cr.r = pt.get(keyBase + ".cr.r", 0xFF);
    cr.g = pt.get(keyBase + ".cr.g", 0xFF);
    cr.b = pt.get(keyBase + ".cr.b", 0xFF);
    obj->m_pRenderModel->setColor(cr);
    return obj;
}

void
SimplePhysicsObject::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    pt.put(keyBase + ".id", getId());
    pt.put(keyBase + ".tex", m_pRenderModel->getTexture(SOUTH));
    Box bxVolume = m_pPhysicsModel->getCollisionVolume();
    pt.put(keyBase + ".vol.x", bxVolume.x);
    pt.put(keyBase + ".vol.y", bxVolume.y);
    pt.put(keyBase + ".vol.z", bxVolume.z);
    pt.put(keyBase + ".vol.w", bxVolume.w);
    pt.put(keyBase + ".vol.h", bxVolume.h);
    pt.put(keyBase + ".vol.l", bxVolume.l);
    Color cr = m_pRenderModel->getColor();
    pt.put(keyBase + ".cr.r", cr.r);
    pt.put(keyBase + ".cr.g", cr.g);
    pt.put(keyBase + ".cr.b", cr.b);
    pt.put(keyBase + ".density", m_pPhysicsModel->getDensity());
}


int
SimplePhysicsObject::callBack(uint uiId, void *data, uint uiEventId) {
    int status = EVENT_CAUGHT;
    switch(uiEventId) {
    case TPE_ON_COLLISION:
        /*
        if(!m_bPlayingSound) {
            HandleCollisionData *d = (HandleCollisionData*)data;
            if(d->iDirection == UP || d->iDirection == DOWN) return EVENT_DROPPED;
            m_bPlayingSound = true;
            BAE::get()->playSound(AUD_DRAG);
        }
        */
        break;
    default:
        status = EVENT_DROPPED;
        break;
    }
    return status;
}

bool
SimplePhysicsObject::update(float fDeltaTime) {
    //Useful place to put test code
    #define MIN_SHIFT_FOR_SOUND 0.01f
    if(m_iSoundChannel >= 0 &&
           (m_pPhysicsModel->getSurface() == NULL ||
            m_pPhysicsModel->getLastVelocity().magnitude() <= MIN_SHIFT_FOR_SOUND)) {
        BAE::get()->playSound(AUD_NONE, 0, m_iSoundChannel);
        m_iSoundChannel = -1;

    } else if(m_iSoundChannel < 0 &&
              m_pPhysicsModel->getSurface() != NULL &&
              m_pPhysicsModel->getLastVelocity().magnitude() > MIN_SHIFT_FOR_SOUND) {
        m_iSoundChannel = BAE::get()->playSound(AUD_DRAG, -1, -1);
    }
    return false;
}
