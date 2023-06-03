#include "network.hpp"

Network::~Network() {
    for (auto& node : nodes_) {
        delete node;
    }
}

Node* Network::operator[](const std::string& name) const {
    auto it = nodeMap_.find(name);
    if (it == nodeMap_.end()) {
        return nullptr;
    }
    return nodes_[it->second];
}

Node* Network::operator[](const int index) const {
    if (index < 0 || static_cast<unsigned>(index) >= nodes_.size()) {
        return nullptr;
    }
    return nodes_[index];
}

bool Network::doesNodeExist(const std::string& name) const {
    return nodeMap_.find(name) != nodeMap_.end();
}

bool Network::addNode(const std::string& name) {
    if (doesNodeExist(name)) {
        return false;
    }
    nodes_.push_back(new Node(name));
    nodeMap_[name] = nodes_.size() - 1;
    return true;
}

bool Network::addEdge(const std::string& source, const std::string& destination, int weight) {
    if (!doesNodeExist(source) || !doesNodeExist(destination)) {
        return false;
    }
    bool res = (*this)[source]->addEdge((*this)[destination], weight);
    if (!res) {
        return false;
    }
    return (*this)[destination]->addEdge((*this)[source], weight);
}

bool Network::removeEdge(const std::string& source, const std::string& destination) {
    if (!doesNodeExist(source) || !doesNodeExist(destination)) {
        return false;
    }
    bool res = (*this)[source]->removeEdge((*this)[destination]);
    if (!res) {
        return false;
    }
    return (*this)[destination]->removeEdge((*this)[source]);
}

void Network::modifyEdge(const std::string& source, const std::string& destination, int weight) {
    if (!doesNodeExist(source) || !doesNodeExist(destination)) {
        return;
    }
    (*this)[source]->modifyEdge((*this)[destination], weight);
    (*this)[destination]->modifyEdge((*this)[source], weight);
}

const std::vector<Node*>& Network::getNodes() const {
    return nodes_;
}

const std::vector<std::vector<int>> Network::getAdjacencyMatrix() const {
    std::vector<std::vector<int>> matrix(nodes_.size(), std::vector<int>(nodes_.size(), -1));
    for (unsigned i = 0; i < nodes_.size(); ++i) {
        matrix[i][i] = 0;
        for (auto& edge : nodes_[i]->getEdges()) {
            int dest = nodeMap_.find(edge->destination->getName())->second;
            matrix[i][dest] = edge->weight;
        }
    }
    return matrix;
}
