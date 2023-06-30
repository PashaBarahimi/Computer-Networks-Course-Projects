#include <iostream>

#include "bbr.hpp"
#include "new_reno.hpp"
#include "reno.hpp"

int main() {
    std::vector<int> data(100000);
    for (unsigned i = 0; i < data.size(); ++i) {
        data[i] = i;
    }

    Reno reno(data);
    reno.run(std::cout);
    return 0;
}
