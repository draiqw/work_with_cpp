#pragma once
#include <string>
#include "Vec3.h"

class SolarBody {
    double mass;
    Vec3 pos;
    Vec3 vel;
    std::string name;
public:
    SolarBody(
        double m, const Vec3& q, const Vec3& v, const std::string& n
    ): mass(m), pos(q), vel(v), name(n) {}

    double get_mass() const {
        return mass;
    }
    const Vec3& get_pos() const {
        return pos;
    }
    const Vec3& get_vel() const {
        return vel;
    }
    const std::string& get_name() const {
        return name;
    }
    void set_pos(const Vec3& q) {
        pos = q;
    }
    void set_vel(const Vec3& v) {
        vel = v;
    }
};