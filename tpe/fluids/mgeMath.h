#ifndef MGE_MATH_H
#define MGE_MATH_H

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

    //Constant ops
    Matrix<rows,cols> operator+(const Matrix<rows,cols> &rhs) const {
        Matrix<rows,cols> result;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                result[r][c] = m[r][c] + rhs[r][c];
            }
        }
    }

    Matrix<rows,cols> operator-(const Matrix<rows,cols> &rhs) const {
        Matrix<rows,cols> result;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                result[r][c] = m[r][c] - rhs[r][c];
            }
        }
    }


    Matrix<rows,cols> operator*(float scalar) const {
        Matrix<rows,cols> result;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                result[r][c] = m[r][c] * scalar;
            }
        }
    }

    Matrix<rows,cols> operator/(float scalar) const {
        Matrix<rows,cols> result;
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols; ++c) {
                result[r][c] = m[r][c] / scalar;
            }
        }
    }
/*
    friend Matrix<rows,cols> Matrix<rows,cols> operator*(float scalar, const Matrix<rows,cols> &rhs) const {
        return rhs * scalar;
    }
*/

    //Matrix multiplication
    template<int rows2, int cols2>
    Matrix<rows,cols2> operator*(const Matrix<rows2,cols2> &rhs) const {
        Matrix<rows,cols2> result;
        if(cols2 != rows) {
            //printf("ERROR: Cannot multiply matrices!\n");
            return result;
        }
        for(int r = 0; r < rows; ++r) {
            for(int c = 0; c < cols2; ++c) {
                //Get the sum of multiples across all rows
                for(int rx = 0; rx < rows2; ++rx) {
                    for(int cx = 0; cx < cols; ++cx) {
                        result.m[r][c] += m[r][cx] * rhs.m[rx][c];
                    }
                }
            }
        }
    }
};


#endif // MGE_MATH_H
