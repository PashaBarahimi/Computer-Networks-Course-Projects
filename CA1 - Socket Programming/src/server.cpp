#include <fstream>
#include <iostream>
#include <json.hpp>

#include "datetime.hpp"
#include "hotel_manager.hpp"
#include "net.hpp"

const std::string SERVER_CONFIG_FILE = "config/config.json";

std::pair<net::IpAddr, net::Port> getServerConfig() {
    std::ifstream config(SERVER_CONFIG_FILE);
    if (!config.is_open()) {
        throw std::runtime_error("Cannot open config file");
    }
    nlohmann::json j;
    try {
        config >> j;
        std::string hostname = j["hostname"];
        int port = j["port"];
        return {hostname, port};
    }
    catch (const nlohmann::json::parse_error& e) {
        std::cout << "Failed to parse config file" << std::endl;
    }
    catch (const nlohmann::json::out_of_range& e) {
        std::cout << "Invalid config" << std::endl;
    }
    catch (const nlohmann::json::type_error& e) {
        std::cout << "Invalid config" << std::endl;
    }
    return {net::IpAddr::loopback(), 8000};
}

int main() {
    while (true) {
        std::cout << "Please enter the date in the format of YYYY-MM-DD: ";
        std::string dateStr;
        std::cin >> dateStr;

        date::year_month_day date;
        if (!DateTime::parse(dateStr, date)) {
            std::cout << "Invalid date." << std::endl;
        }
        else {
            DateTime::setServerDate(date);
            std::cout << "The server date is set to: "
                      << DateTime::toStr(DateTime::getServerDate()) << std::endl;
            break;
        }
    }

    auto config = getServerConfig();
    try {
        HotelManager manager(config.first, config.second);
        manager.run();
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
