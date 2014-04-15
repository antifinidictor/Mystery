#include "EarthElementalVolume.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/GameManager.h"

//TODO: remove
#include "game/FxSprite.h"

#define HMAP_RES 4.f
EarthElementalVolume::EarthElementalVolume(uint id, uint texId, const Box &bxVolume, float fDensity) :
    ElementalVolume(id)
{
    Box bxRelativeVol =Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                            bxVolume.w,      bxVolume.h,      bxVolume.l);

    m_pPhysicsModel = new TimePhysicsModel(bxCenter(bxVolume), fDensity);
    m_pPhysicsModel->setListener(this);

    m_pxMap = new PixelMap(bxVolume.w * HMAP_RES + 1, bxVolume.l * HMAP_RES + 1, 0);
    m_pRenderModel = new D3HeightmapRenderModel(m_pPhysicsModel, texId, m_pxMap, bxRelativeVol);



    for(uint x = 0; x < m_pxMap->m_uiW; ++x) {
        for(uint z = 0; z < m_pxMap->m_uiH; ++z) {
            m_pxMap->m_pData[x][z] = 0.5f;// * (x * z) / (m_pxMap->m_uiW * m_pxMap->m_uiH);
        }
    }
    m_pPhysicsModel->addCollisionModel(new PixelMapCollisionModel(bxRelativeVol, m_pxMap));

    //m_pxOrigMap = new PixelMap(*m_pxMap);
    //m_pxTempMap = NULL;

    setFlag(TPE_STATIC, true);

    //TODO: Remove
    m_pRenderModel->setColor(Color(0xFF,0xFF,0x00));
}

EarthElementalVolume::~EarthElementalVolume() {
    delete m_pPhysicsModel;
    delete m_pRenderModel;
}


GameObject*
EarthElementalVolume::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint uiId = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    uint uiTexId = pt.get(keyBase + ".tex", 0);
    Box bxVolume;
    bxVolume.x = pt.get(keyBase + ".vol.x", 0.f);
    bxVolume.y = pt.get(keyBase + ".vol.y", 0.f);
    bxVolume.z = pt.get(keyBase + ".vol.z", 0.f);
    bxVolume.w = pt.get(keyBase + ".vol.w", 0.f);
    bxVolume.h = pt.get(keyBase + ".vol.h", 0.f);
    bxVolume.l = pt.get(keyBase + ".vol.l", 0.f);
    float fDensity = pt.get(keyBase + ".density", DENSITY_WATER);
    EarthElementalVolume *obj = new EarthElementalVolume(uiId, uiTexId, bxVolume, fDensity);
    return obj;
}

void
EarthElementalVolume::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    pt.put(keyBase + ".id", getId());
    pt.put(keyBase + ".tex", m_pRenderModel->getTexture());
    Box bxVolume = m_pPhysicsModel->getCollisionVolume();
    pt.put(keyBase + ".vol.x", bxVolume.x);
    pt.put(keyBase + ".vol.y", bxVolume.y);
    pt.put(keyBase + ".vol.z", bxVolume.z);
    pt.put(keyBase + ".vol.w", bxVolume.w);
    pt.put(keyBase + ".vol.h", bxVolume.h);
    pt.put(keyBase + ".vol.l", bxVolume.l);
    Color cr = m_pRenderModel->getColor();
    pt.put(keyBase + ".density", m_pPhysicsModel->getDensity());
}
bool
EarthElementalVolume::update(float fDeltaTime) {
    Point ptMousePos = D3RE::get()->getMousePos();
    if(ptInXZRect(ptMousePos, m_pPhysicsModel->getCollisionVolume())) {
        //m_pRenderModel->setColor(Color(0x00,0xFF,0xFF));
        GameManager::get()->addActiveVolume(this);
    } else {
        //m_pRenderModel->setColor(Color(0x00,0x00,0xFF));
        GameManager::get()->removeActiveVolume(getId());
    }

    return false;
}


void
EarthElementalVolume::setVolume(float fVolume) {
}

#define BOUND(min, val, max) ((val < min) ? min : ((val > max) ? max : val))
void
EarthElementalVolume::addVolumeAt(float fVolume, const Point &ptPos) {
    //Find the four nearest points and split the volume by interpolation
    float fTotalVolume = m_pPhysicsModel->getVolume();
    for(uint i = 0; i < m_pPhysicsModel->getNumModels(); ++i) {
        CollisionModel *mdl = m_pPhysicsModel->getCollisionModel(i);
        float fThisVolume = fVolume * mdl->getVolume() / fTotalVolume;
        switch(mdl->getType()) {
        case CM_BOX:
            addVolumeToBox((BoxCollisionModel*)mdl, fThisVolume);
            break;
        case CM_Y_HEIGHTMAP:
            addVolumeToHmap((PixelMapCollisionModel*)mdl, fThisVolume);
            break;
        default:
            break;
        }
    }

    m_pPhysicsModel->resetVolume();
}

void
EarthElementalVolume::addVolumeToHmap(PixelMapCollisionModel *mdl, float fVolume) {
    float fTotalArea = mdl->m_bxBounds.w * mdl->m_bxBounds.l;
    float fApproxTotalNewVolume = fTotalArea * mdl->m_bxBounds.h + fVolume;
    float fExtraCellHeight = fApproxTotalNewVolume / fTotalArea - mdl->m_bxBounds.h;

    //Since heights are from 0.f to 1.f (% of the total height):
    fExtraCellHeight /= mdl->m_bxBounds.h;

    float fMaxHeight = 1.f;
    float fMinHeight = 0.f;
    for(uint x = 0; x < m_pxMap->m_uiW; ++x) {
        for(uint z = 0; z < m_pxMap->m_uiH; ++z) {
            m_pxMap->m_pData[x][z] += fExtraCellHeight;
            if(m_pxMap->m_pData[x][z] > fMaxHeight) {
                fMaxHeight = m_pxMap->m_pData[x][z];
            } else if(m_pxMap->m_pData[x][z] < fMinHeight) {
                fMinHeight = m_pxMap->m_pData[x][z];
            }
        }
    }
    if(fMinHeight < 0.f || fMaxHeight > 1.f) {
        //Establish new bounds
        float fNewLowBound =  mdl->m_bxBounds.y + mdl->m_bxBounds.h * fMinHeight;
        float fNewHeight = mdl->m_bxBounds.h * fMaxHeight;

        //Adjust internal parameters accordingly
        for(uint x = 0; x < m_pxMap->m_uiW; ++x) {
            for(uint z = 0; z < m_pxMap->m_uiH; ++z) {
                float fActualHeight = mdl->m_bxBounds.y + mdl->m_bxBounds.h * m_pxMap->m_pData[x][z];
                m_pxMap->m_pData[x][z] = (fActualHeight - fNewLowBound) / fNewHeight;
            }
        }

        //Adjust the bounding box
        mdl->m_bxBounds.h = fNewHeight;
        mdl->m_bxBounds.y = fNewLowBound;
    }
}

void
EarthElementalVolume::addVolumeToBox(BoxCollisionModel *mdl, float fVolume) {
    float fExtraHeight = fVolume / (mdl->m_bxBounds.w * mdl->m_bxBounds.l);
    mdl->m_bxBounds.h += fExtraHeight;
}


float
EarthElementalVolume::getVolume() {
    //For now, this is sufficient
    return m_pPhysicsModel->getVolume();
}

void
EarthElementalVolume::interpRestore(float fTime) {
    /*
    if(m_pxTempMap == NULL) {
        printf("ERROR: Null TEMP map while restoring!\n");
        exit(-1);
    }
    float fOtherTime = 1.f - fTime;
    for(uint x = 0; x < m_pxMap->m_uiW; ++x) {
        for(uint z = 0; z < m_pxMap->m_uiH; ++z) {
            m_pxMap->m_pData[x][z] = m_pxOrigMap->m_pData[x][z] * fTime + m_pxTempMap->m_pData[x][z] * fOtherTime;
        }
    }
    */
}

void
EarthElementalVolume::beginRestore() {
    /*
    if(m_pxTempMap != NULL) {
        printf("WARNING: Mid-restoration begin!");
        delete m_pxTempMap;
    }
    m_pxTempMap = new PixelMap(*m_pxMap);
    */
}

void
EarthElementalVolume::endRestore() {
    /*
    if(m_pxTempMap == NULL) {
        printf("ERROR: Null TEMP map while ending restoration!\n");
        exit(-1);
    }
    delete m_pxTempMap;
    m_pxTempMap = NULL;
    */
}


float
EarthElementalVolume::getHeightAt(const Point &pt) {
    //Get the physics-position adjusted point for use in bounds comparison
    Point ptAdjusted = pt - m_pPhysicsModel->getPosition();
    float fHeight = m_pPhysicsModel->getPosition().y;   //Base collision volume
    for(uint curCmdl = 0; curCmdl < m_pPhysicsModel->getNumModels(); ++curCmdl) {
        //Make sure this point is in the bounds of this collision model
        CollisionModel *cmdl = m_pPhysicsModel->getCollisionModel(curCmdl);
        if(!ptInXZRect(ptAdjusted, cmdl->getBounds())) {
            continue;
        }

        //Get height information specific to the type of collision model
        switch(cmdl->getType()) {
        case CM_BOX: {
            BoxCollisionModel *bxmdl = (BoxCollisionModel*)cmdl;
            fHeight += bxmdl->m_bxBounds.y + bxmdl->m_bxBounds.h;
          }
        case CM_Y_HEIGHTMAP: {
            PixelMapCollisionModel *pxmdl = (PixelMapCollisionModel*)cmdl;
            fHeight += pxmdl->getHeightAtPoint(ptAdjusted);
          }
        default:
            break;
        }
    }
    return fHeight;
}
