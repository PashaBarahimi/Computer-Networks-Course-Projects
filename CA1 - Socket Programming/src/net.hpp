#ifndef NETWORK_HPP_INCLUDE
#define NETWORK_HPP_INCLUDE

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

} // namespace net

#endif // NETWORK_HPP_INCLUDE
