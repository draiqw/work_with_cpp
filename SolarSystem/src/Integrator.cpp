#include "Integrator.h"
#include <iostream>
#include <cmath>
#include <iomanip>

#ifdef DEBUG
#define DBG(x) do { x; } while(0)
#else
#define DBG(x) do {} while(0)
#endif

void Integrator::integrate(double t_start, double t_end, double dt_init) {
    double t = t_start, dt = dt_init;
    auto m = system.get_masses();
    auto q = system.get_positions();
    auto v = system.get_velocities();
    size_t step = 0;

    // --- Стартовый отладочный вывод ---
    DBG(
        std::cout << "\n========== [INTEGRATOR START] ==========\n";
        std::cout << "Epoch date: " << system.get_epoch_date() << "\n";
        std::cout << "Bodies parsed: " << system.get_bodies().size() << "\n";
        for (const auto& b : system.get_bodies())
            std::cout << std::setw(8) << b.get_name()
                      << " | mass: " << std::setw(12) << b.get_mass()
                      << " | pos: " << b.get_pos()
                      << " | vel: " << b.get_vel() << "\n";
        std::cout << "Integration from t = " << t_start
                  << " to t = " << t_end << " sec, dt_init = " << dt_init << " sec\n";
        std::cout << "=========================================\n";
    );

    // --- Сохраним начальные инварианты для сравнения
    #ifdef DEBUG
        double E0 = system.compute_energy();
        Vec3 P0 = system.compute_total_momentum();
        Vec3 L0 = system.compute_angular_momentum();
    #endif
    DBG(
        std::cout << "Initial energy: " << E0
                  << "\nInitial total momentum: " << P0
                  << "\nInitial angular momentum: " << L0 << "\n";
        std::cout << "-----------------------------------------\n";
    );

    while (t < t_end) {
        double old_E = system.compute_energy();
        Vec3 old_P = system.compute_total_momentum();
        Vec3 old_L = system.compute_angular_momentum();

        auto a = system.compute_accelerations(q);

        // 1. Kick (полушаг)
        std::vector<Vec3> v_half = v;
        for (size_t i=0; i<v.size(); ++i) v_half[i] += a[i] * (0.5*dt);

        // 2. Drift
        std::vector<Vec3> q_new = q;
        for (size_t i=0; i<q.size(); ++i) q_new[i] += v_half[i] * dt;

        // 3. Новый kick
        auto a_new = system.compute_accelerations(q_new);
        std::vector<Vec3> v_new = v_half;
        for (size_t i=0; i<v.size(); ++i) v_new[i] += a_new[i] * (0.5*dt);

        // Проверка инвариантов
        system.set_positions(q_new); system.set_velocities(v_new);
        double new_E = system.compute_energy();
        Vec3 new_P = system.compute_total_momentum();
        Vec3 new_L = system.compute_angular_momentum();

        bool ok = invariants_ok(old_E, new_E, old_P, new_P, old_L, new_L);

        if (ok) {
            t += dt;
            q = q_new;
            v = v_new;

            // --- ОТЛАДОЧНЫЙ ВЫВОД ---
            DBG(
                if (step % 100 == 0 || t + dt > t_end) {
                    std::cout << "Step: " << std::setw(6) << step
                              << " | t: " << std::setw(9) << t/86400 << " days"
                              << " | dt: " << std::setw(8) << dt
                              << " | E: " << std::scientific << new_E
                              << " | rel dE: " << std::fixed << std::setprecision(4)
                              << (new_E-E0)/E0
                              << " | |dP|: " << std::scientific << (new_P - P0).norm()
                              << " | |dL|: " << std::scientific << (new_L - L0).norm()
                              << "\n";
                }
            );
            step++;
            if (dt < (t_end-t)/2) dt *= 2;
        } else {
            dt /= 2;
            DBG(std::cout << "[ADAPT] dt reduced to " << dt << "\n";);
        }
        if (t+dt > t_end) dt = t_end-t;
    }

    DBG(
        std::cout << "========== [INTEGRATOR END] ===========\n";
        std::cout << "Final time: " << t/86400 << " days\n";
        std::cout << "Final step: " << step << "\n";
        std::cout << "========================================\n\n";
    );
}

bool Integrator::invariants_ok(
    double old_E,
    double new_E,
    const Vec3& old_P,
    const Vec3& new_P,
    const Vec3& old_L,
    const Vec3& new_L
) {
    double de = std::fabs((new_E-old_E)/old_E);
    double dp = (new_P-old_P).norm()/(old_P.norm()+1e-12);
    double dl = (new_L-old_L).norm()/(old_L.norm()+1e-12);
    return (de < epsrel && dp < epsrel && dl < epsrel);
}