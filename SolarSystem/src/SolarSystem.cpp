#include "SolarSystem.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <cstdio>

#define G_CONSTANT 6.67430e-20

#ifdef DEBUG
#define DBG(x) do { x; } while(0)
#else
#define DBG(x) do {} while(0)
#endif

void SolarSystem::load_from_directory(const std::string& dir) {
    namespace fs = std::filesystem;
    epoch_date_ = "";
    bodies.clear();
    bool epoch_found = false;

    DBG(
        std::cout << "\n========== [SolarSystem::load_from_directory] ==========\n";
        std::cout << "Начинаем парсинг каталога: " << dir << std::endl;
    );
    DBG(
        int file_count = std::distance(fs::directory_iterator(dir), fs::directory_iterator{});
        std::cout << "[DBG] Файлов найдено: " << file_count << std::endl;
    );

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) {
            DBG(std::cout << "[DBG] Пропущен не-файл: " << entry.path() << std::endl;);
            continue;
        }
        DBG(std::cout << "\n[DBG] Обработка файла: " << entry.path() << std::endl;);

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            DBG(std::cout << "[DBG][WARN] Не удалось открыть файл: " << entry.path() << std::endl;);
            continue;
        }

        double m = 0, qx = 0, qy = 0, qz = 0, vx = 0, vy = 0, vz = 0;
        std::string name, line;
        while (getline(file, line)) {
            if (line.find("Revised") != std::string::npos) {
                std::istringstream iss(line); std::string token;
                for (int k=0; k<4; ++k) iss >> token;
                iss >> name;
                DBG(std::cout << "[DBG] Имя тела найдено: " << name << std::endl;);
            }
            if (line.find("Mass (kg) = ") != std::string::npos) {
                std::istringstream iss(line); std::string token;
                for (int k=0; k<3; ++k) iss >> token;
                iss >> m;
                DBG(std::cout << "[DBG] Масса: " << m << std::endl;);
            }
            if (line.find("X =") != std::string::npos) {
                size_t pos1 = line.find("X =") + 3, pos2 = line.find("Y =") + 3, pos3 = line.find("Z =") + 3;
                qx = std::stod(line.substr(pos1));
                qy = std::stod(line.substr(pos2));
                qz = std::stod(line.substr(pos3));
                DBG(std::cout << "[DBG] Координаты: X=" << qx << " Y=" << qy << " Z=" << qz << std::endl;);
            }
            if (line.find("VX=") != std::string::npos) {
                size_t pos1 = line.find("VX=") + 3, pos2 = line.find("VY=") + 3, pos3 = line.find("VZ=") + 3;
                vx = std::stod(line.substr(pos1));
                vy = std::stod(line.substr(pos2));
                vz = std::stod(line.substr(pos3));
                DBG(std::cout << "[DBG] Скорости: VX=" << vx << " VY=" << vy << " VZ=" << vz << std::endl;);
            }
            if (!epoch_found && line.find("A.D.") != std::string::npos) {
                // Пример: "A.D. 2025-Feb-14 00:00:00.0000 TDB"
                size_t pos = line.find("A.D.") + 5;
                std::string date_str = line.substr(pos, 11); // "2025-Feb-14"
                int year = 0, day = 0;
                char month_str[4] = {0};
                int parsed = sscanf(date_str.c_str(), "%d-%3s-%d", &year, month_str, &day);
                if (parsed == 3) {
                    static std::map<std::string, std::string> month_map = {
                        {"Jan","01"},{"Feb","02"},{"Mar","03"},{"Apr","04"},{"May","05"},{"Jun","06"},
                        {"Jul","07"},{"Aug","08"},{"Sep","09"},{"Oct","10"},{"Nov","11"},{"Dec","12"}
                    };
                    std::string month = month_map[std::string(month_str)];
                    char date_buf[11];
                    snprintf(date_buf, sizeof(date_buf), "%02d.%s.%d", day, month.c_str(), year);
                    epoch_date_ = std::string(date_buf);
                    epoch_found = true;
                    DBG(std::cout << "[DBG] Дата эпохи (epoch_date) определена: " << epoch_date_ << std::endl;);
                } else {
                    DBG(std::cout << "[DBG][WARN] Не удалось распарсить дату эпохи! Исходная строка: " << line << std::endl;);
                }
            }
        }
        bodies.emplace_back(m, Vec3(qx, qy, qz), Vec3(vx, vy, vz), name);
        DBG(
            std::cout << "[DBG] Добавлено тело: "
                      << name << " | m=" << m
                      << " | pos=(" << qx << ", " << qy << ", " << qz << ")"
                      << " | vel=(" << vx << ", " << vy << ", " << vz << ")"
                      << std::endl;
        );
    }
    DBG(
        std::cout << "\n[DBG] Всего тел загружено: " << bodies.size() << std::endl;
        if (epoch_date_.empty())
            std::cout << "[DBG][ERR] Epoch date не определена ни в одном файле!" << std::endl;
        else
            std::cout << "Epoch date (эпоха): " << epoch_date_ << std::endl;
        std::cout << "========================================================\n";
    );
}


std::vector<double> SolarSystem::get_masses() const {
    std::vector<double> m;
    for (const auto& b : bodies) m.push_back(b.get_mass());
    return m;
}
std::vector<Vec3> SolarSystem::get_positions() const {
    std::vector<Vec3> q;
    for (const auto& b : bodies) q.push_back(b.get_pos());
    return q;
}
std::vector<Vec3> SolarSystem::get_velocities() const {
    std::vector<Vec3> v;
    for (const auto& b : bodies) v.push_back(b.get_vel());
    return v;
}
void SolarSystem::set_positions(const std::vector<Vec3>& q) {
    for (size_t i=0; i<bodies.size(); ++i) bodies[i].set_pos(q[i]);
}
void SolarSystem::set_velocities(const std::vector<Vec3>& v) {
    for (size_t i=0; i<bodies.size(); ++i) bodies[i].set_vel(v[i]);
}

std::vector<Vec3> SolarSystem::compute_accelerations(const std::vector<Vec3>& q) const {
    std::vector<Vec3> acc(bodies.size(), Vec3());
    for (size_t i=0; i<bodies.size(); ++i) {
        for (size_t j=0; j<bodies.size(); ++j) if (i!=j) {
            Vec3 dr = q[i] - q[j];
            double dist = dr.norm();
            if (dist > 0)
                acc[i] += dr * (-G_CONSTANT * bodies[j].get_mass() / (dist*dist*dist));
        }
    }
    return acc;
}

double SolarSystem::compute_energy() const {
    std::vector<Vec3> q = get_positions();
    std::vector<Vec3> v = get_velocities();
    std::vector<double> m = get_masses();
    double E_kin = 0, E_pot = 0;
    for (size_t i=0; i<m.size(); ++i)
        E_kin += 0.5 * m[i] * Vec3::dot(v[i], v[i]);
    for (size_t i=0; i<m.size(); ++i)
        for (size_t j=i+1; j<m.size(); ++j) {
            Vec3 dr = q[i]-q[j]; double dist = dr.norm();
            E_pot -= G_CONSTANT * m[i]*m[j]/dist;
        }
    return E_kin + E_pot;
}
Vec3 SolarSystem::compute_total_momentum() const {
    std::vector<Vec3> v = get_velocities();
    std::vector<double> m = get_masses();
    Vec3 P;
    for (size_t i=0; i<m.size(); ++i)
        P += v[i] * m[i];
    return P;
}
Vec3 SolarSystem::compute_angular_momentum() const {
    std::vector<Vec3> q = get_positions();
    std::vector<Vec3> v = get_velocities();
    std::vector<double> m = get_masses();
    Vec3 L;
    for (size_t i=0; i<m.size(); ++i)
        L += Vec3::cross(q[i], v[i]) * m[i];
    return L;
}
