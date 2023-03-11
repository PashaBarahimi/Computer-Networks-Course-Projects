#include "net.hpp"

#include <regex>
#include <sstream>

namespace net {

IpAddr::IpAddr(unsigned short a, unsigned short b, unsigned short c, unsigned short d)
    : addr_{a, b, c, d} {}

IpAddr::IpAddr(const std::string& addr) {
    fromStr(addr);
}

IpAddr::IpAddr(const char* addr)
    : IpAddr(std::string(addr)) {}

IpAddr::IpAddr(unsigned int addr) {
    fromInt(addr);
}

unsigned short& IpAddr::operator[](int i) { return addr_[i]; }
unsigned short IpAddr::operator[](int i) const { return addr_[i]; }

bool IpAddr::operator==(IpAddr rhs) const { return addr_ == rhs.addr_; }
bool IpAddr::operator!=(IpAddr lhs) const { return addr_ != lhs.addr_; }

std::string IpAddr::toStr() const {
    std::ostringstream sstr;
    sstr << addr_[0] << '.' << addr_[1] << '.' << addr_[2] << '.' << addr_[3];
    return sstr.str();
}

unsigned int IpAddr::toInt() const {
    unsigned int res = 0;
    for (int i = 0; i < 4; ++i) {
        res |= addr_[i] << ((3 - i) * 8);
    }
    return res;
}

bool IpAddr::fromStr(const std::string& addr) {
    static std::regex re(R"((\d+)\.(\d+)\.(\d+)\.(\d+))");
    std::smatch match;
    if (!std::regex_match(addr, match, re)) {
        return false;
    }
    IpAddr res;
    for (int i = 0; i < 4; ++i) {
        int octet = std::stoi(match[i + 1]);
        if (octet > 255) {
            return false;
        }
        res[i] = octet;
    }
    *this = res;
    return true;
}

void IpAddr::fromInt(unsigned int addr) {
    for (int i = 0; i < 4; ++i) {
        addr_[i] = (addr >> ((3 - i) * 8)) & 0xFF;
    }
}

} // namespace net
