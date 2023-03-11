#include "datetime.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

date::year_month_day DateTime::serverDate_(date::floor<date::days>(std::chrono::system_clock::now()));

std::string DateTime::getDateTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%F %T");
    return ss.str();
}

std::string DateTime::getServerDate() {
    return date::format("%F", serverDate_);
}

bool DateTime::setServerDate(const std::string& date) {
    std::istringstream ss(date);
    ss >> date::parse("%F", serverDate_);
    return !ss.fail();
}

void DateTime::increaseServerDate(int days) {
    serverDate_ = date::sys_days(serverDate_) + date::days(days);
}

bool DateTime::isValid(const std::string& date) {
    date::year_month_day ymd;
    std::istringstream ss(date);
    ss >> date::parse("%F", ymd);
    return !ss.fail();
}
