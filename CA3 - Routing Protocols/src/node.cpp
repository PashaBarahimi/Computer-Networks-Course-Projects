#include "node.hpp"

Node::Node(const std::string& name) : name_(name) {}

bool Node::addEdge(Node* destination, int weight) {
    if (edges_.find(destination->getName()) != edges_.end()) {
        return false;
    }
    edges_.emplace(destination->getName(), std::make_pair(destination, weight));
    return true;
}

const std::unordered_map<std::string, Adjacency>& Node::getEdges() const {
    return edges_;
}

const std::string& Node::getName() const {
    return name_;
}
