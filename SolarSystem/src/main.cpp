#include <iostream>
#include <iomanip>
#include "SolarSystem.h"
#include "Integrator.h"
#include "Utils.h"

// Макрос для отладочного вывода
#ifdef DEBUG
#define DBG(x) do { x; } while(0)
#else
#define DBG(x) do {} while(0)
#endif

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " <data_dir> <target_date: DD.MM.YYYY> <epsrel> <dt_init>\n";
        return 1;
    }
    std::string data_dir = argv[1];
    std::string target_date = argv[2];
    double epsrel = std::stod(argv[3]);
    double dt_init = std::stod(argv[4]);

    SolarSystem system;
    system.load_from_directory(data_dir);

    std::string epoch_date = system.get_epoch_date();
    if (epoch_date.empty()) {
        std::cerr << "Epoch date not found!\n";
        return 2;
    }

    // --- Переводим дату пользователя в секунды с момента эпохи:
    double t_end;
    try {
        t_end = date_to_seconds(target_date, epoch_date);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing date: " << e.what() << std::endl;
        return 3;
    }
    if (t_end < 0) {
        std::cerr << "Target date is before epoch!\n";
        return 4;
    }

    DBG(
        std::cout << "\n==== [SOLAR SYSTEM SOLVER] ====\n";
        std::cout << "Data dir:     " << data_dir << "\n";
        std::cout << "Epoch date:   " << epoch_date << "\n";
        std::cout << "Target date:  " << target_date << " (+" << t_end/86400 << " days)\n";
        std::cout << "epsrel:       " << epsrel << "\n";
        std::cout << "dt_init:      " << dt_init << " sec\n";
        std::cout << "Number of bodies: " << system.get_bodies().size() << "\n";
        std::cout << "===============================\n";
    );

    Integrator integrator(system, epsrel);

    double t_start = 0.0;
    integrator.integrate(t_start, t_end, dt_init);

    // Итоговый вывод позиций и скоростей всех тел:
    auto& bodies = system.get_bodies();
    std::cout << std::fixed << std::setprecision(6);
    for (const auto& body : bodies) {
        std::cout << std::setw(8) << body.get_name() << " | ";
        std::cout << "pos: " << body.get_pos() << " | vel: " << body.get_vel() << std::endl;
    }
    return 0;
}
