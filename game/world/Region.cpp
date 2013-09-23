#include "Region.h"

#if 0
Region::Region(uint uiId, Box bxVolume)
{
    m_uiId = uiId;
    m_uiFlags = 0;

    m_pPhysicsModel = new TimePhysicsModel(bxVolume);
    m_pRenderModel = new OrderedRenderModel();

    m_bIsDead = false;
}

Region::~Region()
{
    delete m_pPhysicsModel;
    delete m_pRenderModel;
}

bool
Region::update(uint time) {
    return m_bIsDead;
}

void
Region::split(Box bxVolume) {
    //Splits a region along the lines of bxVolume, into several new regions
    Box bx0, bx1, bx2, bx3, bx4, bx5;

    //Correct the given volume so it does not extend outside of the region
    #if 0
    /*
    bx0 and bx5 are above and below this pattern, respectively
     ________________ _____
    |     ^          |     |
    |     l bx1      |     |
    |_____v__________|     |
    |     |          | bx2 |
    |<-w->| bxVolume |     |
    |     |__________|_____|
    | bx4 |                |
    |     |      bx3       |
    |_____|________________|
     */
    int w = bxVolume.x - m_bxVolume.x,
        l = bxVolume.y - m_bxVolume.y;
    bx0 = Box(m_bxVolume.x, m_bxVolume.y,   bxVolume.z + bxVolume.h,
              m_bxVolume.w, m_bxVolume.l, m_bxVolume.h - bxVolume.h);
    bx1 = Box(m_bxVolume.x,   m_bxVolume.y, bxVolume.z,
              bxVolume.w + w, l,            bxVolume.h);
    bx2 = Box(  bxVolume.x + bxVolume.w        m_bxVolume.y,     bxVolume.z,
              m_bxVolume.w - (bxVolume.w + w),   bxVolume.l + l, bxVolume.h);
    bx3 = Box(bxVolume.x,         bxVolume.y + bxVolume.l, bxVolume.z,
              m_bxVolume.w - w, m_bxVolume.l - l,          bxVolume.h);
    bx4 = Box(m_bxVolume.x, m_bxVolume.y + l, bxVolume.z,
              w,            m_bxVolume.l - l, bxVolume.h);
    bx5 = Box(m_bxVolume.x, m_bxVolume.y, m_bxVolume.z,
              m_bxVolume.w, m_bxVolume.l, m_bxVolume.z - bxVolume.z);
    buildRegion(bx0);
    buildRegion(bx1);
    buildRegion(bx2);
    buildRegion(bx3);
    buildRegion(bx4);
    buildRegion(bx5);

    m_bIsDead = true;
    #endif
}

bool isValid(const Box &bxVolume) {
    return bxVolume.w > 0 && bxVolume.l > 0 && bxVolume.h > 0;
}

void buildRegion(Box bxVolume) {
}
#endif
