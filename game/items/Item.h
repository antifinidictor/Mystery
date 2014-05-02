#ifndef ITEM_H
#define ITEM_H

#include "mge/GameObject.h"
#include "game/game_defs.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"


#define ITEM_NONE 0
enum ElementItemIds {
    ITEM_ELEMENT_FIRE = 1,
    ITEM_ELEMENT_AIR,
    ITEM_ELEMENT_EARTH,
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

class Item : public GameObject
{
public:
    Item(uint id, uint imgId, const Point &pos);
    virtual ~Item();

    //I/O
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual uint getId() { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual bool update(float fDeltaTime);
    virtual uint getType() { return TYPE_ITEM; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "Item"; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Listener
    virtual int callBack(uint uiID, void *data, uint id);

    uint getItemId();
    std::string getItemName();
    std::string getItemInfo() { return m_sItemInfo; }
    void setItemInfo(std::string sItemInfo) { m_sItemInfo = sItemInfo; }
    virtual void onItemPickup();
    virtual void onItemDrop();

private:
    uint m_uiId;
    flag_t m_uiFlags;

    int m_iAnimTimer;

    bool m_bCollidingWithPlayer;

    D3SpriteRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
    std::string m_sItemInfo;
};

#endif // ITEM_H
