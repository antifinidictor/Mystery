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
    float yff = (float)m_pxMap->m_pData[fx][fz].toUint();
    float yfc = (float)m_pxMap->m_pData[fx][cz].toUint();
    float ycc = (float)m_pxMap->m_pData[cx][cz].toUint();
    float ycf = (float)m_pxMap->m_pData[cx][fz].toUint();

    //Interpolate
    float xDiff = (fx == cx) ? 0.5f : (cx - x) / (cx - fx);
    float xInterpFz = (xDiff) * yff + (1.f - xDiff) * ycf;
    float xInterpCz = (xDiff) * yfc + (1.f - xDiff) * ycc;
    float zDiff = (fz == cz) ? 0.5f : (cz - z) / (cz - fz);
    float interp = zDiff * xInterpFz + (1 - zDiff) * xInterpCz;
    interp = interp / MAX_COLOR_VAL * (m_bxBounds.h) + m_bxBounds.y;

    return interp;
}
