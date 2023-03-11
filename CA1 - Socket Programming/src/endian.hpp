#include <algorithm>
#include <type_traits>

namespace byte {

enum class Endian {
    little,
    big
};

inline Endian getEndian() {
    const int value = 0x01;
    const unsigned char* lsb = reinterpret_cast<const unsigned char*>(&value);
    return (*lsb == 0x01) ? Endian::little : Endian::big;
}

inline const Endian endian = getEndian();

template <class T,
          std::enable_if_t<!std::is_integral_v<T>>* = nullptr>
inline T swapOrder(T val) {
    unsigned char* ptr = reinterpret_cast<unsigned char*>(&val);
    std::reverse(ptr, ptr + sizeof(T));
    return val;
}

template <class T,
          std::enable_if_t<std::is_integral_v<T>>* = nullptr>
inline T swapOrder(T val) noexcept {
    if constexpr (sizeof(T) == 2) {
        return ((val & 0xFF00) >> 8) |
               ((val & 0x00FF) << 8);
    }
    else if constexpr (sizeof(T) == 4) {
        return ((val & 0xFF000000) >> 24) |
               ((val & 0x00FF0000) >> 8) |
               ((val & 0x0000FF00) << 8) |
               ((val & 0x000000FF) << 24);
    }
    else if constexpr (sizeof(T) == 8) {
        return ((val & 0xFF00000000000000ull) >> 56) |
               ((val & 0x00FF000000000000ull) >> 40) |
               ((val & 0x0000FF0000000000ull) >> 24) |
               ((val & 0x000000FF00000000ull) >> 8) |
               ((val & 0x00000000FF000000ull) << 8) |
               ((val & 0x0000000000FF0000ull) << 24) |
               ((val & 0x000000000000FF00ull) << 40) |
               ((val & 0x00000000000000FFull) << 56);
    }
    else {
        return val;
    }
}

} // namespace byte
