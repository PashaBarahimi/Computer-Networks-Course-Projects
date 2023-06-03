#include "command_line_interface.hpp"
#include "network.hpp"

int main() {
    Network network;
    CommandLineInterface cli(network);
    cli.run();
    return 0;
}
