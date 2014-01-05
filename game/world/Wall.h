#ifndef WALL_H
#define WALL_H

#include "mge/GameObject.h"
#include "d3re/d3re.h"
#include "tpe/tpe.h"
#include "game/game_defs.h"

enum WallVisibleFaces {
    WALL_NONE  =  0x0,
    WALL_NORTH =  0x1,
    WALL_SOUTH =  0x2,
    WALL_EAST  =  0x4,
    WALL_WEST  =  0x8,
    WALL_UP    = 0x10,
    WALL_DOWN  = 0x20,
    WALL_DEFAULT = 0x1E,
    WALL_ALL   = 0x3F
};

class Wall : public GameObject
{
public:
    Wall(uint uiId, uint texTopId, uint texBottomId, uint texSideId, Box bxVolume, uint visibleFaces = WALL_DEFAULT);
    virtual ~Wall();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual uint getId() { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual bool update(uint time)              { return false; }
    virtual uint getType() { return TYPE_GENERAL; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "Wall"; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Misc
    void setColor(const Color &cr) { m_pRenderModel->setColor(cr); }
    Color &getColor() { return m_pRenderModel->getColor(); }

    //Listener
    virtual int callBack(uint uiID, void *data, uint id) { return EVENT_DROPPED; }

private:
    uint m_uiId, m_uiFlags;

    D3PrismRenderModel *m_pRenderModel;
    TimePhysicsModel   *m_pPhysicsModel;
};

#endif // WALL_H
