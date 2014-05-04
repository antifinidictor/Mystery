/*
 * Water
 */

#include "Water.h"
#include "pwe/PartitionedWorldEngine.h"
#define MAX_EXPANSION_TIME 100

Water::Water(uint id, uint texId, Box bxVolume, float fDensity) {
    m_uiID = id;
    m_uiFlags = 0;

    Box bxRelativeVol =Box(-bxVolume.w / 2, -bxVolume.h / 2, -bxVolume.l / 2,
                            bxVolume.w,      bxVolume.h,      bxVolume.l);

    PixelMap *m_pxMap = new PixelMap(2, 2,0);
    for(uint i = 0; i < m_pxMap->m_uiW; ++i) {
        for(uint j = 0; j < m_pxMap->m_uiH; ++j) {
            m_pxMap->m_pData[i][j] = 1.f;
        }
    }

    m_pPhysicsModel = new TimePhysicsModel(bxCenter(bxVolume), fDensity);
    m_pPhysicsModel->addCollisionModel(new PixelMapCollisionModel(bxRelativeVol, m_pxMap));
    m_pPhysicsModel->setListener(this);

    m_pRenderModel = new D3HeightmapRenderModel(m_pPhysicsModel, texId, m_pxMap, bxRelativeVol);

    m_pNorth = m_pEast = m_pSouth = m_pWest = NULL;

    setFlag(TPE_LIQUID, true);
    setFlag(TPE_STATIC, true);
    setFlag(D3RE_TRANSPARENT, true);
    m_iTimer = 0;
    m_uiExpansionFlags = 0xFFFFFFFF;
}

Water::~Water() {
    PWE::get()->freeId(getId());
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

GameObject*
Water::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
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
    Water *obj = new Water(uiId, uiTexId, bxVolume, fDensity);
    Color cr;
    cr.r = pt.get(keyBase + ".cr.r", 0x0);
    cr.g = pt.get(keyBase + ".cr.g", 0x0);
    cr.b = pt.get(keyBase + ".cr.b", 0xFF);
    obj->m_pRenderModel->setColor(cr);
    return obj;
}

void
Water::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
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
Water::update(float fDeltaTime) {
    if(m_iTimer < MAX_EXPANSION_TIME) {
        m_iTimer++;
    } else {
        m_iTimer = 0;
        expand();
        m_uiExpansionFlags = 0;
    }
    m_pPhysicsModel->setPhysicsChanged(true);
    return false;
}

void
Water::setColor(const Color &cr) {
    m_pRenderModel->setColor(cr);
}

Color &
Water::getColor() {
    return m_pRenderModel->getColor();
}

int
Water::callBack(uint cID, void *data, uint uiEventId) {
    int status = EVENT_CAUGHT;
    switch(uiEventId) {
    case TPE_ON_COLLISION: {
        handleCollision((HandleCollisionData*)data);
        break;
      }
    default:
        status = EVENT_DROPPED;
        break;
    }
    return status;
}

void
Water::handleCollision(HandleCollisionData *data) {
    //We'll have some other conditions for failure to expand, but for now this will do
    if(data->obj->getFlag(TPE_STATIC)) {
        //Direction is meaningless.  Need to get the position of the object relative to myself
        Box bxMe = m_pPhysicsModel->getCollisionVolume();
        Box bxThem = data->obj->getPhysicsModel()->getCollisionVolume();
        int iDir = data->iDirection;    //DOWN;
        switch(iDir) {
        case BIT_NORTH:
        case BIT_EAST:
        case BIT_SOUTH:
        case BIT_WEST:
        case BIT_DOWN:
        case BIT_UP:
            //Single directions = face collision
            m_uiExpansionFlags |= iDir;
            break;
        default:
            break;
        }
    } else {
        //Nonstatic collision
    }
    //printf("Water %d collided with %d\n", getId(), data->obj->getId());
}

void
Water::expand() {
    //PWE *we = PWE::get();
    PixelMapCollisionModel *pmdl = (PixelMapCollisionModel*)m_pPhysicsModel->getCollisionModel(0);
    Box bxMe = pmdl->getBounds(); //m_pPhysicsModel->getCollisionVolume();
    for(int dir = 0; dir < NUM_DIRECTIONS; ++dir) {
        if(!GET_BIT(m_uiExpansionFlags, dir)) {
            switch(dir) {
            case NORTH:
                bxMe.l += 1.0f;
                printf("Expand %d (NORTH)\n", NORTH);
                break;
            case EAST:
                bxMe.x -= 1.0f;
                bxMe.w += 1.0f;
                printf("Expand %d (EAST)\n", EAST);
                break;
            case SOUTH:
                bxMe.z -= 1.0f;
                bxMe.l += 1.0f;
                printf("Expand %d (SOUTH)\n", SOUTH);
                break;
            case WEST:
                bxMe.w += 1.0f;
                printf("Expand %d (WEST)\n", WEST);
                break;
            case DOWN:
                //bxMe.y -= 1.0f; //water falls
                break;
            default:
                break;
            }
            //PixelMapCollisionModel *pmdl = new PixelMapCollisionModel();
        }
    }

    pmdl->m_bxBounds = bxMe;
    m_pPhysicsModel->resetVolume();

    /*
    if(m_pEast == NULL) {
        m_pEast = new Water(
            we->genId(),
            m_pRenderModel->getTexture(UP),
            Box(bxMe.x + bxMe.w, bxMe.y, bxMe.z, bxMe.w, bxMe.h, bxMe.l)
        );
        m_pEast->m_pWest = this;
        we->add(m_pEast);
    }
    */
}
