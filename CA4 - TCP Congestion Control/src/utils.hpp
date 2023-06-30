#ifndef UTILS_HPP_INCLUDE
#define UTILS_HPP_INCLUDE

#include <chrono>
#include <random>
#include <cmath>

namespace utils {

inline int randInt(int first, int last) {
    thread_local static const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    thread_local static std::mt19937 gen(static_cast<std::mt19937::result_type>(seed));
    std::uniform_int_distribution<int> dist(first, last);
    return dist(gen);
}

inline double expProb(int x, int first, int last) {
    auto ex = std::pow(10 * last, static_cast<double>(x - first) / (last - first)) - 1;
    return ex / (10 * last - 1);
}

} // namespace utils

#endif // UTILS_HPP_INCLUDE
