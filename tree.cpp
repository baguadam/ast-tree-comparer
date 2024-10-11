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

        std::string nodeName;
        std::string nodeValue;
        std::istringstream iss(line);

        // reading the first word into nodeName
        iss >> nodeName;

        // rest of the line into nodeValue
        std::getline(iss, nodeValue);
        // removing whitespaces
        nodeValue.erase(nodeValue.begin(), std::find_if(nodeValue.begin(), nodeValue.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        nodeValue.erase(std::find_if(nodeValue.rbegin(), nodeValue.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), nodeValue.end()); 

        Node* node = new Node;
        node->name = nodeName;
        node->value = nodeValue;

        while (nodeStack.size() > depth) {
            nodeStack.pop_back();
        }

        node->parent = nodeStack.empty() ? nullptr : nodeStack.back();
        if (node->parent) {
            node->parent->children.push_back(node);
        }

        nodeStack.push_back(node);
    }
    
    return nodeStack.front();
}

void Tree::deleteTree(Node* node) {
    if (node) {
        for (Node* child : node->children) {
            deleteTree(child);
        }
        delete node;
    }
} 