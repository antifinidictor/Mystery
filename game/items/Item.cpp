#include "Item.h"
#include "pwe/PartitionedWorldEngine.h"
#define ANIM_TIMER_MAX 1

using namespace std;

Item::Item(uint id, uint itemId, const Point &pos)
    :   m_uiId(id),
        m_uiFlags(0),
        m_iAnimTimer(ANIM_TIMER_MAX),
        m_bCollidingWithPlayer(true),
        m_sItemInfo("no information available")
{
    m_pPhysicsModel = new TimePhysicsModel(pos);
    Box bxVol = Box(-0.125f,0.f,-0.125f,0.25f,0.25f,0.25f);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxVol));
    m_pPhysicsModel->setListener(this);
    m_pRenderModel = new D3SpriteRenderModel(m_pPhysicsModel, D3RE::get()->getImageId("items"), Rect(-0.125f,0.f,0.25f,0.25f));
    m_pRenderModel->setFrameH(itemId);
    //setFlag(TPE_PASSABLE, true);
    //setFlag(TPE_FALLING, true);

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
    string sInfo = pt.get(keyBase + ".info", "no information available");
    Item *item = new Item(id, itemId, ptPos);
    item->setItemInfo(sInfo);
    return item;
}

void
Item::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    Point ptPos = m_pPhysicsModel->getPosition();
    pt.put(keyBase + ".id", getId());
    pt.put(keyBase + ".itemId", m_pRenderModel->getFrameH());
    pt.put(keyBase + ".pos.x", ptPos.x);
    pt.put(keyBase + ".pos.y", ptPos.y);
    pt.put(keyBase + ".pos.z", ptPos.z);
    pt.put(keyBase + ".info", m_sItemInfo);
}

bool
Item::update(float fDeltaTime) {
    //animation
    if(m_iAnimTimer < 0) {
        m_iAnimTimer = ANIM_TIMER_MAX;
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

    //Survived one round without touching the player
    if(!m_bCollidingWithPlayer && !getFlag(GAM_CAN_PICK_UP)) {
        setFlag(GAM_CAN_PICK_UP, true);
        //setFlag(TPE_PASSABLE, false);
    }

    m_bCollidingWithPlayer = false; //This object is not yet colliding with the player
    return false;
}

int
Item::callBack(uint uiID, void *data, uint id) {
    switch(id) {
    case TPE_ON_COLLISION: {
        HandleCollisionData *hd = (HandleCollisionData*)data;
        if(hd->obj->getType() == TYPE_PLAYER) {
            m_bCollidingWithPlayer = true;
            printf("I hit the player @ time %d\n", Clock::get()->getTime());
        }
        break;
    }
    default:
        break;
    }
    return EVENT_DROPPED;
}

uint
Item::getItemId() {
    return m_pRenderModel->getFrameH();
}


void
Item::onItemPickup() {
}

void
Item::onItemDrop() {
    setFlag(GAM_CAN_PICK_UP, false);
    //setFlag(TPE_PASSABLE, true);
}


/*
#define ITEM_NONE 0
enum ElementItemIds {
    ITEM_ELEMENT_EARTH = 1,
    ITEM_ELEMENT_AIR,
    ITEM_ELEMENT_FIRE,
    ITEM_ELEMENT_WATER,
    ITEM_ELEMENT_TIME,
    ITEM_NUM_ELEMENTS
};

enum SpellItemIds {
    ITEM_SPELL_CYCLIC = ITEM_NUM_ELEMENTS,
    ITEM_SPELL_FLOW,
    ITEM_SPELL_VORTEX,
    ITEM_NUM_SPELLS
};

enum GeneralItemIds {
    ITEM_TEST = ITEM_NUM_SPELLS,
    ITEM_NUM_ITEMS
};
*/

string
Item::getItemName() {
    switch(m_pRenderModel->getFrameH()) {
    case ITEM_NONE:
        return "(none)";
    //Elements
    case ITEM_ELEMENT_EARTH:
        return "Earth (element)";
    case ITEM_ELEMENT_AIR:
        return "Air (element)";
    case ITEM_ELEMENT_FIRE:
        return "Fire (element)";
    case ITEM_ELEMENT_WATER:
        return "Water (element)";
    case ITEM_ELEMENT_TIME:
        return "Time (element)";
    //Spells
    case ITEM_SPELL_CYCLIC:
        return "Cycle (spell)";
    case ITEM_SPELL_FLOW:
        return "Flow (spell)";
    case ITEM_SPELL_VORTEX:
        return "Vortex (spell)";
    //General items
    case ITEM_TEST:
        return "Test";
    default:
        return "unknown";
    }
}
