#include "node.hpp"

#include <algorithm>

Node::Node(const std::string& name) : name_(name) {}

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

bool Node::addEdge(Node* destination, int weight) {
    if (std::find_if(edges_.begin(), edges_.end(), [&destination](const auto& edge) {
            return edge->destination->getName() == destination->getName();
        }) != edges_.end()) {
        return false;
    }
    edges_.push_back(new Edge{destination, weight});
    return true;
}

bool Node::removeEdge(Node* destination) {
    auto it = std::find_if(edges_.begin(), edges_.end(), [&destination](const auto& edge) {
        return edge->destination->getName() == destination->getName();
    });
    if (it == edges_.end()) {
        return false;
    }
    delete *it;
    edges_.erase(it);
    return true;
}

void Node::modifyEdge(Node* destination, int weight) {
    auto it = std::find_if(edges_.begin(), edges_.end(), [&destination](const auto& edge) {
        return edge->destination->getName() == destination->getName();
    });
    if (it == edges_.end()) {
        addEdge(destination, weight);
        return;
    }
    (*it)->weight = weight;
}

const std::vector<Edge*>& Node::getEdges() const {
    return edges_;
}

const std::string& Node::getName() const {
    return name_;
}
