#pragma once
#include "SolarSystem.h"

class Integrator {
    SolarSystem& system;
    double epsrel;
public:
    Integrator(SolarSystem& sys, double eps): system(sys), epsrel(eps) {}
    void integrate(double t_start, double t_end, double dt_init);
private:
    bool invariants_ok(
        double old_E,
        double new_E,
        const Vec3& old_P,
        const Vec3& new_P,
        const Vec3& old_L,
        const Vec3& new_L
    );
};
