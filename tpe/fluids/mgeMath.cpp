
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
