#ifndef UTILS_HPP_INCLUDE
#define UTILS_HPP_INCLUDE

#include <string>
#include <vector>

namespace utils {

std::vector<std::string> split(const std::string& str, char delim);
bool isNumber(const std::string& str);
std::string replicate(const std::string& str, int n);
std::string ljust(const std::string& str, int n);
std::string rjust(const std::string& str, int n);
std::string center(const std::string& str, int n);

} // namespace utils

#endif // UTILS_HPP_INCLUDE
