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
    Node* src = (*this)[source];
    Node* dest = (*this)[destination];
    if (src == nullptr || dest == nullptr) {
        return false;
    }
    return src->addEdge(dest, weight) && dest->addEdge(src, weight);
}

bool Network::removeEdge(const std::string& source, const std::string& destination) {
    Node* src = (*this)[source];
    Node* dest = (*this)[destination];
    if (src == nullptr || dest == nullptr) {
        return false;
    }
    return src->removeEdge(dest) && dest->removeEdge(src);
}

void Network::modifyEdge(const std::string& source, const std::string& destination, int weight) {
    Node* src = (*this)[source];
    Node* dest = (*this)[destination];
    if (src == nullptr || dest == nullptr) {
        return;
    }
    src->modifyEdge(dest, weight);
    dest->modifyEdge(src, weight);
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
