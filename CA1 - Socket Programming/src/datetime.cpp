#include "datetime.hpp"

#include <iomanip>
#include <sstream>

date::year_month_day DateTime::serverDate_(getDate());

date::year_month_day DateTime::getDate() {
    return date::floor<date::days>(std::chrono::system_clock::now());
}

std::chrono::system_clock::time_point DateTime::getDateTime() {
    return std::chrono::system_clock::now();
}

date::year_month_day DateTime::getServerDate() {
    return serverDate_;
}

std::string DateTime::toStr(std::chrono::system_clock::time_point dateTime) {
    auto in_time_t = std::chrono::system_clock::to_time_t(dateTime);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%F %T");
    return ss.str();
}

std::string DateTime::toStr(date::year_month_day date) {
    return date::format("%F", date);
}

void DateTime::setServerDate(date::year_month_day date) {
    serverDate_ = date;
}

void DateTime::increaseServerDate(int days) {
    serverDate_ = date::sys_days(serverDate_) + date::days(days);
}

bool DateTime::isValid(const std::string& date) {
    date::year_month_day ymd;
    return parse(date, ymd);
}

bool DateTime::parse(const std::string& date, date::year_month_day& res) {
    date::year_month_day ymd;
    std::istringstream ss(date);
    ss >> date::parse("%F", ymd);
    if (ss.fail()) {
        return false;
    }
    std::string str;
    if (ss >> str) {
        return false;
    }
    res = ymd;
    return true;
}

int DateTime::compare(const std::string& lhs, const std::string& rhs) {
    date::year_month_day ymd1, ymd2;
    if (!parse(lhs, ymd1) || !parse(rhs, ymd2)) {
        return 0;
    }
    if (ymd1 == ymd2) {
        return 0;
    }
    return ymd1 < ymd2 ? -1 : 1;
}
