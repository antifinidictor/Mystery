/*
 * WaterElementalVolume
 * Water volume for spellcasting.
 * Properties:
 * -Dynamic volume: It expands to fill the current region
 */

#ifndef WATER_ELEMENTAL_VOLUME_H
#define WATER_ELEMENTAL_VOLUME_H

#include "ElementalVolume.h"
#include "mge/GameObject.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"
#include "game/game_defs.h"

class WaterElementalVolume : public ElementalVolume {
public:
    WaterElementalVolume(uint id, uint texId, Box bxVolume, float fSwellRes = 1.f, float fDensity = DENSITY_WATER);
    virtual ~WaterElementalVolume();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    virtual bool update(uint time);
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "WaterElementalVolume"; }

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

private:
    D3HeightmapRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
    PixelMap *m_pxMap;
    float m_fSwellSize;
    float m_fSwellSpacingX;
    float m_fSwellSpacingZ;
    float m_fSwellRes;
    float m_fOriginalVolume;
    float m_fTargetVolume;
};


#endif //WATER_ELEMENTAL_VOLUME_H
