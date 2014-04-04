
#ifndef FORCE_FIELD
#define FORCE_FIELD

#include "mge/defs.h"

class ForceField {
public:
    virtual ~ForceField() {}
    virtual Point getForceAt(const Point &pt) = 0;
};

class SourceForceField : public ForceField {
public:
    SourceForceField(const Point &ptSource, float fMagnitude) {
        m_fMagnitude = fMagnitude;
        m_ptSource = ptSource;
    }

    virtual ~SourceForceField() {}

    virtual Point getForceAt(const Point &pt) {
        Point ptDiff = pt - m_ptSource; //vector from source to point
        float fSqDist = (ptDiff.x * ptDiff.x + ptDiff.y * ptDiff.y + ptDiff.z * ptDiff.z);
        if(fSqDist < 1.f) {
            fSqDist = 1.f;
        }
        ptDiff.normalize();
        return ptDiff * m_fMagnitude / fSqDist;
    }
private:
    Point m_ptSource;
    float m_fMagnitude;
};


class SinkForceField : public ForceField {
public:
    SinkForceField(const Point &ptSink, float fMagnitude) {
        m_fMagnitude = fMagnitude;
        m_ptSink = ptSink;
    }
    virtual ~SinkForceField() {}

    virtual Point getForceAt(const Point &pt) {
        Point ptDiff = m_ptSink - pt; //vector from point to sink
        float fSqDist = (ptDiff.x * ptDiff.x + ptDiff.y * ptDiff.y + ptDiff.z * ptDiff.z);
        if(fSqDist < 1.f) {
            fSqDist = 1.f;
        }
        ptDiff.normalize();
        return ptDiff * m_fMagnitude / fSqDist;
    }
private:
    Point m_ptSink;
    float m_fMagnitude;
};

class LineForceField : public ForceField {
public:
    LineForceField(const Point &ptSource, const Point &ptSink, float fMagnitude) {
        m_fMagnitude = fMagnitude;
        m_ptSource = ptSource;
        m_ptSink = ptSink;
    }
    virtual ~LineForceField() {}

    virtual Point getForceAt(const Point &pt) {
        //Distance between a point and a line (from WolframMathWorld)
        Point ptDiffPtSink = pt - m_ptSink;
        Point ptDiffPtSource = pt - m_ptSource;
        Point ptDiffSinkSource = m_ptSink - m_ptSource;
        float fDist = cross(ptDiffPtSource, ptDiffPtSink).magnitude() / ptDiffSinkSource.magnitude();

        if(fDist < 1.f) {
            fDist = 1.f;
        }
        ptDiffSinkSource.normalize();
        return ptDiffSinkSource * m_fMagnitude / fDist / fDist;
    }
private:
    Point m_ptSource;
    Point m_ptSink;
    float m_fMagnitude;
};

class VortexForceField : public ForceField {
public:
    VortexForceField(const Point &pos, const Point &norm, float mag) {
        m_ptCenter = pos;
        m_ptNormal = norm;
        m_fMagnitude = mag;

        m_ptNormal.normalize();
    }
    virtual ~VortexForceField() {}

    virtual Point getForceAt(const Point &pt) {
        Point ptDiff = pt - m_ptCenter;
        float fDist = ptDiff.magnitude();
        if(fDist < 1.f) {
            fDist = 1.f;
        }
        float fForceMag = m_fMagnitude / fDist / fDist;
        ptDiff.normalize();
        Point ptForce = cross(ptDiff, m_ptNormal);
        ptForce *= fForceMag / ptForce.magnitude();
        return ptForce;
    }

private:
    Point m_ptCenter;
    Point m_ptNormal;
    float m_fMagnitude;
};
#endif //FORCE_FIELD
