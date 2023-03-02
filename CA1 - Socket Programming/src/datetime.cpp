#include "datetime.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

std::chrono::system_clock::time_point DateTime::serverDate_ = std::chrono::system_clock::now();

std::string DateTime::getDateTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

std::string DateTime::getServerDate() {
    auto in_time_t = std::chrono::system_clock::to_time_t(serverDate_);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    return ss.str();
}

void DateTime::setServerDate(const std::string& date) {
    std::istringstream ss(date);
    date::year_month_day ymd;
    ss >> date::parse("%Y-%m-%d", ymd);
    serverDate_ = std::chrono::system_clock::from_time_t(std::chrono::system_clock::to_time_t(date::sys_days(ymd)));
}

void DateTime::increaseServerDate(int days) {
    serverDate_ += date::days(days);
}
