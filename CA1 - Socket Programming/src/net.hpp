#ifndef NETWORK_HPP_INCLUDE
#define NETWORK_HPP_INCLUDE

#include <array>
#include <string>

#include "endian.hpp"

namespace net {

template <class T>
inline T hton(T in) {
    if (byte::endian == byte::Endian::big) {
        return in;
    }
    else {
        return byte::swapOrder(in);
    }
}

template <class T>
inline T ntoh(T in) {
    if (byte::endian == byte::Endian::big) {
        return in;
    }
    else {
        return byte::swapOrder(in);
    }
}

using Port = unsigned short;

class IpAddr {
public:
    IpAddr() = default;
    IpAddr(unsigned short a, unsigned short b, unsigned short c, unsigned short d);
    IpAddr(const std::string& addr);
    IpAddr(const char* addr);
    explicit IpAddr(unsigned int addr);

    unsigned short& operator[](int i);
    unsigned short operator[](int i) const;

    bool operator==(IpAddr rhs) const;
    bool operator!=(IpAddr lhs) const;

    std::string toStr() const;
    unsigned int toInt() const;
    bool fromStr(const std::string& addr);
    void fromInt(unsigned int addr);

    static IpAddr any() { return IpAddr(0, 0, 0, 0); }
    static IpAddr loopback() { return IpAddr(127, 0, 0, 1); }

private:
    std::array<unsigned short, 4> addr_{};
};

} // namespace net

#endif // NETWORK_HPP_INCLUDE
