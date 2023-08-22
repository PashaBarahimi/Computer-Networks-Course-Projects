#include <ostream>

enum class Color {
    BLK = 30, // black
    RED = 31, // red
    GRN = 32, // green
    YEL = 33, // yellow
    BLU = 34, // blue
    MAG = 35, // magenta
    CYN = 36, // cyan
    WHT = 37, // white
    DEF = 39, // default
    RST = 0,  // reset
};

inline std::ostream& operator<<(std::ostream& os, Color clr) {
    return os << "\x1B[" << static_cast<int>(clr) << 'm';
}
