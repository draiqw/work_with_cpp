#pragma once
#include <cmath>
#include <iostream>

struct Vec3 {
    double x, y, z;
    
    Vec3(double x_=0, double y_=0, double z_=0): x(x_), y(y_), z(z_) {}
    Vec3 operator+(const Vec3& b) const {
        return Vec3(x+b.x, y+b.y, z+b.z);
    }
    Vec3 operator-(const Vec3& b) const {
        return Vec3(x-b.x, y-b.y, z-b.z);
    }
    Vec3 operator*(double k) const {
        return Vec3(x*k, y*k, z*k);
    }
    Vec3 operator/(double k) const {
        return Vec3(x/k, y/k, z/k);
    }
    Vec3& operator+=(const Vec3& b) {
        x+=b.x; y+=b.y; z+=b.z;
        return *this;
    }
    double norm() const {
        return std::sqrt(x*x+y*y+z*z);
    }
    static double dot(const Vec3& a, const Vec3& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return Vec3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
    }
    friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        os << v.x << " " << v.y << " " << v.z; return os;
    }
};
