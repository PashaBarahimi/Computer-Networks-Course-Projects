#ifndef NODE_HPP_INCLUDE
#define NODE_HPP_INCLUDE

#include <string>
#include <vector>

class Node;

struct Edge {
    Node* destination;
    int weight;
};

class Node {
public:
    Node(std::string name);
    ~Node();

    const Edge* operator[](const std::string& destination) const;

    bool addEdge(Node* destination, int weight);
    bool removeEdge(Node* destination);
    void modifyEdge(Node* destination, int weight);

    const std::string& getName() const;
    const std::vector<Edge*>& getEdges() const;

private:
    std::string name_;
    std::vector<Edge*> edges_;

    std::vector<Edge*>::iterator findDestNode(Node* destination);
};

#endif // NODE_HPP_INCLUDE
