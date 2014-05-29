#include "CollisionModel.h"

/*
 * PixelMapCollisionModel
 */
float
PixelMapCollisionModel::getVolume() {
    //Approximate the volume.  This won't be perfectly accurate but it should be fairly close
    float fPercentCellVolume = 0.f;
    float fCellVolume = m_bxBounds.w * m_bxBounds.l * m_bxBounds.h / (m_pxMap->m_uiW * m_pxMap->m_uiH);
    for(uint x = 0; x < m_pxMap->m_uiW; ++x) {
        for(uint z = 0; z < m_pxMap->m_uiH; ++z) {
            fPercentCellVolume += m_pxMap->m_pData[x][z];
        }
    }
    return fPercentCellVolume * fCellVolume;
}

float
PixelMapCollisionModel::getHeightAtPoint(const Point &ptPos) {
    //Scale ptPos to a set of four indices
    float x = (ptPos.x - m_bxBounds.x) * (m_pxMap->m_uiW - 1) / m_bxBounds.w;
    float z = (ptPos.z - m_bxBounds.z) * (m_pxMap->m_uiH - 1) / m_bxBounds.l;
    int fx = BOUND(0, (int)floor(x), m_pxMap->m_uiW - 1);
    int fz = BOUND(0, (int)floor(z), m_pxMap->m_uiH - 1);
    int cx = BOUND(0, (int)ceil(x), m_pxMap->m_uiW - 1);
    int cz = BOUND(0, (int)ceil(z), m_pxMap->m_uiH - 1);

/*
    if(fx < 0 || cx >= m_pxMap->m_uiW || fz < 0 || cz >= m_pxMap->m_uiH) {
        return 0.f;
    }
*/

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

Vec3f
PixelMapCollisionModel::getNormalAtPoint(const Point &ptPos) {
    //Get the indices of the corners of the particular cell this point is in
    int width = m_pxMap->m_uiW - 1;
    int length = m_pxMap->m_uiH - 1;
    float x = (ptPos.x - m_bxBounds.x) * width / m_bxBounds.w;
    float z = (ptPos.z - m_bxBounds.z) * length / m_bxBounds.l;
    int minX = BOUND(0, (int)floor(x), width);
    int minZ = BOUND(0, (int)floor(z), length);
    int maxX = BOUND(0, (int)ceil(x), width);
    int maxZ = BOUND(0, (int)ceil(z), length);

    //Get the height at each of the corner indices
    float y00 = m_pxMap->m_pData[minX][minZ];
    float y01 = m_pxMap->m_pData[minX][maxZ];
    float y11 = m_pxMap->m_pData[maxX][maxZ];
    float y10 = m_pxMap->m_pData[maxX][minZ];

    //Calculate the normal using the vector cross product
    /* This is the layout of the triangle strip:
     * y00-y10
     *  | / |
     * y01-y11
     */
    //Determine which triangle we are using
    float zPrime = maxX - x + minZ;
    Vec3f v1;
    Vec3f v2;
    if(zPrime < z) {	//TODO: The triangles are determined correctly, but not the normals
        v1 = Vec3f(0.f,         y01 - y00, maxZ - minZ);
        v2 = Vec3f(maxX - minX, y10 - y00, 0.f);
    } else {
        v1 = Vec3f(0.f,         y10 - y11, minZ - maxZ);
        v2 = Vec3f(minX - maxX, y01 - y11, 0.f);
    }
    Vec3f normal = cross(v1, v2);
    if(normal.magSq() == 0.f) {
        normal.y = 1.f;
    } else {
        normal.normalize();
    }
    return normal;
}

/*
 * VortonCollisionModel
 */
VortonCollisionModel::VortonCollisionModel(Positionable *pParent, const Vec3f &v3InitVorticity, float fRadius)
    :   m_pParent(pParent),
        m_v3Vorticity(v3InitVorticity),
        m_fRadius(fRadius)
{
}

void
VortonCollisionModel::update(float fTimeQuantum) {
    Point ptPosition = m_pParent->getPosition();
    Matrix<3,3> matJacobian = m_pFluidManager->getJacobianAt(ptPosition);
    Vec3f v3Velocity        = m_pFluidManager->getVelocityAt(ptPosition);

    //Stretching and tilting (halve this to preserve stability)
    m_v3Vorticity += matMult(m_v3Vorticity, matJacobian) * fTimeQuantum * 0.5f;

    //Advect this vorton
    m_pParent->moveBy(v3Velocity * fTimeQuantum);
    //m_ptVelocity = ptVelocity;

    //Inform the fluid manager that this update has occurred
    m_pFluidManager->onVortonUpdated();
}

Vec3f
VortonCollisionModel::velocityAt(const Point &pos) {
    //Taken directly from intel code
    Vec3f diff = pos - m_pParent->getPosition();
    float radius2 = m_fRadius * m_fRadius;
    float dist2 = diff.magSq();
    float oneOverDist = 1.f / sqrt(dist2);
    float distLaw = (dist2 < radius2) ? (oneOverDist / radius2) : (oneOverDist / dist2);
    float oneOverFourPi = 1 / M_PI / 4;
    return cross(m_v3Vorticity, diff) * oneOverFourPi * ( 8.0f * radius2 * m_fRadius ) * distLaw;
}

void
VortonCollisionModel::exchangeVorticityWith(float fViscocity, VortonCollisionModel *v) {
    //Vorticity adjustment is based on the viscocity from 0-1
    Point v3VorticityExchange = (m_v3Vorticity - v->m_v3Vorticity) *  fViscocity;
    float fRemainingVorticity = 1.f - fViscocity;

    //Adjust my vorticity
    m_v3Vorticity = m_v3Vorticity * fRemainingVorticity + v3VorticityExchange;

    //Adjust their vorticity
    v->m_v3Vorticity = v->m_v3Vorticity * fRemainingVorticity - v3VorticityExchange;
}
