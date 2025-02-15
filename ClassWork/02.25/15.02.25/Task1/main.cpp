#include <iostream>  
#include <quadmath.h>
#include <vector>
#include <string>
#include <cmath>
#include <filesystem>
#include <fstream>

#define G_CONSTANT 6.67430e-20

class SolarBody {
    double mass;
    double qx, qy, qz;
    double px, py, pz;
    std::string name;
public:
    SolarBody(double mass, double qx, double qy, double qz, double px, double py, double pz, std::string name){
        this->mass = mass;
        this->qx = qx;
        this->qy = qy;
        this->qz = qz;
        this->px = px;
        this->py = py;
        this->pz = pz;
        this->name = name;
    }

    inline double get_qx() const {return qx;}
    inline double get_qy() const {return qy;}
    inline double get_qz() const {return qz;}
    inline double get_px() const {return px;}
    inline double get_py() const {return py;}
    inline double get_pz() const {return pz;}
    inline double get_mass() const {return mass;}

    SolarBody(){mass = 1; qx = 0; qy = 0; qz = 0; px = 0; py = 0; pz = 0; name = "";}

    double inline give_speed_x() const {return px/mass;}
    double inline give_speed_y() const {return py/mass;}
    double inline give_speed_z() const {return pz/mass;}

    void inline set_mass(double mass){this->mass = mass;}
    void inline set_qx(double qx){this->qx = qx;}
    void inline set_qy(double qy){this->qy = qy;}
    void inline set_qz(double qz){this->qz = qz;}
    void inline set_px(double px){this->px = px;}
    void inline set_py(double py){this->py = py;}
    void inline set_pz(double pz){this->pz = pz;}
};


class SolarSystem {
    std::vector<SolarBody> body_init;

public:
    SolarSystem(){;}

    void body_init_push_back(const SolarBody &body){
        body_init.push_back(body);
    }

    double H_init(){
        int n = body_init.size();

        double H_kin = 0;
        for(int i = 0; i < n; i ++){
            double p_i_x = body_init[i].get_px();
            double p_i_y = body_init[i].get_py();
            double p_i_z = body_init[i].get_pz();
            double m_i = body_init[i].get_mass();

            double H_i_kin = 0.5 / m_i * (p_i_x * p_i_x + p_i_y * p_i_y + p_i_z * p_i_z);
            H_kin += H_i_kin;
        }

        double H_pot = 0;
        
        // for(SolarBody i : body_init){} // range based for loop
        // for (std :: vector<SolarBody> :: iterator i = body_init.begin(); i < body_init.end(); i ++){} // iterator based for loop
        for (int i = 0; i < n; i ++){
            double q_i_x = body_init[i].get_qx();
            double q_i_y = body_init[i].get_qy();
            double q_i_z = body_init[i].get_qz();
            double m_i = body_init[i].get_mass();

            for (int j = i + 1; j < n; j ++ ){
                double q_j_x = body_init[j].get_qx();
                double q_j_y = body_init[j].get_qy();
                double q_j_z = body_init[j].get_qz();
                double m_j = body_init[j].get_mass();

                double r_ij = sqrt(std::pow((q_i_x - q_j_x), 2)  + std::pow((q_i_y - q_j_y) , 2) + std::pow((q_i_z - q_j_z), 2));

                H_pot += m_i * m_j / r_ij;
            }
        }

        return H_kin - G_CONSTANT * H_pot;
    };
};

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;
    std::string directoryPath = argv[1];

    SolarSystem system;

    for (const auto & entry : fs::directory_iterator(directoryPath)){
        std::cout << entry.path() << std::endl;
        if (!entry.is_regular_file()){
            std::cout << "Not a Regular file" << std::endl;
            return 1;
        }

        std::ifstream file(entry.path());
        if (!file.is_open()){
            std::cout << "File not found" << std::endl;
            return 1;
        }

        double mass, qx, qy, qz, vx, vy, vz;
        std :: string name;
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("Revised") != std::string::npos){
                // std::cout << line << std::endl;
                std::istringstream iss(line);
                std::string token;

                iss >> token;
                iss >> token;
                iss >> token;
                iss >> token;

                iss >> name;
                // std::cout << "Name: " << name << std::endl;
            }

            if (line.find("Mass (kg) = ") != std::string::npos){
                std::istringstream iss(line);

                std::string token;
                iss >> token; 
                iss >> token; 
                iss >> token;

                iss >> mass;
                // std::cout << "M " << mass << std::endl;
            } 

            if (line.find("X =") != std::string::npos){
                // std::cout << line << std::endl;
                size_t pos1 = line.find("X =") + 3;
                size_t pos2 = line.find("Y =") + 3;             
                size_t pos3 = line.find("Z =") + 3;

                std::string numberStr = line.substr(pos1);
                qx = std::stod(numberStr);

                numberStr = line.substr(pos2);
                qy = std::stod(numberStr);

                numberStr = line.substr(pos3);
                qz = std::stod(numberStr);

                // std::cout << "Qx: " << qx << std::endl;
                // std::cout << "Qy: " << qy << std::endl;
                // std::cout << "Qz: " << qz << std::endl;
            }

            if (line.find("VX=") != std::string::npos){
                // std::cout << line << std::endl;
                size_t pos1 = line.find("VX=") + 3;
                size_t pos2 = line.find("VY=") + 3;             
                size_t pos3 = line.find("VZ=") + 3;

                std::string numberStr = line.substr(pos1);
                vx = std::stod(numberStr);

                numberStr = line.substr(pos2);
                vy = std::stod(numberStr);

                numberStr = line.substr(pos3);
                vz = std::stod(numberStr);

                // std::cout << "vx: " << vx << std::endl;
                // std::cout << "vy: " << vy << std::endl;
                // std::cout << "vz: " << vz << std::endl;
            }
        }

        SolarBody body(mass, qx, qy, qz, mass * vx, mass * vy, mass * vz, name);
        // std :: cout << "Speed x: " << body.give_speed_x() << std::endl;
        system.body_init_push_back(body);
    }
    double H = system.H_init();
    std::cout << "H: " << H << std::endl;
    return 0;  
}
