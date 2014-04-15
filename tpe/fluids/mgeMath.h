#ifndef MGE_MATH_H
#define MGE_MATH_H

#include "mge/defs.h"

//Useful math structures
template<int rows, int cols>
struct Matrix {
    float m[rows][cols];

    Matrix() {
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                m[r][c] = 0;
            }
        }
    }

    Matrix(const Matrix<rows,cols> &rhs) {
        (*this) = rhs;
    }

    Matrix<rows,cols> &operator=(const Matrix<rows,cols> &rhs) {
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                m[r][c] = rhs[r][c];
            }
        }
        return *this;
    }
    //Constant ops
    Matrix<rows,cols> operator+(const Matrix<rows,cols> &rhs) const {
        Matrix<rows,cols> result;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                result[r][c] = m[r][c] + rhs[r][c];
            }
        }
        return result;
    }

    Matrix<rows,cols> operator-(const Matrix<rows,cols> &rhs) const {
        Matrix<rows,cols> result;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                result[r][c] = m[r][c] - rhs[r][c];
            }
        }
        return result;
    }


    Matrix<rows,cols> operator*(float scalar) const {
        Matrix<rows,cols> result;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                result[r][c] = m[r][c] * scalar;
            }
        }
        return result;
    }

    Matrix<rows,cols> operator/(float scalar) const {
        Matrix<rows,cols> result;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                result[r][c] = m[r][c] / scalar;
            }
        }
        return result;
    }

    float *operator[](int r) {
        return m[r];
    }

    const float *operator[](int r) const {
        return m[r];
    }

/*
    friend Matrix<rows,cols> Matrix<rows,cols> operator*(float scalar, const Matrix<rows,cols> &rhs) const {
        return rhs * scalar;
    }
*/

    Matrix<cols,rows> transpose() {
        Matrix<cols,rows> result;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                result.m[c][r] = m[r][c];
            }
        }
        return result;
    }

    //Matrix multiplication
    template<int cols2>
    Matrix<rows,cols2> operator*(const Matrix<cols,cols2> &rhs) const {
        Matrix<rows,cols2> result;
        const int rows2 = cols;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols2; ++c) {
                //Get the sum of multiples across all rows
                for(int rc = 0; rc < rows2; ++rc) {
                    result[r][c] += m[r][rc] * rhs[rc][c];
                }
            }
        }
        return result;
    }
};

Matrix<3,1> toMatrix(const Point &pt);

Point toPoint(const Matrix<3,1> &mat);

#endif // MGE_MATH_H
