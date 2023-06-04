#include "command_line_interface.hpp"

#include <chrono>
#include <iostream>

#include "utils.hpp"

CommandLineInterface::CommandLineInterface(Network& network) : network_(network) {
    commands_ = {
        {"help", std::bind(&CommandLineInterface::help, this, std::placeholders::_1)},
        {"topology", std::bind(&CommandLineInterface::topology, this, std::placeholders::_1)},
        {"show", std::bind(&CommandLineInterface::show, this, std::placeholders::_1)},
        {"modify", std::bind(&CommandLineInterface::modify, this, std::placeholders::_1)},
        {"remove", std::bind(&CommandLineInterface::remove, this, std::placeholders::_1)},
        {"lsrp", std::bind(&CommandLineInterface::lsrp, this, std::placeholders::_1)},
        // {"dvrp", std::bind(&CommandLineInterface::dvrp, this, std::placeholders::_1)},
    };
}

void CommandLineInterface::run() {
    std::string line;
    std::vector<std::string> args;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line.empty()) {
            continue;
        }

        args = utils::split(line, ' ');
        std::string command = args[0];
        args.erase(args.begin());

        if (command == "exit") {
            break;
        }
        if (commands_.find(command) == commands_.end()) {
            std::cout << "Unknown command: " << command << std::endl;
            continue;
        }

        try {
            std::cout << commands_.at(command)(args) << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

std::string CommandLineInterface::help(const std::vector<std::string>& args) {
    static const std::string help =
        "Available commands:"
        "\n  help - show this message"
        "\n  topology [<s>-<d>-<w>] - create a topology"
        "\n  show - show the current topology"
        "\n  modify <s>-<d>-<w> - modify the weight of an edge"
        "\n  remove <s>-<d> - remove an edge"
        "\n  lsrp <s> - run the link state routing protocol"
        "\n  dvrp <s> - run the distance vector routing protocol"
        "\n  exit - exit the program\n";
    return help;
}

std::string CommandLineInterface::topology(const std::vector<std::string>& args) {
    static const std::string usage = "Usage: topology [<s>-<d>-<w>]";
    if (args.empty()) {
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
        maxLen = std::max<int>(maxLen, node->getName().size());
    }
    for (const auto& adjRow : adjMatrix) {
        for (auto adjCell : adjRow) {
            maxLen = std::max<int>(maxLen, std::to_string(adjCell).size());
        }
    }

    std::string result;
    result += utils::replicate(' ', maxLen) + " | ";
    for (auto node : nodes) {
        result += utils::rjust(node->getName(), maxLen) + ' ';
    }
    int lineLen = result.size();
    result += '\n';
    result += utils::replicate('-', lineLen) + '\n';
    for (unsigned i = 0; i < adjMatrix.size(); ++i) {
        result += utils::rjust(nodes[i]->getName(), maxLen) + " | ";
        for (unsigned j = 0; j < adjMatrix[i].size(); ++j) {
            result += utils::rjust(std::to_string(adjMatrix[i][j]), maxLen) + ' ';
        }
        result += '\n';
    }
    return result;
}

std::string CommandLineInterface::modify(const std::vector<std::string>& args) {
    static const std::string usage = "Usage: modify <s>-<d>-<w>";
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
    static const std::string usage = "Usage: remove <s>-<d>";
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

std::string CommandLineInterface::lsrp(const std::vector<std::string>& args) {
    static std::string usage = "Usage: lsrp <s>";
    if (args.size() > 1) {
        return usage;
    }
    std::vector<Node*> sources;
    if (args.size() == 0) {
        sources = network_.getNodes();
    }
    else {
        if (!network_.doesNodeExist(args[0])) {
            return "Source node does not exist";
        }
        sources.push_back(network_[args[0]]);
    }
    auto start = std::chrono::high_resolution_clock::now();
    std::string result = "";
    for (auto source : sources) {
        result += "Source: " + source->getName() + "\n";
        auto table = network_.getLsrpTable(source);
        result += getLsrpInfo(table);
        result += getLsrpShortestPaths(source->getName(), network_.getShortestPaths(), table.back());
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
    result += "Time elapsed: " + std::to_string(duration) + "ms";
    return result;
}

std::string CommandLineInterface::getLsrpInfo(const std::vector<std::vector<int>>& table) const {
    std::string result = "";
    auto nodes = network_.getNodes();
    int maxLen = 0;
    for (auto node : nodes) {
        maxLen = std::max<int>(maxLen, node->getName().size());
    }
    for (auto& iter : table) {
        for (auto& cell : iter) {
            maxLen = std::max<int>(maxLen, std::to_string(cell).size());
        }
    }
    for (unsigned i = 0; i < table.size(); ++i) {
        result += "Iter " + std::to_string(i + 1) + ":\n";
        std::string dests = "Dest ";
        for (unsigned j = 0; j < table[i].size(); ++j) {
            dests += utils::rjust(nodes[j]->getName(), maxLen) + " | ";
        }
        int lineLen = dests.size();
        result += dests + "\n";
        result += "Cost ";
        for (unsigned j = 0; j < table[i].size(); ++j) {
            result += utils::rjust(std::to_string(table[i][j]), maxLen) + " | ";
        }
        result += "\n";
        result += utils::replicate('-', lineLen) + "\n";
    }
    return result;
}

std::string CommandLineInterface::getLsrpShortestPaths(const std::string& src,
                                                       const std::unordered_map<std::string, std::vector<std::string>>& paths,
                                                       const std::vector<int>& costs) const {
    std::vector<std::vector<std::string>> table(paths.size() + 1, std::vector<std::string>(3));
    table[0] = {"Path [s]->[d]", "Min-Cost", "Shortest-Path"};
    auto nodes = network_.getNodes();
    for (unsigned i = 0; i < nodes.size(); ++i) {
        if (nodes[i]->getName() == src) {
            continue;
        }
        table[i + 1][0] = src + "->" + nodes[i]->getName();
        table[i + 1][1] = std::to_string(costs[i]);
        table[i + 1][2] = utils::join(paths.at(nodes[i]->getName()), "->");
    }

    int srcIndex = network_.getNodeIndex(src);
    table.erase(table.begin() + srcIndex + 1);
    int maxLen = 0;
    for (auto& row : table) {
        for (auto& cell : row) {
            maxLen = std::max<int>(maxLen, cell.size());
        }
    }
    std::string result = "";
    for (auto& row : table) {
        for (auto& cell : row) {
            result += utils::center(cell, maxLen) + " | ";
        }
        result += "\n";
    }
    return result;
}
