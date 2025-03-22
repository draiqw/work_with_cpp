#include <epsilon.h>

template <typename T> T epsilon1(){
    T eps = T{1};
    while (T{1} + eps / T{2} > T{1}){
        eps /= T{2};
    }
    return eps;
}

template <typename T> T epsilon0(){
    T eps = T{1};
    while (T{0} +  eps / T{2} > T{0}){
        eps /= T{2};
    }
    return eps;
}

template float epsilon1<float>();
template double epsilon1<double>();
template long double epsilon1<long double>();

template float epsilon0<float>();
template double epsilon0<double>();
template long double epsilon0<long double>();