#include <fstream>
#include <iostream>
#include <json.hpp>

#include "datetime.hpp"
#include "hotel_manager.hpp"

const std::string SERVER_CONFIG_FILE = "bin/config/config.json";

std::pair<std::string, int> getServerConfig() {
    std::ifstream config(SERVER_CONFIG_FILE);
    if (!config.is_open()) {
        throw std::runtime_error("Cannot open config file");
    }
    nlohmann::json j;
    config >> j;
    return {j["hostname"], j["port"]};
}

int main() {
    std::cout << "Please enter the date in the format of YYYY-MM-DD: ";
    std::string date;
    std::cin >> date;
    if (DateTime::setServerDate(date)) {
        std::cout << "The server date is set to " << DateTime::getServerDate() << std::endl;
    }
    else {
        std::cout << "Invalid date" << std::endl;
    }
    auto config = getServerConfig();
    HotelManager manager(config.first, config.second);
    manager.run();
    return 0;
}
