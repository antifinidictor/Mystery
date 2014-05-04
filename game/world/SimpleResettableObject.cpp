/*
 * SimpleResettableObject
 */

#include "SimpleResettableObject.h"
#include "pwe/PartitionedWorldEngine.h"
#include "bae/BasicAudioEngine.h"

SimpleResettableObject::SimpleResettableObject(uint id, uint texId, Box bxVolume, float fDensity) {
    m_uiID = id;
    m_uiFlags = 0;

    Box bxRelativeVol =Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                            bxVolume.w,      bxVolume.h,      bxVolume.l);

    m_pPhysicsModel = new TimePhysicsModel(this, bxCenter(bxVolume), fDensity);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxRelativeVol));

    Image *img = D3RE::get()->getImage(texId);
    m_pRenderModel = new D3PrismRenderModel(m_pPhysicsModel, bxRelativeVol);
    //Hidden faces not rendered
    m_pRenderModel->setTexture(NORTH, img->m_uiID);
    m_pRenderModel->setTexture(SOUTH, img->m_uiID);
    m_pRenderModel->setTexture(EAST,  img->m_uiID);
    m_pRenderModel->setTexture(WEST,  img->m_uiID);
    m_pRenderModel->setTexture(UP,    img->m_uiID);
    m_pRenderModel->setTexture(DOWN,  IMG_NONE);//img->m_uiID);

    m_ptOriginalPosition = m_pPhysicsModel->getPosition();
}

SimpleResettableObject::~SimpleResettableObject() {
    PWE::get()->freeId(getId());
    PWE::get()->removeListener(getId(), PWE_ON_AREA_SWITCH_TO, m_uiArea);
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

GameObject*
SimpleResettableObject::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
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
    SimpleResettableObject *obj = new SimpleResettableObject(uiId, uiTexId, bxVolume, fDensity);
    Color cr;
    cr.r = pt.get(keyBase + ".cr.r", 0xFF);
    cr.g = pt.get(keyBase + ".cr.g", 0xFF);
    cr.b = pt.get(keyBase + ".cr.b", 0xFF);
    obj->m_pRenderModel->setColor(cr);
    return obj;
}

void
SimpleResettableObject::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
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
SimpleResettableObject::callBack(uint uiId, void *data, uint uiEventId) {
    int status = EVENT_CAUGHT;
    switch(uiEventId) {
    case PWE_ON_ADDED_TO_AREA:
        m_uiArea = *(uint*)data;
        PWE::get()->addListener(this, PWE_ON_AREA_SWITCH_TO, m_uiArea);
        break;
    case PWE_ON_AREA_SWITCH_TO:
        reset();
        break;
    default:
        status = EVENT_DROPPED;
        break;
    }
    return status;
}


void
SimpleResettableObject::reset() {
    moveBy(m_ptOriginalPosition - m_pPhysicsModel->getPosition());
}


bool
SimpleResettableObject::update(float fDeltaTime) {
    #define MIN_SHIFT_FOR_SOUND 0.001f
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

