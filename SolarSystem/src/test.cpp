#include <iostream>
#include "SolarSystem.h"
#include "Integrator.h"
#include "Utils.h"

#ifdef DEBUG
#define DBG(x) do { x; } while(0)
#else
#define DBG(x) do {} while(0)
#endif

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <data_dir> [PlanetName]\n";
        return 1;
    }
    std::string data_dir = argv[1];
    std::string planet_name = "Earth";
    if (argc >= 3) planet_name = argv[2];

    SolarSystem system;
    system.load_from_directory(data_dir);

    std::string epoch_date = system.get_epoch_date();
    if (epoch_date.empty()) {
        std::cerr << "Epoch date not found!\n";
        return 2;
    }

    int d, m, y;
    if (sscanf(epoch_date.c_str(), "%d.%d.%d", &d, &m, &y) != 3) {
        std::cerr << "Cannot parse epoch_date: " << epoch_date << std::endl;
        return 3;
    }
    y += 1;
    if (y < 0 || y > 9999) {
        std::cerr << "Year out of range: " << y << "\n";
        return 1;
    }
    char target_date[11];
    snprintf(target_date, sizeof(target_date),"%02d.%02d.%04d", d, m, y);
    Vec3 start_pos, start_vel;
    bool found = false;
    for (const auto& body : system.get_bodies()) {
        if (body.get_name() == planet_name) {
            start_pos = body.get_pos();
            start_vel = body.get_vel();
            found = true;
            break;
        }
    }
    if (!found) {
        std::cerr << "Planet '" << planet_name << "' not found in system!\n";
        return 42;
    }

    DBG(
        std::cout << "=== [DEBUG] Start test for planet: " << planet_name << std::endl;
        std::cout << "Start epoch: " << epoch_date << std::endl;
        std::cout << "Start position: " << start_pos << std::endl;
        std::cout << "Start velocity: " << start_vel << std::endl;
    );

    double epsrel = 1e-9;
    double dt_init = 86400;
    double t_end = date_to_seconds(target_date, epoch_date);

    double energy_start = system.compute_energy();

    Integrator integrator(system, epsrel);
    integrator.integrate(0.0, t_end, dt_init);

    Vec3 end_pos, end_vel;
    found = false;
    for (const auto& body : system.get_bodies()) {
        if (body.get_name() == planet_name) {
            end_pos = body.get_pos();
            end_vel = body.get_vel();
            found = true;
            break;
        }
    }
    if (!found) {
        std::cerr << "Planet '" << planet_name << "' not found after integration!\n";
        return 43;
    }
    double distance = (end_pos - start_pos).norm();

    DBG(
        std::cout << "End epoch: " << target_date << std::endl;
        std::cout << "End position: " << end_pos << std::endl;
        std::cout << "End velocity: " << end_vel << std::endl;
    );

    std::cout << planet_name << " offset after 1 year: " << distance << " km" << std::endl;

    double energy_end = system.compute_energy();
    std::cout << "Energy at start: " << energy_start << std::endl;
    std::cout << "Energy at end:   " << energy_end << std::endl;
    std::cout << "Rel. energy diff: " << (energy_end - energy_start) / energy_start << std::endl;

    return 0;
}
