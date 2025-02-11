#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <queue>
#include <stack>
#include "../include/tree.h"
#include "../include/utils.h"

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

    file.close(); // explicitly close the file
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
    Returns the declaration nodes based on the key.
*/
const std::pair<std::unordered_multimap<std::string, Node*>::const_iterator,
                std::unordered_multimap<std::string, Node*>::const_iterator>
Tree::getDeclNodes(const std::string& nodeKey) const {
    return declNodeMultiMap.equal_range(nodeKey);
}

/*
Description:
    Returns the statement nodes based on the key of the declaration.
*/
const std::pair<std::vector<Node*>::const_iterator, std::vector<Node*>::const_iterator> Tree::getStmtNodes(const std::string& nodeKey) const {
    auto it = stmtNodeMultiMap.find(nodeKey);
    if (it != stmtNodeMultiMap.end()) {
        return {it->second.cbegin(), it->second.cend()};
    }

    return {std::vector<Node*>::const_iterator{}, std::vector<Node*>::const_iterator{}}; // empty range is key is not found
}

/*
Description:
    Returns the declaration node of the multiple nodes in the tree.
*/
const std::unordered_multimap<std::string, Node*>& Tree::getDeclNodeMultiMap() const {
    return declNodeMultiMap;
}

/*
    Returns the statement node map of the tree.
*/
const std::unordered_map<std::string, std::vector<Node*>>& Tree::getStmtNodeMultiMap() const {
    return stmtNodeMultiMap;
}

/*
Description:
    Checks if the node is in the tree.  
*/
bool Tree::isDeclNodeInAST(const std::string& nodeKey) const {
    return (declNodeMultiMap.count(nodeKey) > 0);
}

/*
Description:
    Processes a sutree of a given node using DFS traversal, uses the callback fuction to process the node, therefore it can be used
    both for Statements and Declarations.
*/
void Tree::processSubTree(Node* node, std::function<void(Node*, int)> processNode) {
    if (!node) {
        return;
    }

    // stack for DFS traversal; store both the node and its depth in the tree
    std::stack<std::pair<Node*, int>> stack;
    stack.push({node, 0});

    while (!stack.empty()) {
        // pop the current node and its depth
        auto [current, depth] = stack.top();
        stack.pop();
        
        processNode(current, depth);

        for (Node* child : current->children) {
            if (child) {
                stack.push({child, depth + 1});
            }
        }
    }
}

/*
Description:
    Builds a tree from the given file, creates nodes, performs various checks, and returns the root node.
*/
Node* Tree::buildTree(std::ifstream& file) {
    std::vector<Node*> nodeStack;
    std::string line;
    int currentIndex = 0;

    while (std::getline(file, line)) {
        // normalize line endings (remove trailing \r if present, typical in Windows)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            continue;
        }

        // depth of the current node
        int depth = 0;
        while (depth < line.size() && line[depth] == ' ') {
            ++depth;
        }

        std::vector<std::string> tokens = Utils::splitString(line);
        if (tokens.size() < 6) {
            std::cerr << "Warning: Invalid line in the file (expected at least 6 tokens): " << line << '\n';
            continue;
        }

        // trimming
        Utils::ltrim(tokens[0]);

        int lineNumber = 0;
        int columnNumber = 0;
        try {
            lineNumber = std::stoi(tokens[4]);
            columnNumber = std::stoi(tokens[5]);
        } catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to parse line or column number from line: " << line << " - " << e.what() << '\n';
            throw std::runtime_error("Failed to parse line or column number.");
        }

        // new node
        Node* node = new Node;
        node->type = Utils::stringToNodeType(tokens[0]);
        node->kind = tokens[1];
        node->usr = tokens[2];
        node->path = tokens[3];
        node->lineNumber = lineNumber;
        node->columnNumber = columnNumber;
        node->topologicalOrder = currentIndex++;

        // adjust the stack's size
        if (nodeStack.size() > depth) {
            nodeStack.resize(depth);
        }

        // parent of the current node
        node->parent = nodeStack.empty() ? nullptr : nodeStack.back();
        if (node->parent) {
            node->parent->children.push_back(node);
        }
        nodeStack.push_back(node);

        // fingerprint generation + unique key
        node->fingerprint = Utils::getFingerPrint(node);
        if (node->type == DECLARATION) {
            node->enhancedKey = Utils::getEnhancedDeclKey(node);
            addDeclNodeToNodeMap(node);
        } else {
            const Node* lastDeclarationNode = Utils::findDeclarationParent(node);
            if (lastDeclarationNode) {
                node->enhancedKey = Utils::getStmtKey(node, lastDeclarationNode->enhancedKey);
                addStmtNodeToNodeMap(node, lastDeclarationNode);
            } else {
                // if no declaration parent found, delete the node to prevent a memory leak
                std::cerr << "Warning: Could not find declaration parent for statement node: " << node->kind
                          << " at path: " << node->path << " (line: " << node->lineNumber
                          << ", column: " << node->columnNumber << ")\n";
                continue;
            }
        }
    }


    return nodeStack.empty() ? nullptr : nodeStack.front();
}

/*
Description:
    Adds the statement node with its key to the stmtNodeMultiMap.
*/
void Tree::addStmtNodeToNodeMap(Node* node, const Node* declarationParent) {
    std::string key = declarationParent->enhancedKey + "|" + std::to_string(declarationParent->topologicalOrder);
    stmtNodeMultiMap[key].emplace_back(node);
}

/*
Description:
    Adds the declaration node to the declNodeMultiMap.
*/
void Tree::addDeclNodeToNodeMap(Node* node) {
    declNodeMultiMap.emplace(node->enhancedKey, node);
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