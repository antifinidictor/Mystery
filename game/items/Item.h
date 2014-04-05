#ifndef ITEM_H
#define ITEM_H

#include "mge/GameObject.h"
#include "game/game_defs.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"

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
    virtual bool update(uint time);
    virtual uint getType() { return TYPE_ITEM; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "Item"; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Listener
    virtual int callBack(uint uiID, void *data, uint id);

    uint getItemId();

private:
    uint m_uiId, m_uiFlags;

    int m_iAnimTimer;

    bool m_bCollidingWithPlayer;

    D3SpriteRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
};

#endif // ITEM_H
