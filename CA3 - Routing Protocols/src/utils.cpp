#include "utils.hpp"

#include <algorithm>
#include <cctype>
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
        tokens.push_back(std::move(token));
    }
    return tokens;
}

bool isNumber(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    auto begin = str.begin();
    if (str[0] == '-') {
        if (str.size() == 1) {
            return false;
        }
        ++begin;
    }
    return std::all_of(begin, str.end(), [](unsigned char c) {
        return std::isdigit(c);
    });
}

std::string replicate(char c, int n) {
    if (n < 0) {
        return {};
    }
    return std::string(n, c);
}

std::string replicate(const std::string& str, int n) {
    if (n < 0) {
        return {};
    }
    std::string result;
    result.reserve(n * str.size());
    for (int i = 0; i < n; ++i) {
        result += str;
    }
    return result;
}

std::string ljust(const std::string& str, int n) {
    return str + replicate(' ', n - str.size());
}

std::string rjust(const std::string& str, int n) {
    return replicate(' ', n - str.size()) + str;
}

std::string center(const std::string& str, int n) {
    int left = (n - str.size()) / 2;
    int right = n - str.size() - left;
    return replicate(' ', left) + str + replicate(' ', right);
}

} // namespace utils
