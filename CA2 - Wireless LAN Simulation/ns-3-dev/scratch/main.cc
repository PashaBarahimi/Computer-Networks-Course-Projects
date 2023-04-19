#include "constants.hpp"
#include "network.hpp"
#include "ns3/core-module.h"
#include "utils.hpp"

int main(int argc, char* argv[]) {
    bool verbose = consts::VERBOSE_DEFAULT;
    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse(argc, argv);

    std::string chars = consts::VALID_CHARACTERS;
    if (consts::SHUFFLE_MAPPINGS)
        chars = utils::shuffle(chars);
    auto clientSendData = utils::reverseMap(chars, consts::MESSAGE);
    auto mappings = utils::partitionMappings(chars, consts::MAPPERS_COUNT);

    Network net(verbose, clientSendData, mappings);
    net.Simulate();

    return 0;
}
