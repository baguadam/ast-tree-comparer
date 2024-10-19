#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "./headers/tree.h"

Tree::Tree(const std::string& fileName) {
    root = buildTree(fileName);
}

Tree::~Tree() {
    deleteTree(root);
}

Node* Tree::getRoot() const {
    return root;
}

Node* Tree::buildTree(const std::string& fileName) {
    std::vector<Node*> nodeStack;
    std::string line;
    std::ifstream file(fileName);
    if (!file) {
        std::cerr << "Unable to open file: " << fileName << std::endl;
        return nullptr;
    }

    while (std::getline(file, line)) {
        int depth = 0;
        while (line[depth] == ' ') {
            ++depth;
        }

        std::string type, kind, usr = "N/A", path = "N/A";
        int lineNumber = -1, columnNumber = -1;
        std::istringstream iss(line);

        // Read the node information based on the current type
        if (!(iss >> type >> kind)) {
            std::cerr << "Error parsing type and kind from line: " << line << std::endl;
            continue; // Skip invalid line
        }

        // Try to read optional fields; default to "N/A" or -1 if not available
        if (!(iss >> usr)) {
            usr = "N/A"; // Fallback if usr is missing
        }
        if (!(iss >> path)) {
            path = "N/A"; // Fallback if path is missing
        }
        if (!(iss >> lineNumber)) {
            lineNumber = -1; // Fallback if line number is missing
        }
        if (!(iss >> columnNumber)) {
            columnNumber = -1; // Fallback if column number is missing
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

        // Set the parent of the current node
        node->parent = nodeStack.empty() ? nullptr : nodeStack.back();
        if (node->parent) {
            node->parent->children.push_back(node);
        }

        nodeStack.push_back(node);
    }

    return nodeStack.empty() ? nullptr : nodeStack.front();
}


void Tree::deleteTree(Node* node) {
    if (node) {
        for (Node* child : node->children) {
            deleteTree(child);
        }
        delete node;
    }
}