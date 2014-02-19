#include "CollisionModel.h"

#define BOUND(min, val, max) ((val < min) ? min : ((val > max) ? max : val))

float
PixelMapCollisionModel::getHeightAtPoint(const Point &ptPos) {
    //Scale ptPos to a set of four indices
    float x = (ptPos.x - m_bxBounds.x) * (m_pxMap->m_uiW - 1) / m_bxBounds.w;
    float z = (ptPos.z - m_bxBounds.z) * (m_pxMap->m_uiH - 1) / m_bxBounds.l;
    int fx = BOUND(0, (int)floor(x), m_pxMap->m_uiW - 1);
    int fz = BOUND(0, (int)floor(z), m_pxMap->m_uiH - 1);
    int cx = BOUND(0, (int)ceil(x), m_pxMap->m_uiW - 1);
    int cz = BOUND(0, (int)ceil(z), m_pxMap->m_uiH - 1);

    if(fx < 0 || cx >= m_pxMap->m_uiW || fz < 0 || cz >= m_pxMap->m_uiH) {
        return 0.f;
    }

    //Get heights at four neighboring indices
    float yff = m_pxMap->m_pData[fx][fz];
    float yfc = m_pxMap->m_pData[fx][cz];
    float ycc = m_pxMap->m_pData[cx][cz];
    float ycf = m_pxMap->m_pData[cx][fz];

    //Interpolate
    float xDiff = (fx == cx) ? 0.5f : (cx - x) / (cx - fx);
    float xInterpFz = (xDiff) * yff + (1.f - xDiff) * ycf;
    float xInterpCz = (xDiff) * yfc + (1.f - xDiff) * ycc;
    float zDiff = (fz == cz) ? 0.5f : (cz - z) / (cz - fz);
    float interp = zDiff * xInterpFz + (1 - zDiff) * xInterpCz;
    interp = interp * (m_bxBounds.h) + m_bxBounds.y;

    return interp;
}
