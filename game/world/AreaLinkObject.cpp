/*
 * AreaLinkObject
 */

#include "AreaLinkObject.h"
#include "tpe/tpe.h"
#include "pwe/PartitionedWorldEngine.h"

AreaLinkObject::AreaLinkObject(uint id, uint uiDestAreaId, const Point &ptDestPos, const Box &bxTriggerVolume) {
    m_uiID = id;
    m_uiFlags = 0;
    Box bxRelativeVol =Box(-bxTriggerVolume.w / 2, -bxTriggerVolume.h / 2, -bxTriggerVolume.l / 2,
                            bxTriggerVolume.w,      bxTriggerVolume.h,      bxTriggerVolume.l);
    m_pRenderModel = new D3PrismRenderModel(this, bxRelativeVol);
    //Prism render model because in the editor, it will look like a volume
    m_pRenderModel->setTexture(NORTH, IMG_NONE);
    m_pRenderModel->setTexture(SOUTH, IMG_NONE);
    m_pRenderModel->setTexture(EAST,  IMG_NONE);
    m_pRenderModel->setTexture(WEST,  IMG_NONE);
    m_pRenderModel->setTexture(UP,    IMG_NONE);
    m_pRenderModel->setTexture(DOWN,  IMG_NONE);

    m_pPhysicsModel = new TimePhysicsModel(bxCenter(bxTriggerVolume));
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxRelativeVol));
    m_pPhysicsModel->setListener(this);

    m_uiDestAreaId = uiDestAreaId;
    m_ptDestPos = ptDestPos;

    setFlag(TPE_STATIC, true);
    setFlag(TPE_PASSABLE, true);
    setFlag(D3RE_INVISIBLE, true);
    m_uiDirections = BIT(NUM_DIRECTIONS) - 1;
}

AreaLinkObject::~AreaLinkObject() {
    PWE::get()->freeId(getId());
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

GameObject*
AreaLinkObject::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint uiId = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    uint uiAreaId = pt.get(keyBase + ".destAreaId", 0);
    Box bxVolume;
    bxVolume.x = pt.get(keyBase + ".vol.x", 0.f);
    bxVolume.y = pt.get(keyBase + ".vol.y", 0.f);
    bxVolume.z = pt.get(keyBase + ".vol.z", 0.f);
    bxVolume.w = pt.get(keyBase + ".vol.w", 0.f);
    bxVolume.h = pt.get(keyBase + ".vol.h", 0.f);
    bxVolume.l = pt.get(keyBase + ".vol.l", 0.f);
    Point ptDestPos;
    ptDestPos.x = pt.get(keyBase + ".dest.x", 0.f);
    ptDestPos.y = pt.get(keyBase + ".dest.y", 0.f);
    ptDestPos.z = pt.get(keyBase + ".dest.z", 0.f);
    AreaLinkObject *obj = new AreaLinkObject(uiId, uiAreaId, ptDestPos, bxVolume);

    obj->m_uiDirections = pt.get(keyBase + ".dirs", BIT(NUM_DIRECTIONS) - 1);
    return obj;
}

void
AreaLinkObject::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    pt.put(keyBase + ".id", getId());
    pt.put(keyBase + ".destAreaId", m_uiDestAreaId);
    Box bxVolume = m_pPhysicsModel->getCollisionVolume();
    pt.put(keyBase + ".vol.x", bxVolume.x);
    pt.put(keyBase + ".vol.y", bxVolume.y);
    pt.put(keyBase + ".vol.z", bxVolume.z);
    pt.put(keyBase + ".vol.w", bxVolume.w);
    pt.put(keyBase + ".vol.h", bxVolume.h);
    pt.put(keyBase + ".vol.l", bxVolume.l);

    pt.put(keyBase + ".dest.x", m_ptDestPos.x);
    pt.put(keyBase + ".dest.y", m_ptDestPos.y);
    pt.put(keyBase + ".dest.z", m_ptDestPos.z);

    pt.put(keyBase + ".dirs", m_uiDirections);
}


int
AreaLinkObject::callBack(uint uiID, void *data, uint eventId) {
    int status = EVENT_CAUGHT;
    switch(eventId) {
    case TPE_ON_COLLISION: {
        HandleCollisionData *hcd = (HandleCollisionData*)data;
        PWE *we = PWE::get();

        if(hcd->obj->getFlag(GAM_CAN_LINK)/*&& (hcd->iDirection & m_uiDirections)*/) {
            Point ptPosDelta = m_ptDestPos - hcd->obj->getPhysicsModel()->getPosition();
            hcd->obj->moveBy(ptPosDelta);
            we->moveObjectToArea(hcd->obj->getId(), we->getCurrentArea(), m_uiDestAreaId);
        }
        break;
      }
    default:
        status = EVENT_DROPPED;
        break;
    }
    return status;
}
