#include <iostream>  
#include <quadmath.h>
#include <vector>
#include <string>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <tuple>

#define G_CONSTANT 2.95912208286e-4 // AU^3 / (day^2 * M_sun)

class Vector3 {
public:
    double x, y, z;

    Vector3(double x = 0.0, double y = 0.0, double z = 0.0)
        : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(double scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3 operator/(double scalar) const {
        return Vector3(x / scalar, y / scalar, z / scalar);
    }

    double norm() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    void print(const std::string& label = "") const {
        std::cout << label << "(" << x << ", " << y << ", " << z << ")" << std::endl;
    }
};

class SolarBody {
    double mass;
    double qx, qy, qz;
    double px, py, pz;
    std::string name;
public:
    SolarBody(
        double mass,
        double qx,
        double qy,
        double qz,
        double px,
        double py,
        double pz, 
        std::string name
    ) {
        this->mass = mass;
        this->qx = qx;
        this->qy = qy;
        this->qz = qz;
        this->px = px;
        this->py = py;
        this->pz = pz;
        this->name = name;
    }

    SolarBody() {
        mass = 1;
        qx = 0;
        qy = 0;
        qz = 0;
        px = 0;
        py = 0;
        pz = 0;
        name = "";
    }

    double get_qx() const { return qx; }
    double get_qy() const { return qy; }
    double get_qz() const { return qz; }

    double get_px() const { return px; }
    double get_py() const { return py; }
    double get_pz() const { return pz; }

    double get_mass() const { return mass; }
    std::string get_name() const { return name; }

    void set_mass(double mass) { this->mass = mass; }
    void set_qx(double qx) { this->qx = qx; }
    void set_qy(double qy) { this->qy = qy; }
    void set_qz(double qz) { this->qz = qz; }
    void set_px(double px) { this->px = px; }
    void set_py(double py) { this->py = py; }
    void set_pz(double pz) { this->pz = pz; }

    double give_speed_x() const { return px / mass; }
    double give_speed_y() const { return py / mass; }
    double give_speed_z() const { return pz / mass; }

    double distance_to(const SolarBody& other) const {
        double dx = qx - other.qx;
        double dy = qy - other.qy;
        double dz = qz - other.qz;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

class SolarSystem {
    std::vector <SolarBody> body_init;
    std::vector <double> q, v, v0, q0;
    double t, t0;
public:
    SolarSystem(){;}

    SolarBody* getBodyByName(const std::string & searchName) {
        for (auto & body : body_init) {
            if (body.get_name() == searchName) {
                return &body;
            }
        }
        return nullptr;
    };

    void body_init_push_back(const SolarBody &body){
        body_init.push_back(body);
    }

    double H_init(bool verbose = false) {
        int n = body_init.size();
    
        double H_kin = 0;
        for (int i = 0; i < n; i++) {
            double p_i_x = body_init[i].get_px();
            double p_i_y = body_init[i].get_py();
            double p_i_z = body_init[i].get_pz();
            double m_i = body_init[i].get_mass();
    
            double H_i_kin = (0.5 / m_i) * (p_i_x * p_i_x + p_i_y * p_i_y + p_i_z * p_i_z);
            H_kin += H_i_kin;
        }
    
        double H_pot = 0;
        for (int i = 0; i < n; i++) {
            double q_i_x = body_init[i].get_qx();
            double q_i_y = body_init[i].get_qy();
            double q_i_z = body_init[i].get_qz();
            double m_i = body_init[i].get_mass();
    
            for (int j = i + 1; j < n; j++) {
                double q_j_x = body_init[j].get_qx();
                double q_j_y = body_init[j].get_qy();
                double q_j_z = body_init[j].get_qz();
                double m_j = body_init[j].get_mass();
    
                double r_ij = std::sqrt(
                    std::pow(q_i_x - q_j_x, 2) +
                    std::pow(q_i_y - q_j_y, 2) +
                    std::pow(q_i_z - q_j_z, 2)
                );
    
                H_pot += m_i * m_j / r_ij;
            }
        }
    
        if (verbose) {
            std::cout << "H_kin: " << H_kin << std::endl;
            std::cout << "H_pot: " << G_CONSTANT * H_pot << std::endl;
        }
    
        return H_kin - G_CONSTANT * H_pot;
    }
    
    std::tuple <double, double, double> P_init(){
        double P_x = 0.0;
        double P_y = 0.0;
        double P_z = 0.0;
        int n = body_init.size();
        for (int i = 0; i < n; i ++){
            double p_i_x = body_init[i].get_px();
            double p_i_y = body_init[i].get_py();
            double p_i_z = body_init[i].get_pz();

            P_x += p_i_x;
            P_y += p_i_y;
            P_z += p_i_z;
        }

        return std::make_tuple(P_x, P_y, P_z);
    };

    std::tuple <double, double, double> L_init(){
        double L_x = 0.0;
        double L_y = 0.0;
        double L_z = 0.0;
        int n = body_init.size();
        for (int i = 0; i < n; i++){
            double q_i_x = body_init[i].get_qx();
            double q_i_y = body_init[i].get_qy();
            double q_i_z = body_init[i].get_qz();
            double p_i_x = body_init[i].get_px();
            double p_i_y = body_init[i].get_py();
            double p_i_z = body_init[i].get_pz();
    
            L_x += (q_i_y * p_i_z - q_i_z * p_i_y);
            L_y += (q_i_z * p_i_x - q_i_x * p_i_z);
            L_z += (q_i_x * p_i_y - q_i_y * p_i_x);
        }
        return std::make_tuple(L_x, L_y, L_z);
    };

    std::vector <double> f(){
        int n = body_init.size();

        std::vector<double> forces;
        forces.reserve(3 * n);

        for (int i = 0; i < n; i ++){
            Vector3 f_i(0.0, 0.0, 0.0);

            Vector3 q_i(
                body_init[i].get_qx(),
                body_init[i].get_qy(),
                body_init[i].get_qz()
            );
            

            for (int j = 0; j < i; j ++){
                Vector3 q_j(
                    body_init[j].get_qx(),
                    body_init[j].get_qy(),
                    body_init[j].get_qz()
                );

                Vector3 diff = q_i - q_j;
                double norm = diff.norm();
                double m_j = body_init[j].get_mass();
                
                diff = diff *  m_j;
                diff = diff / (norm * norm * norm);
                f_i = f_i + diff;
            }

            for (int j = (i + 1); j < n; j ++){
                Vector3 q_j(
                    body_init[j].get_qx(),
                    body_init[j].get_qy(),
                    body_init[j].get_qz()
                );

                Vector3 diff = q_j - q_i;
                double norm = diff.norm();
                double m_j = body_init[j].get_mass();
                
                diff = diff * m_j;
                diff = diff / (norm * norm * norm);
                f_i = f_i + diff;
            }
            forces.insert(forces.end(), {f_i.x, f_i.y, f_i.z});
        }
        return forces;
    }

    std::vector <double> v0_init(){
        int n = body_init.size();
        std::vector <double> v0;
        for (int i = 0; i < n; i ++){
            double v_i_x = body_init[i].give_speed_x();
            double v_i_y = body_init[i].give_speed_y();
            double v_i_z = body_init[i].give_speed_z();

            v0.push_back(v_i_x);
            v0.push_back(v_i_y);
            v0.push_back(v_i_z);
        }
        return v0;
    };

    std::vector <double> q0_init(){
        int n = body_init.size();
        std::vector <double> q0;
        for (int i = 0; i < n; i ++){
            double q_i_x = body_init[i].get_qx();
            double q_i_y = body_init[i].get_qy();
            double q_i_z = body_init[i].get_qz();

            q0.push_back(q_i_x);
            q0.push_back(q_i_y);
            q0.push_back(q_i_z);
        }
        return q0;
    }

    void print_f(){
        std::vector<double> f_vec = f();
        std::cout << "Вектор F(3*n):" << std::endl;
        for (size_t i = 0; i < f_vec.size(); i += 3) {
            std::cout << "f_v" << (i / 3) << "x =  " << f_vec[i] << std::endl;
            std::cout << "f_v" << (i / 3) << "y =  " << f_vec[i + 1] << std::endl;
            std::cout << "f_v" << (i / 3) << "z =  " << f_vec[i + 2] << std::endl;
        }
    }
    
    void print_v(){
        std::vector<double> v_vec = v0_init();
        std::cout << "Вектор V(3*n):" << std::endl;
        for (size_t i = 0; i < v_vec.size(); i += 3) {
            std::cout << "V" << (i / 3) << "x =  " << v_vec[i] << std::endl;
            std::cout << "V" << (i / 3) << "y =  " << v_vec[i + 1] << std::endl;
            std::cout << "V" << (i / 3) << "z =  " << v_vec[i + 2] << std::endl;
        }
    }

    void print_H(bool verbose = false) {
        double H = H_init(verbose);
        std::cout << "H: " << H << std::endl;
    }
    
    void print_P(){
        auto [Px, Py, Pz] = P_init();
        std::cout << "P: (" << Px << ", " << Py << ", " << Pz << ")" << std::endl;
    }
    
    void print_L(){
        auto [Lx, Ly, Lz] = L_init();
        std::cout << "L: (" << Lx << ", " << Ly << ", " << Lz << ")" << std::endl;
    }
};

SolarBody parseBodyFromFile(const std::filesystem::path& path, bool verbose = false) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Ошибка: не удалось открыть файл " + path.string());
    }

    double mass = 1, qx = 0, qy = 0, qz = 0, vx = 0, vy = 0, vz = 0;
    std::string name = "Unknown";
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("Revised") != std::string::npos) {
            std::istringstream iss(line);
            std::string token;
            for (int i = 0; i < 4; ++i) iss >> token;
            iss >> name;
            if (verbose) std::cout << "Name: " << name << std::endl;
        }

        if (line.find("Mass (kg) = ") != std::string::npos) {
            std::istringstream iss(line);
            std::string token;
            for (int i = 0; i < 3; ++i) iss >> token;
            iss >> mass;
            mass = mass / 1988410e+24; // В солнечные массы
            if (verbose) std::cout << "M M0: " << mass << std::endl;
        }

        if (line.find("X =") != std::string::npos) {
            size_t pos1 = line.find("X =") + 3;
            size_t pos2 = line.find("Y =") + 3;
            size_t pos3 = line.find("Z =") + 3;

            qx = std::stod(line.substr(pos1)) * 1000 / 149597870700.0;
            qy = std::stod(line.substr(pos2)) * 1000 / 149597870700.0;
            qz = std::stod(line.substr(pos3)) * 1000 / 149597870700.0;

            if (verbose) {
                std::cout << "Qx Au: " << qx << std::endl;
                std::cout << "Qy Au: " << qy << std::endl;
                std::cout << "Qz Au: " << qz << std::endl;
            }
        }

        if (line.find("VX=") != std::string::npos) {
            size_t pos1 = line.find("VX=") + 3;
            size_t pos2 = line.find("VY=") + 3;
            size_t pos3 = line.find("VZ=") + 3;

            vx = std::stod(line.substr(pos1)) * 1000 / 149597870700.0 * 86400;
            vy = std::stod(line.substr(pos2)) * 1000 / 149597870700.0 * 86400;
            vz = std::stod(line.substr(pos3)) * 1000 / 149597870700.0 * 86400;

            if (verbose) {
                std::cout << "vx Au/day: " << vx << std::endl;
                std::cout << "vy Au/day: " << vy << std::endl;
                std::cout << "vz Au/day: " << vz << std::endl;
            }
        }
    }

    return SolarBody(
        mass, qx, qy, qz,
        mass * vx, mass * vy, mass * vz,
        name
    );
}

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;
    std::string directoryPath = argv[1];

    SolarSystem system;

    bool verbose = false;

    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (!entry.is_regular_file()) continue;
    
        try {
            SolarBody body = parseBodyFromFile(entry.path(), verbose);
            system.body_init_push_back(body);
            std::cout << "Загружен: " << body.get_name() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Ошибка при чтении " << entry.path() << ": " << e.what() << std::endl;
        }
    }
    
    system.print_H(false);
    system.print_P();
    system.print_L();


    SolarBody* earth = system.getBodyByName("Earth");
    SolarBody* sun = system.getBodyByName("Sun");
    double distance = earth->distance_to(*sun);
    std::cout << "Distance Earth-Sun: " << distance << std::endl;

    
    // system.print_f();
    // system.print_v();
    return 0;  
}
