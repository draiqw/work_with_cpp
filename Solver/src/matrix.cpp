#include "matrix.h"
#include <iostream>

template <typename T> T solve(const Matrix_Base <T> & A, Matrix_Base <T> &x, const Matrix_Base <T> & B){
    if (A.m_rows() != A.n_cols()){
        throw std::runtime_error(
            "Error: Matrix is not square"
        );
    }
    if (A.n_cols() != x.m_rows() || A.m_rows() != B.m_rows()){
        throw std::runtime_error(
            "Error: Matrix size do not match to be solve"
        );
    }
    if (x.n_cols() != 1 || B.n_cols() != 1){
        throw std::runtime_error(
            "Error: x and B must be column vectors"
        );
    }

    int flag_double_diagonal = 0;
    for (size_t i = 1; i < A.m_rows(); i++){
        for (size_t j = 0; j < A.n_cols(); ++j){
            if (i != j && i != (j - 1) && A(i, j) != T{0}){
                flag_double_diagonal = 1;
                break;
            } 
        }
    }
    if (flag_double_diagonal != 0){
        throw std::runtime_error(
            "Error: Matrix is not double diagonal"
        );
    }

    size_t n = A.m_rows();
    for (size_t i = n; i-- > 0;) {
        T sum = B(i, 0);
        if (i < n - 1) {
            sum -= A(i, i + 1) * x(i + 1, 0);
        }
        x(i, 0) = sum / A(i, i);
    }


    return T{0};
}

template float solve(const Matrix_Base <float> & A, Matrix_Base <float> &x, const Matrix_Base <float> & B);
template double solve(const Matrix_Base <double> & A, Matrix_Base <double> &x, const Matrix_Base <double> & B);
template long double solve(const Matrix_Base <long double> & A, Matrix_Base <long double> &x, const Matrix_Base <long double> & B);