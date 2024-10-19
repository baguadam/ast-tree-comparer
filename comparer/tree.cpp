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

        std::string type, kind, usr, path;
        int lineNumber, columnNumber;
        std::istringstream iss(line);

        // reading the node informations
        iss >> type >> kind >> usr >> path >> lineNumber >> columnNumber;

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

void Tree::deleteTree(Node* node) {
    if (node) {
        for (Node* child : node->children) {
            deleteTree(child);
        }
        delete node;
    }
} 