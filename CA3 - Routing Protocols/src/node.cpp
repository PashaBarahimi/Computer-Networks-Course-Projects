#include "node.hpp"

#include <algorithm>

Node::Node(std::string name) : name_(std::move(name)) {}

Node::~Node() {
    for (auto& edge : edges_) {
        delete edge;
    }
}

const Edge* Node::operator[](const std::string& destination) const {
    auto it = std::find_if(edges_.begin(), edges_.end(), [&destination](const auto& edge) {
        return edge->destination->getName() == destination;
    });
    if (it == edges_.end()) {
        return nullptr;
    }
    return *it;
}

std::vector<Edge*>::iterator Node::findDestNode(Node* destination) {
    return std::find_if(edges_.begin(), edges_.end(), [&destination](const auto& edge) {
        return edge->destination->getName() == destination->getName();
    });
}

bool Node::addEdge(Node* destination, int weight) {
    auto it = findDestNode(destination);
    if (it != edges_.end()) {
        return false;
    }
    edges_.push_back(new Edge{destination, weight});
    return true;
}

bool Node::removeEdge(Node* destination) {
    auto it = findDestNode(destination);
    if (it == edges_.end()) {
        return false;
    }
    delete *it;
    edges_.erase(it);
    return true;
}

void Node::modifyEdge(Node* destination, int weight) {
    auto it = findDestNode(destination);
    if (it == edges_.end()) {
        addEdge(destination, weight);
        return;
    }
    (*it)->weight = weight;
}

const std::string& Node::getName() const {
    return name_;
}

const std::vector<Edge*>& Node::getEdges() const {
    return edges_;
}
