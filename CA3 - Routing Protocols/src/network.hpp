#ifndef NETWORK_HPP_INCLUDE
#define NETWORK_HPP_INCLUDE

#include <string>
#include <unordered_map>
#include <vector>

#include "node.hpp"

struct DistanceVector {
    Node* destination;
    int distance;

    bool operator<(const DistanceVector& other) const {
        return distance < other.distance;
    }

    bool operator>(const DistanceVector& other) const {
        return distance > other.distance;
    }
};

class Network {
public:
    Network() = default;
    ~Network();

    Node* operator[](const std::string& name) const;
    Node* operator[](const int index) const;

    bool doesNodeExist(const std::string& name) const;
    bool addNode(const std::string& name);
    bool addEdge(const std::string& source, const std::string& destination, int weight);
    bool removeEdge(const std::string& source, const std::string& destination);
    void modifyEdge(const std::string& source, const std::string& destination, int weight);

    const std::vector<Node*>& getNodes() const;
    const int getNodeIndex(const std::string& name) const;
    const std::vector<std::vector<int>> getAdjacencyMatrix() const;

    std::vector<std::vector<int>> getLsrpTable(Node* src);

    const std::unordered_map<std::string, std::vector<std::string>> getShortestPaths() const;

private:
    std::vector<Node*> nodes_;
    std::unordered_map<std::string, int> nodeMap_;
    std::unordered_map<Node*, Node*> parent_;
};

#endif // NETWORK_HPP_INCLUDE
