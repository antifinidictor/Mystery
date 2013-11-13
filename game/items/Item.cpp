#include "Item.h"
#include "pwe/PartitionedWorldEngine.h"

Item::Item(uint id, uint itemId, const Point &pos) {
    m_uiId = id;
    m_uiFlags = 0;
    m_pRenderModel = new D3SpriteRenderModel(this, D3RE::get()->getImageId("items"), Rect(-0.125f,0.f,0.25f,0.25f));
    m_pRenderModel->setFrameH(itemId);
    m_pPhysicsModel = new TimePhysicsModel(pos);
    Box bxVol = Box(-0.125f,0.f,-0.125f,0.25f,0.25f,0.25f);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxVol));
    m_iAnimTimer = 20;
    setFlag(TPE_PASSABLE, true);
}

Item::~Item() {
    //dtor
}


GameObject*
Item::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint id = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    uint itemId = pt.get(keyBase + ".itemId", 0);
    Point ptPos;
    ptPos.x = pt.get(keyBase + ".pos.x", 0.f);
    ptPos.y = pt.get(keyBase + ".pos.y", 0.f);
    ptPos.z = pt.get(keyBase + ".pos.z", 0.f);
    return new Item(id, itemId, ptPos);
}

void
Item::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    Point ptPos = m_pPhysicsModel->getPosition();
    pt.put(keyBase + ".id", getId());
    pt.put(keyBase + ".itemId", m_pRenderModel->getFrameH());
    pt.put(keyBase + ".pos.x", ptPos.x);
    pt.put(keyBase + ".pos.y", ptPos.y);
    pt.put(keyBase + ".pos.z", ptPos.z);
}

bool
Item::update(uint time) {
    //animation
    if(m_iAnimTimer < 0) {
        m_iAnimTimer = 20;
        int nextFrame = (m_pRenderModel->getFrameW() + 1) % 8;
        m_pRenderModel->setFrameW(nextFrame);
    } else {
        --m_iAnimTimer;
    }

    //Highlight on mouse over
    Point ptMousePos = D3RE::get()->getMousePos();
    if(ptInXZRect(ptMousePos, m_pPhysicsModel->getCollisionVolume())) {
        m_pRenderModel->setColor(Color(0x80,0x80,0xFF));
    } else {
        m_pRenderModel->setColor(Color(0xFF,0xFF,0xFF));
    }

    return false;
}

void
Item::callBack(uint uiID, void *data, uint id) {
}

uint
Item::getItemId() {
    return m_pRenderModel->getFrameH();
}
