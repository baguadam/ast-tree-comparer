#ifndef _NODE_READER_H_

#define _NODE_READER_H_

#include <memory>
#include <string>
#include <vector>

struct Node {
    std::string name;
    std::string value;
    std::vector<std::unique_ptr<Node>> children;
    Node* parent;
};

class NodeReader {
public:
    std::unique_ptr<Node> readASTDump(const std::string&);
};

#endif