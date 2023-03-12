#ifndef STRUTILS_HPP_INCLUDE
#define STRUTILS_HPP_INCLUDE

#include <string>
#include <vector>

namespace strutils {

void trimLeft(std::string& str);
void trimRight(std::string& str);
void trim(std::string& str);

std::vector<std::string> split(const std::string& str, char delim);
std::vector<std::string> split(const std::string& str, const std::string& delim);

std::string toupper(const std::string& str);
std::string tolower(const std::string& str);

std::string random(int len);

} // namespace strutils

#endif // STRUTILS_HPP_INCLUDE
