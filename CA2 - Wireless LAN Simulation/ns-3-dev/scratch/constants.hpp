#ifndef CONSTANTS_HPP_INCLUDE
#define CONSTANTS_HPP_INCLUDE

#include <array>
#include <cstdint>
#include <string>

namespace consts {

// header
constexpr uint32_t CLIENT_HEADER_LENGTH = 8;
constexpr uint32_t MAPPER_HEADER_LENGTH = 1;

// client
constexpr uint16_t CLIENT_PORT = 1102;

// master
constexpr uint16_t MASTER_PORT = 1102;

// mappers
constexpr uint32_t MAPPERS_COUNT = 3;
const std::array<uint16_t, MAPPERS_COUNT> MAPPER_PORTS = {1102, 1102, 1102};

// network
const std::string SSID = "ns-3-ssid";
const char* BASE_ADDRESS = "10.1.3.0";
const char* NET_MASK = "255.255.255.0";

// simulation
constexpr double ERROR = 0.000001;
constexpr double DURATION = 10.0;
constexpr double SIMULATION_DURATION = DURATION;
constexpr double MONITOR_TIME_INTERVAL = 1.0;
constexpr double TIMEOUT = 0.1;
constexpr double BURSTY_DATA_SEND_INTERVAL = 0.001;
constexpr double DELTA_X = 5.0;
constexpr double DELTA_Y = 10.0;
constexpr uint32_t GRID_WIDTH = 3;
constexpr bool VERBOSE_DEFAULT = true;

// data
constexpr bool RANDOM_DATA = false;
constexpr bool BURSTY_DATA = false;
constexpr bool SHUFFLE_MAPPINGS = false;
const std::string VALID_CHARACTERS = RANDOM_DATA ? "abcdefghijklmnopqrstuvwxyz"
                                                 : "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                   "abcdefghijklmnopqrstuvwxyz"
                                                   "0123456789"
                                                   " _-.,:;!?()";
const std::string MESSAGE =
    "This is a sample text that is written to test the "
    "network which is being simulated using ns-3.35. It"
    " is worth noting that this text will be repeated. "
    "WELCOME TO PROJECT MISASHA (Misagh and Pasha)";

} // namespace consts

#endif // CONSTANTS_HPP_INCLUDE
