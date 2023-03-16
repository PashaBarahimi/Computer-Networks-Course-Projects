#ifndef DATETIME_HPP_INCLUDE
#define DATETIME_HPP_INCLUDE

#include <date.h>

#include <chrono>
#include <string>

class DateTime {
public:
    static date::year_month_day getDate();
    static date::year_month_day getServerDate();
    static std::chrono::system_clock::time_point getDateTime();

    static std::string toStr(date::year_month_day date);
    static std::string toStr(std::chrono::system_clock::time_point dateTime);

    static void setServerDate(date::year_month_day date);
    static void increaseServerDate(int days = 1);

    static bool isValid(const std::string& date);
    static bool parse(const std::string& date, date::year_month_day& res);

    static int compare(const std::string& lhs, const std::string& rhs);

private:
    static date::year_month_day serverDate_;
};

#endif // DATETIME_HPP_INCLUDE
