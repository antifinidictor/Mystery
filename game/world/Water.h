/*
 * Water
 * Liquid test
 */

#ifndef WATER_H
#define WATER_H

#include "mge/GameObject.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"
#include "game/game_defs.h"
#include <list>

class Water : public GameObject {
public:
    Water(uint id, uint texId, Box bxVolume, float fDensity = DENSITY_WATER);      //Approx. density of granite

    virtual ~Water();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual uint getId() { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual bool update(uint time);
    virtual uint getType() { return TYPE_GENERAL; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "Water"; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Misc
    void setColor(const Color &cr);
    Color &getColor();

    //Listener
    virtual int callBack(uint cID, void *data, uint uiEventId);

private:
    struct WaterCell {
        WaterCell() {
            m_uiPhysIndex = 0;
            m_pxMap = NULL;
        }
        uint m_uiPhysIndex;
        PixelMap *m_pxMap;
    };

    struct WaterRow {
        std::list<WaterCell> m_lsCells;
    };

    void handleCollision(HandleCollisionData *data);
    void expand();

    uint m_uiID, m_uiFlags;

    D3HeightmapRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
        PixelMap *m_pxMap;  //temporary
    std::list<WaterRow> m_lsRows;    //Physics & render stored in a grid
    int m_iTimer;
    uint m_uiExpansionFlags;    //True for a direction if it is blocked from expansion

    Water *m_pNorth, *m_pEast, *m_pSouth, *m_pWest;
};

#endif
