#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <epsilon.h>
#include <matrix.h>
#include <bounded.h>

using namespace std;

struct AugmentedMatrix{
    Matrix <double> &A;
    Matrix <double> &B;
    Matrix <double> &X;
    string Filename;
    AugmentedMatrix(
        Matrix <double> &A,
        Matrix <double> &B,
        Matrix <double> &X,
        string Filename
    );
    AugmentedMatrix(const AugmentedMatrix &other) = delete;
    AugmentedMatrix& operator=(const AugmentedMatrix &other) = delete;

    AugmentedMatrix(AugmentedMatrix &&other){
        A = move(other.A);
        B = move(other.B);
        X = move(other.X);
        Filename = move(other.Filename);
    }
    AugmentedMatrix& operator=(AugmentedMatrix &&other) {
        if (this != &other) {
            A = move(other.A);
            B = move(other.B);
            X = move(other.X);
            Filename = move(other.Filename);
        }
        return *this;
    }
    
};

struct Producer{
    Bounded <AugmentedMatrix> &q;
    size_t n_min;
    size_t n_max;
    Producer(Bounded <AugmentedMatrix> &q, size_t n_min, size_t n_max);
    void operator()(){
        for (size_t i = n_min; i < n_max; ++i){
            
        }
    };
};

struct Consumer{
    Bounded <AugmentedMatrix> &q;
    size_t qtty;
    Consumer(Bounded <AugmentedMatrix> &q, size_t qtty);
    void operator()(){};
};

AugmentedMatrix make_matrix(size_t m, size_t n){
    if (n + 2 != m) throw std::runtime_error("Matrix size is not correct");
    Matrix <double> A(n, n);
    Matrix <double> B(n, 1);
    Matrix <double> X(n, 1);
    for (int i = 0; i < (int) n-1; i ++){
        A(i, i) = (double) (7/5);
        A(i, i+1) = (double) (11/3);
        int k = i + 1;
        B(i, 0) = (double) (
            (152 * k + 118.0) 
            / 
            (15.0 * (2 * k + 1) * (2 * k + 3))
        );
        X(i, 0) = (double) (1 / ( 2 * k + 1));
    }
    A(n-1, n-1) = (double) (7/5);
    B(n-1, 0) = (double) (7 / (5 * (2 * (n - 1) +   1)));
    X(n-1, 0) = (double) (1 / (2 * (n - 1) + 1));
    AugmentedMatrix AM(A, B, X, "test.txt");
    return AM;
}   
int main(){
    // cout << "Hello, World!" << endl;

    // cout << "epsilon1 <float> = " << epsilon1 <float>() << endl;
    // cout << "epsilon1 <double> = " << epsilon1 <double>() << endl;
    // cout << "epsilon1 <long double> = " << epsilon1 <long double>() << endl;

    // cout << "epsilon0 <float> = " << epsilon0 <float>() << endl;
    // cout << "epsilon0 <double> = " << epsilon0 <double>() << endl;
    // cout << "epsilon0 <long double> = " << epsilon0 <long double>() << endl;

    Matrix <double> A(3, 3);
    A(0, 0) = double{1}; A(0, 1) = double{2}; A(0, 2) = double{3};
    A(1, 0) = double{4}; A(1, 1) = double{5}; A(1, 2) = double{6};
    A(2, 0) = double{7}; A(2, 1) = double{8}; A(2, 2) = double{9};
    cout << "FDS \n\n" << A << endl;
    
    Matrix <double> A_2diag(3, 3);
    A_2diag(0, 0) = double{2}; A_2diag(0, 1) = double{-1}; A_2diag(0, 2) = double{0};
    A_2diag(1, 0) = double{0}; A_2diag(1, 1) = double{3}; A_2diag(1, 2) = double{1};
    A_2diag(2, 0) = double{0}; A_2diag(2, 1) = double{0}; A_2diag(2, 2) = double{4};

    Matrix<double> B = std::move(A);
    cout << "FDS \n\n" << B << endl;

    Matrix <double> x(3, 1); 
    // Matrix <double> x_err(2, 1);
    
    Matrix <double> B(3, 1);
    B(0, 0) = double{3};
    B(1, 0) = double{5};
    B(2, 0) = double{8};

    cout << "A_double_diagonal\n\n"<< A_2diag << endl;
    cout << "B \n\n"<< B << endl;

    try {
        solve(A_2diag, x, B);
    } 
    catch (const std::runtime_error & e){
        cout << e.what() << endl;
    }
    
    cout << "X\n\n" << x << endl;


    Bounded <AugmentedMatrix> queue {10};

    Producer p1(queue, 1, 16);
    Producer p2(queue, 17, 32);
    Consumer c1(queue, 32);
    Consumer c2(queue, 32);

    return 0;
}    