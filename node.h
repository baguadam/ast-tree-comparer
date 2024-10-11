#ifndef _NODE_H_

#define _NODE_H_

#include <string>
#include <vector>

struct Node {
    std::string name;
    std::string value;
    std::vector<Node*> children;
    Node* parent;
};

#endif 