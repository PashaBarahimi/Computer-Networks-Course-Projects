#ifndef UTILS_HPP_INCLUDE
#define UTILS_HPP_INCLUDE

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace utils {

inline std::string shuffle(const std::string& chars) {
    static auto rng = std::default_random_engine{};
    std::string result = chars;
    std::shuffle(result.begin(), result.end(), rng);
    return result;
}

inline std::vector<uint16_t> reverseMap(const std::string& chars, const std::string& message) {
    std::vector<uint16_t> result;
    result.reserve(message.size());
    for (auto c : message) {
        result.push_back(chars.find(c));
    }
    return result;
}

inline std::vector<std::unordered_map<uint16_t, char>> partitionMappings(const std::string& chars, unsigned partitionCount) {
    std::vector<std::unordered_map<uint16_t, char>> result(partitionCount);
    unsigned partitionCharCount = std::ceil(chars.size() / static_cast<double>(partitionCount));

    for (unsigned i = 0; i < partitionCount; ++i) {
        for (unsigned j = 0; j < partitionCharCount; ++j) {
            unsigned index = i * partitionCharCount + j;
            if (index >= chars.size()) {
                break;
            }
            result[i][index] = chars[index];
        }
    }
    return result;
}

} // namespace utils

#endif // UTILS_HPP_INCLUDE
