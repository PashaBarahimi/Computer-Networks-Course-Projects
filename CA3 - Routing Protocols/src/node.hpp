#ifndef NODE_HPP_INCLUDE
#define NODE_HPP_INCLUDE

#include <string>
#include <unordered_map>
#include <utility>

class Node;
using Adjacency = std::pair<Node*, int>; // destination, weight

class Node {
public:
    Node(const std::string& name);
    ~Node() = default;

    bool addEdge(Node* destination, int weight);
    const std::unordered_map<std::string, Adjacency>& getEdges() const;
    const std::string& getName() const;

private:
    std::string name_;
    std::unordered_map<std::string, Adjacency> edges_;
};

#endif // NODE_HPP_INCLUDE
