#pragma once
#include <vector>
#include "SolarBody.h"
#include "Vec3.h"

class SolarSystem {
    std::vector<SolarBody> bodies;
    std::string epoch_date_;
public:
    void load_from_directory(const std::string& dir); // реализуй в cpp с помощью std::filesystem
    size_t size() const { return bodies.size(); }
    std::vector<SolarBody>& get_bodies() { return bodies; }
    const std::vector<SolarBody>& get_bodies() const { return bodies; }
    // Вектор масс, позиций, скоростей для интегратора:
    std::vector<double> get_masses() const;
    std::vector<Vec3> get_positions() const;
    std::vector<Vec3> get_velocities() const;
    void set_positions(const std::vector<Vec3>&);
    void set_velocities(const std::vector<Vec3>&);
    // Инварианты:
    double compute_energy() const;
    Vec3 compute_total_momentum() const;
    Vec3 compute_angular_momentum() const;
    // Ускорения для интегратора:
    std::vector<Vec3> compute_accelerations(const std::vector<Vec3>& positions) const;
    const std::string& get_epoch_date() const { return epoch_date_; }
};
