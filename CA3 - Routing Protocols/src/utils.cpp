#include "utils.hpp"

#include <sstream>

namespace utils {

std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::istringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        if (token.empty()) {
            continue;
        }
        tokens.push_back(token);
    }
    return tokens;
}

bool isNumber(const std::string& str) {
    for (char c : str) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

std::string replicate(const std::string& str, int n) {
    std::string result;
    for (int i = 0; i < n; ++i) {
        result += str;
    }
    return result;
}

std::string ljust(const std::string& str, int n) {
    return str + replicate(" ", n - str.size());
}

std::string rjust(const std::string& str, int n) {
    return replicate(" ", n - str.size()) + str;
}

std::string center(const std::string& str, int n) {
    int left = (n - str.size()) / 2;
    int right = n - str.size() - left;
    return replicate(" ", left) + str + replicate(" ", right);
}

} // namespace utils
