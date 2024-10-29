#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <queue>
#include <stack>
#include "./headers/tree.h"
#include "./headers/utils.h"

/*
Description:
    Constructs a tree from the given file.
*/
Tree::Tree(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fileName);
    }
    
    root = buildTree(file);
    if (!root) {
        throw std::runtime_error("Failed to build tree from file: " + fileName);
    }
    
    createNodeMap();
}

/*
Description:
    Deletes the tree.
*/
Tree::~Tree() {
    if (root) {
        deleteTree(root);
        root = nullptr;
    }
}

/*
Description:
    Returns the root node of the tree.
*/
Node* Tree::getRoot() const {
    return root;
}

/*
Description:
    Returns the pair of the node based on the key.
*/
const Node* Tree::getDeclNode(const std::string& nodeKey) const {
    auto it = declNodeMap.find(nodeKey);
    if (it == declNodeMap.end()) {
        throw std::out_of_range("Node key not found in the declaration node map: " + nodeKey);
    }
    return it->second.first;
}

/*
Description:
    Returns the statement nodes based on the key of the declaration.
*/
const std::unordered_map<std::string, std::pair<Node*, bool>> Tree::getStmtNodes(const std::string& nodeKey) const {
    std::unordered_map<std::string, std::pair<Node*, bool>> stmtNodes;
    auto range = stmtNodeMultiMap.equal_range(nodeKey);
    for (auto it = range.first; it != range.second; ++it) {
        std::string stmtNodeKey = Utils::getKey(it->second, false);
        stmtNodes[stmtNodeKey] = std::pair<Node*, bool>(it->second, false); // marking the node as not processed
    }
    return stmtNodes;
}

/*
Description:
    Returns the node map of the tree.
*/
const std::unordered_map<std::string, std::pair<Node*, bool>>& Tree::getDeclNodeMap() const {
    return declNodeMap;
}

/*
    Returns the statement node map of the tree.
*/
const std::unordered_multimap<std::string, Node*>& Tree::getStmtNodeMultiMap() const {
    return stmtNodeMultiMap;
}

/*
Description:
    Marks the subtree as processed in the tree.
*/
void Tree::markSubTreeAsProcessed(Node* node) {
    if (!node) {
        return;
    }

    std::stack<Node*> stack;
    stack.push(node);

    while (!stack.empty()) {
        Node* current = stack.top();
        stack.pop();

        std::string nodeKey = Utils::getKey(current, current->type == "Declaration");
        if (!nodeKey.empty()) {
            markDeclNodeAsProcessed(nodeKey);
        }

        for (Node* child : current->children) {
            if (child && child->type == "Declaration") {
                stack.push(child);
            }
        }
    }
}

/*
Description:
    Marks the pair as processed in the tree.
*/
void Tree::markDeclNodeAsProcessed(const std::string& nodeKey) {
    declNodeMap.at(nodeKey).second = true;
}

/*
Description:
    Checks if the node is processed in the tree.
*/
bool Tree::isDeclNodeProcessed(const std::string& nodeKey) const {
    return declNodeMap.at(nodeKey).second;
}

/*
Description:
    Checks if the node is in the tree.  
*/
bool Tree::isDeclNodeInAST(const std::string& nodeKey) const {
    return declNodeMap.count(nodeKey) > 0;
}

/*
Description:
    Builds a tree from the given file, created the node, provides some checks and returns the root node.
*/
Node* Tree::buildTree(std::ifstream& file) {
    std::vector<Node*> nodeStack;
    std::string line;

    while (std::getline(file, line)) {
        int depth = 0;
        while (line[depth] == ' ') {
            ++depth;
        }

        std::string type, kind, usr = "N/A", path = "N/A";
        int lineNumber = -1, columnNumber = -1;
        std::istringstream iss(line);

        // read the node information based on the current type
        if (!(iss >> type >> kind)) {
            std::cerr << "Error parsing type and kind from line: " << line << std::endl;
            continue; // skip invalid line
        }

        // try to read optional fields; default to "N/A" or -1 if not available
        if (!(iss >> usr)) {
            usr = "N/A"; // fallback if usr is missing
        }
        if (!(iss >> path)) {
            path = "N/A"; // fallback if path is missing
        }
        if (!(iss >> lineNumber)) {
            lineNumber = -1; // fallback if line number is missing
        }
        if (!(iss >> columnNumber)) {
            columnNumber = -1; // fallback if column number is missing
        }

        Node* node = new Node;
        node->type = type;
        node->kind = kind;
        node->usr = usr;
        node->path = path;
        node->lineNumber = lineNumber;
        node->columnNumber = columnNumber;

        while (nodeStack.size() > depth) {
            nodeStack.pop_back();
        }

        // set the parent of the current node
        node->parent = nodeStack.empty() ? nullptr : nodeStack.back();
        if (node->parent) {
            node->parent->children.push_back(node);
        }

        nodeStack.push_back(node);
    }

    return nodeStack.empty() ? nullptr : nodeStack.front();
}

/*
Description:
    Creates a map of nodes based on their keys, it's essential to compare the trees and print the differences, also the nodes are stored in a pair
    with the values of the nodes and a flag indicating if the node has been processed or not
*/
void Tree::createNodeMap() {
    if (!root) {
        std::cerr << "Root node is missing, cannot create the node map.\n";
        return;
    }
    
    std::queue<Node*> queue;

    queue.push(root);
    while (!queue.empty()) {
        Node* node = queue.front();
        queue.pop();

        if (!node) continue; // in case of missing information

        // generating the node key and ensuring if it's valid
        std::string nodeKey = Utils::getKey(node, node->type == "Declaration");
        if (!nodeKey.empty()) {
            if (node->type == "Declaration") {
                declNodeMap[nodeKey] = std::pair<Node*, bool>(node, false); // marking the node as not processed
            } else {
                const Node* declarationParent = Utils::findDeclarationParent(node);
                std::string declNodeKey = Utils::getKey(declarationParent, true);
                stmtNodeMultiMap.insert({declNodeKey, node}); // link the statement node to its declaration parent
            }
        }

        // processing child nodes
        for (Node* child : node->children) {
            if (child) {
                queue.push(child);
            }
        }
    }
}

/*
Description:
    Recursively deletes the tree starting from the given node.
*/
void Tree::deleteTree(Node* node) {
    if (node) {
        for (Node* child : node->children) {
            deleteTree(child);
        }
        delete node;
    }
}

/*
Description:
    Static method that marks the subtree of the statement nodes as processed in the given map.
*/
void Tree::markStmtSubTreeAsProcessed(Node* node, std::unordered_map<std::string, std::pair<Node*, bool>>& stmtMap) {
    if (!node) {
        return;
    }

    std::queue<Node*> queue;
    queue.push(node);

    while (!queue.empty()) {
        Node* current = queue.front();
        queue.pop();

        std::string nodeKey = Utils::getKey(current, false);
        stmtMap.at(nodeKey).second = true;

        for (Node* child : current->children) {
            queue.push(child);
        }
    }
}