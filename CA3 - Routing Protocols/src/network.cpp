#include "network.hpp"

#include <algorithm>
#include <queue>

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

const int Network::getNodeIndex(const std::string& name) const {
    auto it = nodeMap_.find(name);
    if (it == nodeMap_.end()) {
        return -1;
    }
    return it->second;
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

std::vector<std::vector<int>> Network::getLsrpTable(Node* src) {
    std::vector<std::vector<int>> iterTable;
    std::unordered_map<Node*, int> distance;
    std::unordered_map<Node*, Node*> parent;
    for (auto& node : nodes_) {
        distance[node] = -1;
    }
    distance[src] = 0;
    parent[src] = nullptr;
    std::priority_queue<DistanceVector, std::vector<DistanceVector>, std::greater<DistanceVector>> pq;
    pq.push({src, 0});
    iterTable.push_back(std::vector<int>(nodes_.size(), -1));
    iterTable.back()[nodeMap_.find(src->getName())->second] = 0;
    while (!pq.empty()) {
        auto top = pq.top();
        pq.pop();
        Node* node = top.destination;
        int dist = top.distance;
        if (dist > distance[node]) {
            continue;
        }
        iterTable.push_back(std::vector<int>(iterTable.back()));
        for (auto& edge : node->getEdges()) {
            int newDist = dist + edge->weight;
            if (distance[edge->destination] == -1 || newDist < distance[edge->destination]) {
                distance[edge->destination] = newDist;
                pq.push({edge->destination, newDist});
                parent[edge->destination] = node;
                iterTable.back()[nodeMap_.find(edge->destination->getName())->second] = newDist;
            }
        }
    }
    parent_ = parent;
    iterTable.erase(iterTable.begin());
    if (iterTable.size() > 1 && iterTable.back() == iterTable[iterTable.size() - 2]) {
        iterTable.pop_back();
    }
    return iterTable;
}

const std::unordered_map<std::string, std::vector<std::string>> Network::getShortestPaths() const {
    std::unordered_map<std::string, std::vector<std::string>> paths;
    for (auto& node : nodes_) {
        std::vector<std::string> path;
        Node* curr = node;
        while (curr != nullptr) {
            path.push_back(curr->getName());
            curr = parent_.find(curr)->second;
        }
        std::reverse(path.begin(), path.end());
        paths[node->getName()] = path;
    }
    return paths;
}
