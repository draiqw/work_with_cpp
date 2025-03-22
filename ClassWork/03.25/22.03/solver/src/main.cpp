#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <epsilon.h>
#include <matrix.h>

using namespace std;

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
    
    Matrix <double> A_2diag(3, 3);
    A_2diag(0, 0) = double{2}; A_2diag(0, 1) = double{-1}; A_2diag(0, 2) = double{0};
    A_2diag(1, 0) = double{0}; A_2diag(1, 1) = double{3}; A_2diag(1, 2) = double{1};
    A_2diag(2, 0) = double{0}; A_2diag(2, 1) = double{0}; A_2diag(2, 2) = double{4};

    Matrix <double> x(3, 1); 
    Matrix <double> x_err(2, 1);
    

    Matrix <double> B(3, 1);
    B(0, 0) = double{3};
    B(1, 0) = double{5};
    B(2, 0) = double{8};

    
    // try {
    //     solve(A, x, B);
    // } 
    // catch (const std::runtime_error & e){
    //     cout << e.what() << endl;
    // }
    
    // try {
    //     solve(A, x_err, B);
    // } 
    // catch (const std::runtime_error & e){
    //     cout << e.what() << endl;
    // }

    cout << "A\n\n"<< A << endl; // A.print();
    cout << "A_double_diagonal\n\n"<< A_2diag << endl;
    cout << "X\n\n" << x << endl;
    cout << "B \n\n"<< B << endl;

    try {
        solve(A_2diag, x, B);
    } 
    catch (const std::runtime_error & e){
        cout << e.what() << endl;
    }
    
    cout << "X\n\n" << x << endl;
    return 0;


}