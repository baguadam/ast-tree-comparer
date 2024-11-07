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
    return it->second;
}

/*
Description:
    Returns the statement nodes based on the key of the declaration.
*/
const std::vector<std::pair<std::string, Node*>> Tree::getStmtNodes(const std::string& nodeKey) const {
    std::vector<std::pair<std::string, Node*>> stmtNodes;
    auto range = stmtNodeMultiMap.equal_range(nodeKey);
    for (auto it = range.first; it != range.second; ++it) {
        stmtNodes.emplace_back(Utils::getKey(it->second, false), it->second);
    }
    return stmtNodes;
}

/*
Description:
    Marks the node as processed in the tree.
*/
void Tree::markDeclNodeAsProcessed(const std::string& nodeKey) {
    auto it = declNodeMap.find(nodeKey);
    if (it != declNodeMap.end()) {
        it->second->isProcessed = true;
    } else {
        std::cerr << "Warning: Node key not found in the declaration node map: " << nodeKey << '\n';
    }
}

/*
Description:
    Returns the node map of the tree.
*/
const std::unordered_map<std::string, Node*>& Tree::getDeclNodeMap() const {
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
    Checks if the node is processed in the tree.
*/
bool Tree::isDeclNodeProcessed(const std::string& nodeKey) const {
    auto it = declNodeMap.find(nodeKey);
    return it != declNodeMap.end() && it->second->isProcessed;
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
    Node* lastDeclarationNode = nullptr;
    std::string line;
    int currentIndex = 0;

    while (std::getline(file, line)) {
        int depth = 0;
        while (line[depth] == ' ') {
            ++depth;
        }

        std::vector<std::string> tokens = Utils::splitString(line);
        if (tokens.size() < 6) {
            std::cerr << "Warning: Invalid line in the file: " << line << '\n';
            continue;
        }

        // trimming the type from the leading whitespace
        Utils::ltrim(tokens[0]);

        try {
            int lineNumber = std::stoi(tokens[4]);
            int columNumber = std::stoi(tokens[5]);

            Node* node = new Node;
            node->type = Utils::stringToNodeType(tokens[0]);
            node->kind = tokens[1];
            node->usr = tokens[2];
            node->path = tokens[3];
            node->lineNumber = lineNumber;
            node->columnNumber = columNumber;

            // topological order of the node
            node->topologicalOrder = currentIndex++;

            while (nodeStack.size() > depth) {
                nodeStack.resize(depth);
            }

            // set the parent of the current node
            node->parent = nodeStack.empty() ? nullptr : nodeStack.back();
            if (node->parent) {
                node->parent->children.push_back(node);
            }

            nodeStack.push_back(node);

            // set the unique id and the fingerprint of the node
            if (node->type == DECLARATION) {
                node->enhancedKey = Utils::getEnhancedDeclKey(node);
                node->fingerprint = Utils::getFingerPrint(node);
                lastDeclarationNode = node;
                addDeclNodeToNodeMap(node);
            } else {
                if (lastDeclarationNode) {
                    node->enhancedKey = Utils::getStmtKey(node, lastDeclarationNode->enhancedKey);
                    addStmtNodeToNodeMap(node, lastDeclarationNode->enhancedKey);
                } else {
                    std::cerr << "Warning: Could not find declaration parent for statement node: " << node->kind << '\n';
                    delete node; // avoid memory leak
                    continue;
                }
            }

        } catch(const std::exception& e) {
            std::cerr << "ERROR: parsing to int " << line << '\n';
            continue;
        }
    }

    return nodeStack.empty() ? nullptr : nodeStack.front();
}

/*
Description:
    Adds the statement node with its key to the stmtNodeMultiMap.
*/
void Tree::addStmtNodeToNodeMap(Node* node, const std::string& declarationParentKey) {
    if (!declarationParentKey.empty()) {
        stmtNodeMultiMap.emplace(declarationParentKey, node);  // Use emplace for direct insertion
    } else {
        std::cerr << "Warning: Could not find declaration parent for statement node: " << node->kind << '\n';
    }
}

/*
Description:
    Adds the declaration node with its key to the declNodeMap.
*/
void Tree::addDeclNodeToNodeMap(Node* node) {
    auto result = declNodeMap.insert({node->enhancedKey, node});
    if (!result.second) {
        std::cerr << "Warning: Duplicate declaration node key detected: " << node->enhancedKey << '\n';
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