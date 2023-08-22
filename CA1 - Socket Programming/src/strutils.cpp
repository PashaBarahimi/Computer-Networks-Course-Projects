#include "strutils.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <random>
#include <sstream>

namespace strutils {

void trimLeft(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
                  return !std::isspace(ch);
              }));
}

void trimRight(std::string& str) {
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                  return !std::isspace(ch);
              }).base(),
              str.end());
}

void trim(std::string& str) {
    trimRight(str);
    trimLeft(str);
}

std::vector<std::string> split(const std::string& str, char delim) {
    std::istringstream sstr(str);
    std::string item;
    std::vector<std::string> result;
    while (std::getline(sstr, item, delim)) {
        result.push_back(std::move(item));
    }
    return result;
}

std::vector<std::string> split(const std::string& str, const std::string& delim) {
    std::string::size_type startPos = 0;
    std::string::size_type endPos;

    std::vector<std::string> result;
    while ((endPos = str.find(delim, startPos)) != std::string::npos) {
        result.push_back(str.substr(startPos, endPos - startPos));
        startPos = endPos + delim.size();
    }
    result.push_back(str.substr(startPos));

    return result;
}

std::string toupper(const std::string& str) {
    std::string res(str);
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return res;
}

std::string tolower(const std::string& str) {
    std::string res(str);
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return res;
}

std::string random(int len) {
    static const char charset[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789-_";

    thread_local static const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    thread_local static std::mt19937 gen(static_cast<std::mt19937::result_type>(seed));
    thread_local static std::uniform_int_distribution<int> dist(0, sizeof(charset) - 2);

    std::string str(len, '0');
    std::generate_n(str.begin(), len, []() {
        return charset[dist(gen)];
    });

    return str;
}

} // namespace strutils
