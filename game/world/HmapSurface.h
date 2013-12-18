#ifndef HMAPSURFACE_H
#define HMAPSURFACE_H

#include "mge/GameObject.h"
#include "mge/PixelMap.h"
#include "game/game_defs.h"
#include "d3re/d3re.h"
#include "tpe/tpe.h"

class HmapSurface : public GameObject
{
public:
    HmapSurface(uint id, uint texId, const std::string &sMapFile, const Box &bxVolume);
    virtual ~HmapSurface();

    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    virtual uint getId() { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }

    virtual bool update(uint time);
    virtual uint getType() { return TYPE_GENERAL; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "HmapSurface"; }

    RenderModel *getRenderModel() { return m_pRenderModel; }
    PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Listener
	virtual void callBack(uint uiEventHandlerId, void *data, uint uiEventId);
private:
    uint m_uiId;
    uint m_uiFlags;
    PixelMap *m_pxMap;

    std::string m_sMapFile;

    D3HeightmapRenderModel *m_pRenderModel;
    TimePhysicsModel *m_pPhysicsModel;
};

#endif // HMAPSURFACE_H
