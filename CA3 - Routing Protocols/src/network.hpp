#ifndef NETWORK_HPP_INCLUDE
#define NETWORK_HPP_INCLUDE

#include <string>
#include <unordered_map>
#include <vector>

#include "node.hpp"

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
    const std::vector<std::vector<int>> getAdjacencyMatrix() const;

private:
    std::vector<Node*> nodes_;
    std::unordered_map<std::string, int> nodeMap_;
};

#endif // NETWORK_HPP_INCLUDE
