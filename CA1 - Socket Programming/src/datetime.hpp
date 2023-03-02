#ifndef DATETIME_HPP_INCLUDE
#define DATETIME_HPP_INCLUDE

#include <string>

#include "date.h"

class DateTime {
public:
    static std::string getDateTime();
    static std::string getServerDate();
    static void setServerDate(const std::string& date);
    static void increaseServerDate(int days = 1);

private:
    static std::chrono::system_clock::time_point serverDate_;
};

#endif // DATETIME_HPP_INCLUDE
