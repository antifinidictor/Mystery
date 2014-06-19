
#include "mgeMath.h"
using namespace std;

Matrix<3,1> toMatrix(const Point &pt) {
    Matrix<3,1> result;
    result[0][0] = pt.x;
    result[1][0] = pt.y;
    result[2][0] = pt.z;
    return result;
}

Point toPoint(const Matrix<3,1> &mat) {
    Point result;
    result.x = mat[0][0];
    result.y = mat[1][0];
    result.z = mat[2][0];
    return result;
}


Vec3f matMult(const Matrix<3,3> &mat, const Vec3f &v) {
    Vec3f result = Vec3f(
        mat[0][0] * v.x + mat[0][1] * v.y + mat[0][2] * v.z,
        mat[1][0] * v.x + mat[1][1] * v.y + mat[1][2] * v.z,
        mat[2][0] * v.x + mat[2][1] * v.y + mat[2][2] * v.z
    );
    return result;
}

Vec3f matMult(const Vec3f &v, const Matrix<3,3> &mat) {

#if DEBUG_VORTONS
printf(__FILE__" %d:[[%2.2f,%2.2f,%2.2f]\n", __LINE__,
    mat[0][0], mat[0][1], mat[0][2]
);
printf(__FILE__" %d: [%2.2f,%2.2f,%2.2f]\n", __LINE__,
    mat[1][0], mat[1][1], mat[1][2]
);
printf(__FILE__" %d: [%2.2f,%2.2f,%2.2f]]\n", __LINE__,
    mat[2][0], mat[2][1], mat[2][2]
);
#endif
    Vec3f result = Vec3f(
        mat[0][0] * v.x + mat[1][0] * v.y + mat[2][0] * v.z,
        mat[0][1] * v.x + mat[1][1] * v.y + mat[2][1] * v.z,
        mat[0][2] * v.x + mat[1][2] * v.y + mat[2][2] * v.z
    );
    return result;
}
