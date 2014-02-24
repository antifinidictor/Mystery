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

    m_pxMap = new PixelMap(bxVolume.w * HMAP_RES + 1, bxVolume.l * HMAP_RES + 1, 0);
    m_pRenderModel = new D3HeightmapRenderModel(this, texId, m_pxMap, bxRelativeVol);

    m_pPhysicsModel = new TimePhysicsModel(bxCenter(bxVolume), fDensity);
    m_pPhysicsModel->addCollisionModel(new PixelMapCollisionModel(bxRelativeVol, m_pxMap));
    m_pPhysicsModel->setListener(this);


    for(uint x = 0; x < m_pxMap->m_uiW; ++x) {
        for(uint z = 0; z < m_pxMap->m_uiH; ++z) {
            m_pxMap->m_pData[x][z] = 0.5f;// * (x * z) / (m_pxMap->m_uiW * m_pxMap->m_uiH);
        }
    }

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
EarthElementalVolume::update(uint time) {
    Point ptMousePos = D3RE::get()->getMousePos();
    if(ptInXZRect(ptMousePos, m_pPhysicsModel->getCollisionVolume())) {
        //m_pRenderModel->setColor(Color(0x00,0xFF,0xFF));
        GameManager::get()->addActiveVolume(this);
    } else {
        //m_pRenderModel->setColor(Color(0x00,0x00,0xFF));
        GameManager::get()->removeActiveVolume(getId());
    }

    #if 1
static int tempTimer = 0;
    if(--tempTimer < 0) {
        tempTimer = 10;

        //Sanity check
        //float fVolBef = getVolume();

        //Height adjustment by force field
        Point aaPosWindow[3][3];
        const uint curWinX = 1; //The window never steps along the x direction
        Box bxVol = m_pPhysicsModel->getCollisionVolume();
        for(uint x = 1; x < m_pxMap->m_uiW - 1; ++x) {
            //Fill out the first two window rows at the new winX coordinates
            //This optimization should reduce the number of times we need to calculate a particular point
            for(int wx = -1; wx <= 1; ++wx) {
                for(int wz = -1; wz <= 0; ++wz) {   //The last z-row will be filled out
                    aaPosWindow[wx + 1][wz + 1] = Point(
                        bxVol.x + (x + wx) / HMAP_RES,
                        bxVol.y + m_pxMap->m_pData[x + wx][1 + wz] * bxVol.h,
                        bxVol.z + (1 + wz) / HMAP_RES
                    );
                }
            }

            //Fill out the rest of the coordinates
            uint curWinZ = 1;   //Index into the window
            uint nextWinZ = 2;
            for(uint z = 1; z < m_pxMap->m_uiH - 1; ++z) {
                //Fill out the last window row at the new winZ coordinate
                for(int wx = -1; wx <= 1; ++wx) {
                    aaPosWindow[wx + 1][nextWinZ] = Point(
                        bxVol.x + (x + wx) / HMAP_RES,
                        bxVol.y + m_pxMap->m_pData[x + wx][z + 1] * bxVol.h,
                        bxVol.z + (z + 1) / HMAP_RES
                    );
                }

                //Calculate the new height by approximating from the forces
                Point ptForce = getTotalForceAt(aaPosWindow[curWinX][curWinZ]);
                ptForce.y = 0;
                if(ptForce.magnitude() > 0.f) {
                    for(uint wx = 0; wx < 3; ++wx) {
                        for(uint wz = 0; wz < 3; ++wz) {
                            uint wdz = (curWinZ + wz + 2) % 3;  //When wz = 1, wdz = curWinZ.  When wz = 2, wdz = nextWinZ
                            Point ptVec = aaPosWindow[curWinX][curWinZ] - aaPosWindow[wx][wdz];
                            ptVec.y = 0;
                            ptVec.normalize();

                            //Get the magnitude and direction of the force projected onto the direction vector
                            float mag = dot(ptVec, ptForce) / 10;
                            if(mag > 1.f || mag < -1.f) {
                                printf("ERROR: mag is too big! %f\n", mag);
                            }

                            //Adjust the heights, bounded by 0.f/1.f
                            uint dx = x + wx - 1;   //index into the pixel map
                            uint dz = z + wz - 1;
                            float fHeightShift = (m_pxMap->m_pData[x][z] + m_pxMap->m_pData[dx][dz]) * mag / 2.f;
                            #define MAX_HEIGHT 1.f
                            #define MIN_HEIGHT 0.0f
                            if(m_pxMap->m_pData[x][z] + fHeightShift > MAX_HEIGHT) {
                                fHeightShift = MAX_HEIGHT - m_pxMap->m_pData[x][z];
                            } else if(m_pxMap->m_pData[x][z] + fHeightShift < MIN_HEIGHT) {
                                fHeightShift = MIN_HEIGHT - m_pxMap->m_pData[x][z];
                            }
                            if(m_pxMap->m_pData[dx][dz] - fHeightShift > MAX_HEIGHT) {
                                fHeightShift = m_pxMap->m_pData[dx][dz] - MAX_HEIGHT;
                            } else if(m_pxMap->m_pData[dx][dz] - fHeightShift < MIN_HEIGHT) {
                                fHeightShift = m_pxMap->m_pData[dx][dz] - MIN_HEIGHT;
                            }
                            m_pxMap->m_pData[x][z] += fHeightShift;
                            m_pxMap->m_pData[dx][dz] -= fHeightShift;
                        }
                    }
                }
                curWinZ = nextWinZ;
                nextWinZ = (nextWinZ + 1) % 3;
            }
        }
        //printf("Total volume before/after: %f/%f\n", fVolBef, getVolume());
    }
    #endif

    return false;
}


void
EarthElementalVolume::setVolume(float fVolume) {
}

#define BOUND(min, val, max) ((val < min) ? min : ((val > max) ? max : val))
void
EarthElementalVolume::addVolumeAt(float fVolume, const Point &ptPos) {
    Box bxBounds = m_pPhysicsModel->getCollisionVolume();
    //Find the four nearest points and split the volume by interpolation

    //Scale ptPos to a set of four indices
    float x = (ptPos.x - bxBounds.x) * (m_pxMap->m_uiW - 1) / bxBounds.w;
    float z = (ptPos.z - bxBounds.z) * (m_pxMap->m_uiH - 1) / bxBounds.l;
#if 1
    float fVolPerUnit = (bxBounds.w * bxBounds.l * bxBounds.h) / ((m_pxMap->m_uiW) * (m_pxMap->m_uiH) * 1.f);
    float fTotalVol = 0.f;
    for(uint ux = 0; ux < m_pxMap->m_uiW; ++ux) {
        for(uint uz = 0; uz < m_pxMap->m_uiH; ++uz) {
            float dx = (x - ux);
            float dz = (z - uz);
            float distance = sqrt(dx * dx + dz * dz);
            float dh = (distance < 1.f) ? fVolume : fVolume / distance;
            fTotalVol += m_pxMap->m_pData[0][0] * fVolPerUnit;
            m_pxMap->m_pData[ux][uz] = BOUND(0.f, m_pxMap->m_pData[ux][uz] + dh, 1.f);
        }
    }
    printf("Unit vol: %f Total vol: %f Actual vol: %f\n",
           m_pxMap->m_pData[0][0] * fVolPerUnit,
           fTotalVol,
           bxBounds.w * bxBounds.l * (bxBounds.h) / 2.f
           );
#else
    int fx = BOUND(0, (int)floor(x), m_pxMap->m_uiW - 1);
    int fz = BOUND(0, (int)floor(z), m_pxMap->m_uiH - 1);
    int cx = BOUND(0, (int)ceil(x), m_pxMap->m_uiW - 1);
    int cz = BOUND(0, (int)ceil(z), m_pxMap->m_uiH - 1);

    if(fx < 0 || cx >= m_pxMap->m_uiW || fz < 0 || cz >= m_pxMap->m_uiH) {
        return;
    }

    //Split the new volume among the nearest four points
    float zDiff = (fz == cz) ? 0.5f : (cz - z) / (cz - fz);
    float zInterpVolF = zDiff * fVolume;
    float zInterpVolC = (1 - zDiff) * fVolume;
    float xDiff = (fx == cx) ? 0.5f : (cx - x) / (cx - fx);

    //Add interpolated volumes
    m_pxMap->m_pData[fx][fz] = BOUND(0.f, m_pxMap->m_pData[fx][fz] + zInterpVolF * xDiff, 1.f);
    m_pxMap->m_pData[fx][cz] = BOUND(0.f, m_pxMap->m_pData[fx][cz] + zInterpVolC * xDiff, 1.f);
    m_pxMap->m_pData[cx][cz] = BOUND(0.f, m_pxMap->m_pData[cx][cz] + zInterpVolC * (1 - xDiff), 1.f);
    m_pxMap->m_pData[cx][fz] = BOUND(0.f, m_pxMap->m_pData[cx][fz] + zInterpVolF * (1 - xDiff), 1.f);
#endif
}

float
EarthElementalVolume::getVolume() {
    float fVolume = 0.f;
    for(uint x = 0; x < m_pxMap->m_uiW; ++x) {
        for(uint z = 0; z < m_pxMap->m_uiH; ++z) {
            fVolume += m_pxMap->m_pData[x][z];
        }
    }
    //For now, this is sufficient
    return fVolume;//m_pPhysicsModel->getVolume();
}
