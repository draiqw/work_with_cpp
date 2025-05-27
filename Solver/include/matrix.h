#pragma once
#include <iostream>
#include <algorithm>     // std::copy

template <typename T>
class Matrix_Base {
    size_t m_, n_;
public:
    Matrix_Base(size_t m, size_t n) : m_{m}, n_{n} {}

    virtual T        operator()(size_t i, size_t j = 0) const = 0;
    virtual T&       operator()(size_t i, size_t j = 0)       = 0;
    virtual void     print()                                   = 0;

    size_t m_rows() const { return m_; }
    size_t n_cols() const { return n_; }
};

template <typename T> class Matrix: public Matrix_Base <T> {
    private:
        T * data;
    public:
        Matrix(size_t _m, size_t _n): Matrix_Base <T>{_m, _n}{
            data = new T[_m * _n];
        };

        T operator()(size_t i, size_t j = 0) const override {return data[i * this -> n_cols() + j];}
        T& operator()(size_t i, size_t j = 0) override {return data[i * this -> n_cols() + j];}

        Matrix(const Matrix& other)
            : Matrix_Base<T>(other.m_rows(), other.n_cols())
        {
            data = new T[other.m_rows() * other.n_cols()];
            std::copy(other.data,
                    other.data + other.m_rows() * other.n_cols(),
                    data);
        }

        void print() override {
            for (size_t i = 0; i < this->m_rows(); ++i){
                for (size_t j = 0; j < this->n_cols(); ++j){
                    std::cout << this->operator()(i, j) << "\t";
                }
                std::cout << std::endl;
            }
        }

        // Move-конструктор
        Matrix(Matrix&& other): Matrix_Base<T>{other.m_rows(),other.n_cols()}, data(other.data) {
            other.data = nullptr;
        }

        // Move-оператор присваивания
        Matrix& operator=(Matrix && other) noexcept {
            if (this != &other) {
                delete[] data;
                data = other.data;
                other.data = nullptr;
            }
            return *this;
        }

        ~Matrix(){
            delete[] data;
        }

        template <typename U>
        friend std::ostream& operator<<(std::ostream& os, const Matrix<U>& mat);
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& mat) {
    for (size_t i = 0; i < mat.m_rows(); ++i) {
        for (size_t j = 0; j < mat.n_cols(); ++j) {
            os << mat(i, j) << '\t';
        }
        os << '\n';
    }
    return os;
}

template <typename T> T solve(const Matrix_Base <T> & A, Matrix_Base <T> &x, const Matrix_Base <T> & B);
