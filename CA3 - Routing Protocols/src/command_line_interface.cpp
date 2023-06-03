#include "command_line_interface.hpp"

#include <iostream>

#include "utils.hpp"

CommandLineInterface::CommandLineInterface(Network& network) : network_(network) {
    commands_ = {
        {"help", std::bind(&CommandLineInterface::help, this, std::placeholders::_1)},
        {"topology", std::bind(&CommandLineInterface::topology, this, std::placeholders::_1)},
        {"show", std::bind(&CommandLineInterface::show, this, std::placeholders::_1)},
        {"modify", std::bind(&CommandLineInterface::modify, this, std::placeholders::_1)},
        {"remove", std::bind(&CommandLineInterface::remove, this, std::placeholders::_1)},
        // {"lsrp", std::bind(&CommandLineInterface::lsrp, this, std::placeholders::_1)},
        // {"dvrp", std::bind(&CommandLineInterface::dvrp, this, std::placeholders::_1)},
    };
}

void CommandLineInterface::run() {
    std::string command;
    std::vector<std::string> args;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        args = utils::split(command, ' ');
        if (args[0] == "exit") {
            break;
        }
        if (commands_.find(args[0]) == commands_.end()) {
            std::cout << "Unknown command: " << args[0] << std::endl;
            continue;
        }
        std::string command = args[0];
        args.erase(args.begin());
        std::cout << commands_[command](args) << std::endl;
    }
}

std::string CommandLineInterface::help(const std::vector<std::string>& args) {
    std::string help =
        "Available commands:\n"
        "  help - show this message\n"
        "  topology [<s>-<d>-<w>] - create a topology\n"
        "  show - show the current topology\n"
        "  modify <s>-<d>-<w> - modify the weight of an edge\n"
        "  remove <s>-<d> - remove an edge\n"
        "  lsrp <s> - run the link state routing protocol\n"
        "  dvrp <s> - run the distance vector routing protocol\n"
        "  exit - exit the program\n";
    return help;
}

std::string CommandLineInterface::topology(const std::vector<std::string>& args) {
    static std::string usage = "Usage: topology [<s>-<d>-<w>]";
    if (args.size() == 0) {
        return usage;
    }
    for (const std::string& arg : args) {
        std::vector<std::string> edge = utils::split(arg, '-');
        if (edge.size() != 3) {
            return usage;
        }
        if (!utils::isNumber(edge[2])) {
            return "Weight must be a number";
        }
        int weight = std::stoi(edge[2]);
        if (weight <= 0) {
            return "Weight must be positive";
        }
        if (edge[0] == edge[1]) {
            return "Self-loop is not allowed";
        }
        if (!network_.doesNodeExist(edge[0])) {
            network_.addNode(edge[0]);
        }
        if (!network_.doesNodeExist(edge[1])) {
            network_.addNode(edge[1]);
        }
        network_.addEdge(edge[0], edge[1], weight);
    }
    return "OK";
}

std::string CommandLineInterface::show(const std::vector<std::string>& args) {
    auto nodes = network_.getNodes();
    auto adjMatrix = network_.getAdjacencyMatrix();
    int maxLen = 0;
    for (auto node : nodes) {
        maxLen = std::max(maxLen, static_cast<int>(node->getName().size()));
    }
    for (auto adjRow : adjMatrix) {
        for (auto adjCell : adjRow) {
            maxLen = std::max(maxLen, static_cast<int>(std::to_string(adjCell).size()));
        }
    }
    std::string result = "";
    result += utils::replicate(" ", maxLen) + " | ";
    for (auto node : nodes) {
        result += utils::rjust(node->getName(), maxLen) + " ";
    }
    int lineLen = result.size();
    result += "\n";
    result += utils::replicate("-", lineLen) + "\n";
    for (unsigned i = 0; i < adjMatrix.size(); ++i) {
        result += utils::rjust(nodes[i]->getName(), maxLen) + " | ";
        for (unsigned j = 0; j < adjMatrix[i].size(); ++j) {
            result += utils::rjust(std::to_string(adjMatrix[i][j]), maxLen) + " ";
        }
        result += "\n";
    }
    return result;
}

std::string CommandLineInterface::modify(const std::vector<std::string>& args) {
    static std::string usage = "Usage: modify <s>-<d>-<w>";
    if (args.size() != 1) {
        return usage;
    }
    std::vector<std::string> edge = utils::split(args[0], '-');
    if (edge.size() != 3) {
        return usage;
    }
    if (!utils::isNumber(edge[2])) {
        return "Weight must be a number";
    }
    int weight = std::stoi(edge[2]);
    if (weight <= 0) {
        return "Weight must be positive";
    }
    if (edge[0] == edge[1]) {
        return "Self-loop is not allowed";
    }
    if (!network_.doesNodeExist(edge[0])) {
        network_.addNode(edge[0]);
    }
    if (!network_.doesNodeExist(edge[1])) {
        network_.addNode(edge[1]);
    }
    network_.modifyEdge(edge[0], edge[1], weight);
    return "OK";
}

std::string CommandLineInterface::remove(const std::vector<std::string>& args) {
    static std::string usage = "Usage: remove <s>-<d>";
    if (args.size() != 1) {
        return usage;
    }
    std::vector<std::string> edge = utils::split(args[0], '-');
    if (edge.size() != 2) {
        return usage;
    }
    if (edge[0] == edge[1]) {
        return "Self-loop is not allowed";
    }
    if (!network_.doesNodeExist(edge[0])) {
        return "Source node does not exist";
    }
    if (!network_.doesNodeExist(edge[1])) {
        return "Destination node does not exist";
    }
    if (!network_.removeEdge(edge[0], edge[1])) {
        return "Edge does not exist";
    }
    return "OK";
}
