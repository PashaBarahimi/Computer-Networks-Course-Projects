#ifndef NODE_HPP_INCLUDE
#define NODE_HPP_INCLUDE

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Node;

struct Edge {
    Node* destination;
    int weight;
};

class Node {
public:
    Node(const std::string& name);
    ~Node();

    const Edge* operator[](const std::string& destination) const;

    bool addEdge(Node* destination, int weight);
    bool removeEdge(const std::string& destination);
    const std::vector<Edge*>& getEdges() const;
    const std::string& getName() const;

private:
    std::string name_;
    std::vector<Edge*> edges_;
};

#endif // NODE_HPP_INCLUDE
