/*
 * AreaLinkObject
 */

#include "AreaLinkObject.h"
#include "tpe/tpe.h"
#include "pwe/PartitionedWorldEngine.h"
#include "mge/Clock.h"
using namespace std;

AreaLinkObject::AreaLinkObject(uint id, uint uiDestAreaId, const Point &ptDestPos, const Box &bxTriggerVolume) {
    m_uiID = id;
    m_uiFlags = 0;
    Box bxRelativeVol =Box(-bxTriggerVolume.w / 2, -bxTriggerVolume.h / 2, -bxTriggerVolume.l / 2,
                            bxTriggerVolume.w,      bxTriggerVolume.h,      bxTriggerVolume.l);

    m_pPhysicsModel = new TimePhysicsModel(this, bxCenter(bxTriggerVolume));
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxRelativeVol));
    m_pPhysicsModel->setListener(this);

    m_pRenderModel = new D3PrismRenderModel(m_pPhysicsModel, bxRelativeVol);
    //Prism render model because in the editor, it will look like a volume
    m_pRenderModel->setTexture(NORTH, IMG_NONE);
    m_pRenderModel->setTexture(SOUTH, IMG_NONE);
    m_pRenderModel->setTexture(EAST,  IMG_NONE);
    m_pRenderModel->setTexture(WEST,  IMG_NONE);
    m_pRenderModel->setTexture(UP,    IMG_NONE);
    m_pRenderModel->setTexture(DOWN,  IMG_NONE);

    m_uiSrcAreaId = 0;  //To be filled out later
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
        //printf("Colliding with obj %d (I am %d) @ time %d\n", hcd->obj->getId(), getId(), Clock::get()->getTime());

        if(hcd->obj->getFlag(GAM_CAN_LINK)/*&& (hcd->iDirection & m_uiDirections)*/) {
            we->moveObjectToArea(hcd->obj->getId(), m_uiSrcAreaId, m_uiDestAreaId);
            if(hcd->obj->getType() == TYPE_PLAYER) {
                m_lsDelayedObjs.push_back(hcd->obj);    //Later we may have more of these
            } else {
                Point ptPosDelta = m_ptDestPos - hcd->obj->getPhysicsModel()->getPosition();
                hcd->obj->moveBy(ptPosDelta);
            }
        }
        break;
      }
    case PWE_ON_ADDED_TO_AREA: {
        m_uiSrcAreaId = *(uint*)data;
        PWE::get()->addListener(this, PWE_ON_AREA_SWITCH_FROM, m_uiSrcAreaId);
        break;
      }
    case PWE_ON_ERASED_FROM_AREA:
    case PWE_ON_REMOVED_FROM_AREA: {
        uint uiAreaId = *(uint*)data;
        PWE::get()->removeListener(getId(), PWE_ON_AREA_SWITCH_FROM, uiAreaId);
        break;
      }
    case PWE_ON_AREA_SWITCH_FROM: {
        //TODO: Could cause mem bugs, should be using find
        for(list<GameObject*>::iterator it = m_lsDelayedObjs.begin(); it != m_lsDelayedObjs.end(); ++it) {
            Point ptPosDelta = m_ptDestPos - (*it)->getPhysicsModel()->getPosition();
            printf("Dest pos vs delta pos: (%f,%f,%f) / (%f,%f,%f)\n",
                   m_ptDestPos.x, m_ptDestPos.y, m_ptDestPos.z,
                   ptPosDelta.x, ptPosDelta.y, ptPosDelta.z
                   );
            (*it)->moveBy(ptPosDelta);
        }
        m_lsDelayedObjs.clear();
        status = EVENT_DROPPED;
        break;
      }
    default:
        status = EVENT_DROPPED;
        break;
    }
    return status;
}
