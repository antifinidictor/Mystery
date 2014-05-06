/*
 * WaterElementalVolume.cpp
 */

#include "WaterElementalVolume.h"
#include "pwe/PartitionedWorldEngine.h"
#include "tpe/tpe.h"
#include "ForceField.h"
#include "game/GameManager.h"

WaterElementalVolume::WaterElementalVolume(uint id, uint texId, Box bxVolume, float fSwellRes, float fDensity)
    :   ElementalVolume(id)
        //m_pPhysicsModel(new TimePhysicsModel(this, bxCenter(bxVolume), fDensity)),
        //m_cgVelocities(NULL, bxVolume, m_fSwellRes)
{
printf(__FILE__" %d\n",__LINE__);

    Box bxRelativeVol =Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                            bxVolume.w,      bxVolume.h,      bxVolume.l);

    m_fSwellRes = fSwellRes;
    m_pxMap = new PixelMap(bxVolume.w / m_fSwellRes, bxVolume.l / m_fSwellRes,0);

    m_pPhysicsModel = new TimePhysicsModel(this, bxCenter(bxVolume), fDensity);
    m_pPhysicsModel->addCollisionModel(new PixelMapCollisionModel(bxRelativeVol, m_pxMap));
    m_pPhysicsModel->setListener(this);

    m_pVelocityGrids[0] = new InterpGrid<Vec3f>(m_pPhysicsModel, bxRelativeVol, bxRelativeVol.w / m_fSwellRes, 1, bxRelativeVol.l / m_fSwellRes);
    m_pVelocityGrids[1] = new InterpGrid<Vec3f>(m_pPhysicsModel, bxRelativeVol, bxRelativeVol.w / m_fSwellRes, 1, bxRelativeVol.l / m_fSwellRes);
    m_uiCurVelGrid = 0;

    m_pRenderModel = new D3HeightmapRenderModel(m_pPhysicsModel, texId, m_pxMap, bxRelativeVol);

    //Default swell values
    m_fSwellSize = 1.f;
    m_fSwellSpacingX = bxVolume.w / (m_pxMap->m_uiW - 1) * M_PI / 5.f;
    m_fSwellSpacingZ = bxVolume.l / (m_pxMap->m_uiH - 1) * M_PI / 5.f;


    for(uint x = 0; x < m_pxMap->m_uiW; ++x) {
        for(uint z = 0; z < m_pxMap->m_uiH; ++z) {
            m_pxMap->m_pData[x][z] = 0.5f;
        }
    }


    VortexForceField test(bxCenter(bxVolume), Point(0.f, 1.f, 0.f), 0.3f);
    int sizeX = m_pVelocityGrids[m_uiCurVelGrid]->getSizeX();
    int sizeY = m_pVelocityGrids[m_uiCurVelGrid]->getSizeY();
    int sizeZ = m_pVelocityGrids[m_uiCurVelGrid]->getSizeZ();
    for(int x = 0; x < sizeX; ++x) {
        Point pos;
        pos.x = x * bxVolume.w / sizeX + bxVolume.x;
        for(int z = 0; z < sizeZ; ++z) {
            pos.z = z * bxVolume.l / sizeZ + bxVolume.z;
            Point ptVel = test.getForceAt(pos);
            for(int y = 0; y < sizeY; ++y) {
                Vec3f &v = m_pVelocityGrids[m_uiCurVelGrid]->at(x, y, z);
                v = ptVel;
            }
        }
    }


    setFlag(TPE_LIQUID, true);
    setFlag(TPE_STATIC, true);
    setFlag(D3RE_TRANSPARENT, true);
}

WaterElementalVolume::~WaterElementalVolume() {
    delete m_pPhysicsModel;
    delete m_pRenderModel;
}

GameObject*
WaterElementalVolume::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
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
    WaterElementalVolume *obj = new WaterElementalVolume(uiId, uiTexId, bxVolume, 0.1f, fDensity);
    Color cr;
    cr.r = pt.get(keyBase + ".cr.r", 0x0);
    cr.g = pt.get(keyBase + ".cr.g", 0x0);
    cr.b = pt.get(keyBase + ".cr.b", 0xFF);
    obj->m_pRenderModel->setColor(cr);
    return obj;
}

void
WaterElementalVolume::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
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
    pt.put(keyBase + ".cr.r", cr.r);
    pt.put(keyBase + ".cr.g", cr.g);
    pt.put(keyBase + ".cr.b", cr.b);
    pt.put(keyBase + ".density", m_pPhysicsModel->getDensity());
}

bool
WaterElementalVolume::update(float fDeltaTime) {
    Point ptMousePos = D3RE::get()->getMousePos();
    if(ptInXZRect(ptMousePos, m_pPhysicsModel->getCollisionVolume())) {
        m_pRenderModel->setColor(Color(0x00,0xFF,0xFF));
        GameManager::get()->addActiveVolume(this);
    } else {
        m_pRenderModel->setColor(Color(0x00,0x00,0xFF));
        GameManager::get()->removeActiveVolume(getId());
    }

    //Update swells
    //float fTime = fDeltaTime / 1000.f;
    uint sizeX = m_pVelocityGrids[m_uiCurVelGrid]->getSizeX();
    uint sizeY = m_pVelocityGrids[m_uiCurVelGrid]->getSizeY();
    uint sizeZ = m_pVelocityGrids[m_uiCurVelGrid]->getSizeZ();
    uint uiNextGrid = (m_uiCurVelGrid + 1) % 2;
    for(uint x = 0; x < sizeX; ++x) {
        uint minX = x == 0 ? x : x - 1;
        uint maxX = (x + 1) == sizeX ? x : x + 1;
        for(uint z = 0; z < sizeZ; ++z) {
            //Height determined by flow out vs flow in
            uint minZ = z == 0 ? z : z - 1;
            uint maxZ = (z + 1) == sizeZ ? z : z + 1;

            float magFlowIn = 0.f;
            Vec3f v3Avg;
            float fAvgCount = 0.f;
            for(uint wx = minX; wx <= maxX; ++wx) {
                for(uint wz = minZ; wz <= maxZ; ++wz) {
                    Vec3f v3Vel;
                    for(uint wy = 0; wy < sizeY; ++wy) {
                        //Accumulate velocities in this column
                        v3Vel += m_pVelocityGrids[m_uiCurVelGrid]->at(wx, wy, wz);
                        fAvgCount++;
                    }
                    v3Avg += v3Vel;
                    v3Vel.y = 0.f;  //Unimportant, we only care about velocity into/ out of the column

                    //Get the velocity flowing into/out of this column
                    if(wx == x && wz == z) {
                        magFlowIn -= v3Vel.magnitude();
                    } else {
                        //Project cumulate velocity onto direction vector
                        //For flows flowing away, this should be negative
                        Vec3f v3Dir = Vec3f(x - wx, 0.f, z - wz);
                        v3Dir.normalize();
                        magFlowIn += dot(v3Vel, v3Dir);
                    }
                }
            }

            v3Avg *= 1.0f / fAvgCount;   //Divide by 9 for average, add decay term as well

            for(uint y = 0; y < sizeY; ++y) {
                //Update velocity at these points
                m_pVelocityGrids[uiNextGrid]->at(x, y, z) = v3Avg;
            }


            m_pxMap->m_pData[x][z] = 0.5f + magFlowIn / 8.f;
            /*
            //Mix height using velocity and a corrective term
            Point ptVel = m_cgVelocities->at(x,maxH,z);
            uint x1 = (ptVel.x < 0.f) ? x - 1 : x + 1;
            uint z1 = (ptVel.z < 0.f) ? z - 1 : z + 1;
            float mean = (m_pxMap->m_pData[x-1][z-1] +
                          m_pxMap->m_pData[x-1][z] +
                          m_pxMap->m_pData[x-1][z+1] +
                          m_pxMap->m_pData[x][z-1] +
                          m_pxMap->m_pData[x][z] +
                          m_pxMap->m_pData[x][z+1] +
                          m_pxMap->m_pData[x+1][z-1] +
                          m_pxMap->m_pData[x+1][z] +
                          m_pxMap->m_pData[x+1][z+1]) / 9.f;

            float h00 = m_pxMap->m_pData[x][z];
            float hdiff = h00 * ptVel.magnitude() * fTime;
            m_pxMap->m_pData[x][z1]  += hdiff / 3.f;
            m_pxMap->m_pData[x1][z]  += hdiff / 3.f;
            m_pxMap->m_pData[x1][z1] += hdiff / 3.f;
            m_pxMap->m_pData[x][z]   -= hdiff;
            */
//printf(__FILE__" %d (%d,%d)\n",__LINE__, x,z);
//printf("%f,%f(%f,%f,%f)\n", h00, hdiff, ptVel.x, ptVel.y, ptVel.z);
            /*
            m_pxMap->m_pData[x][z] = ((m_fSwellSize / 2.f)
                * sin((x + fTime) * M_PI / 5.f)
                * sin((z + fTime) * M_PI / 5.f)
                + m_fSwellSize / 2);
            */
        }
    }
    m_uiCurVelGrid = uiNextGrid;

    return false;
}


void
WaterElementalVolume::setVolume(float fVolume) {
}


void
WaterElementalVolume::addVolumeAt(float fVolume, const Point &pos) {
}

float
WaterElementalVolume::getVolume() {
    return m_pPhysicsModel->getVolume();
}

void
WaterElementalVolume::interpRestore(float fTime) {
}

void
WaterElementalVolume::beginRestore() {
}

void
WaterElementalVolume::endRestore() {
}


float
WaterElementalVolume::getHeightAt(const Point &pt) {
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
            break;
          }
        case CM_Y_HEIGHTMAP: {
            PixelMapCollisionModel *pxmdl = (PixelMapCollisionModel*)cmdl;
            fHeight += pxmdl->getHeightAtPoint(ptAdjusted);
            break;
          }
        default:
            break;
        }
    }
    return fHeight;
}
