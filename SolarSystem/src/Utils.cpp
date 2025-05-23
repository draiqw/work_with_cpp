#include "Utils.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

double date_to_seconds(const std::string& target, const std::string& epoch) {
    std::tm tm_target = {}, tm_epoch = {};
    std::istringstream ss_target(target), ss_epoch(epoch);

    ss_target >> std::get_time(&tm_target, "%d.%m.%Y");
    ss_epoch >> std::get_time(&tm_epoch, "%d.%m.%Y");
    if (ss_target.fail() || ss_epoch.fail()) throw std::runtime_error("Error parsing date");

    time_t t_target = mktime(&tm_target);
    time_t t_epoch = mktime(&tm_epoch);
    return difftime(t_target, t_epoch); // в секундах
}
