#include <fstream>
#include <json.hpp>
#include <string>

#include "client_cli.hpp"
#include "net.hpp"

const std::string SERVER_CONFIG_FILE = "config/config.json";

std::pair<net::IpAddr, net::Port> readConfigFile() {
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
    auto x = readConfigFile();
    HotelClient client(x.first, x.second);
    if (!client.connect()) {
        std::cout << "Could not connect to the server." << std::endl;
        return 1;
    }
    ClientCLI cli(client);
    cli.run();
    return 0;
}
