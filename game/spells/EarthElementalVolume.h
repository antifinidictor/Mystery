#ifndef EARTHELEMENTALVOLUME_H
#define EARTHELEMENTALVOLUME_H

#include "game/spells/ElementalVolume.h"
#include "game/game_defs.h"
#include "d3re/D3HeightmapRenderModel.h"
#include "tpe/TimePhysicsModel.h"
#include "mge/PixelMap.h"
#include "tpe/CollisionModel.h"

class EarthElementalVolume : public ElementalVolume
{
public:
    EarthElementalVolume(uint id, uint texId, const Box &bxVolume, float fDensity);
    virtual ~EarthElementalVolume();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    virtual bool update(float fDeltaTime);
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "EarthElementalVolume"; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

    //Elemental volume
    virtual void setVolume(float fVolume);
    virtual void addVolumeAt(float fVolume, const Point &pos);
    virtual float getVolume();
    virtual void interpRestore(float fTime);
    virtual void beginRestore();
    virtual void endRestore();

    virtual float getHeightAt(const Point &pt);
protected:
private:
    void addVolumeToHmap(PixelMapCollisionModel *mdl, float volume);
    void addVolumeToBox(BoxCollisionModel *mdl, float volume);

    D3HeightmapRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
    PixelMap *m_pxMap;

    //Interpolation maps
    PixelMap *m_pxOrigMap;
    PixelMap *m_pxTempMap;
    Box m_bxOrigBounds;

    float m_fVolume;
};

#endif // EARTHELEMENTALVOLUME_H
