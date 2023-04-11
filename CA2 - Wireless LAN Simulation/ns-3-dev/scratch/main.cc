#include "network.hpp"
#include "ns3/core-module.h"

int main(int argc, char* argv[]) {
    bool verbose = consts::VERBOSE_DEFAULT;
    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse(argc, argv);

    Network net(verbose);
    net.Simulate();

    return 0;
}
