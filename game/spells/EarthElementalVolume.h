#ifndef EARTHELEMENTALVOLUME_H
#define EARTHELEMENTALVOLUME_H

#include "game/spells/ElementalVolume.h"
#include "game/game_defs.h"
#include "d3re/D3HeightmapRenderModel.h"
#include "tpe/TimePhysicsModel.h"
#include "mge/PixelMap.h"

class EarthElementalVolume : public ElementalVolume
{
public:
    EarthElementalVolume(uint id, uint texId, const Box &bxVolume, float fDensity);
    virtual ~EarthElementalVolume();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    virtual bool update(uint time);
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "EarthElementalVolume"; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Elemental volume
    virtual void setVolume(float fVolume);
    virtual void addVolumeAt(float fVolume, const Point &pos);
    virtual float getVolume();
protected:
private:
    D3HeightmapRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
    PixelMap *m_pxMap;

    float m_fVolume;
};

#endif // EARTHELEMENTALVOLUME_H
