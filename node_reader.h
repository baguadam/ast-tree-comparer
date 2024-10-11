#ifndef _NODE_READER_H_

#define _NODE_READER_H_

#include <string>
#include <vector>

struct Node {
    std::string name;
    std::string value;
    std::vector<Node*> children;
    Node* parent;
};

class NodeReader {
public:
    Node* readASTDump(const std::string&);
};

#endif