#include <iostream>
#include <queue>
#include "./headers/tree_comparer.h"

TreeComparer::TreeComparer(Node* firstAST, Node* secondAST) 
    : firstASTTree(firstAST), secondASTTree(secondAST), nodeMapFirstAST(createNodeMap(firstAST)), nodeMapSecondAST(createNodeMap(secondAST)) {} 

void TreeComparer::printDifferences() {
    for (const auto& pair : nodeMapFirstAST) {
        if (nodeMapSecondAST.count(pair.first) == 0) {
            std::cout << "Node " << pair.first << " removed from first AST\n";
        } else {
            if (pair.second->parent && pair.second->parent->name != nodeMapSecondAST[pair.first]->parent->name) {
                std::cout << "Node " << pair.first << " has a different parent in secont AST: " 
                        << nodeMapSecondAST[pair.first]->parent->name << "\n";
            }
            if (pair.second->value != nodeMapSecondAST[pair.first]->value) {
                std::cout << "Node " << pair.first << " has different values in the trees. In first AST: "
                          << pair.second->value << ", in second AST: " << nodeMapSecondAST[pair.first]->value << '\n';
            }
        } 
    }
    for (const auto& pair : nodeMapSecondAST) {
        if (nodeMapFirstAST.count(pair.first) == 0) {
            std::cout << "Node " << pair.first << " added in second AST\n";
        }
    }
}

/*
Generates a unique key for a node based on its type and information
*/
std::string TreeComparer::generateKey(Node* node, bool isDeclaration) {
    if (isDeclaration) {
        // unique identifier for declarations, here we can use the usr
        return node->usr; 
    } else {
        // for statements we use the kind, path, line and column
        return node->kind + "_" + node->path + "_" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber); 
    }
}

/*
Creates a map of nodes based on their keys, it's essential to compare the trees and print the differences
*/
std::unordered_map<std::string, Node*> TreeComparer::createNodeMap(Node* root) {
    std::unordered_map<std::string, Node*> nodeMap;
    std::queue<Node*> queue;

    if (root) {
        queue.push(root);
    }
    while (!queue.empty()) {
        Node* node = queue.front();
        queue.pop();

        if (!node) continue; // in case of missing information

        // generating the node key and ensuring if it's valid
        std::string nodeKey = generateKey(node, node->type == "Declaration");
        if (!nodeKey.empty()) {
            nodeMap[nodeKey] = node;
        }

        // processing child nodes
        for (Node* child : node->children) {
            queue.push(child);
        }
    }

    return nodeMap;
}

void TreeComparer::compareNodes(Node* firstNode, Node* secondNode) {

}

void TreeComparer::printNodeDetails(Node* node) {
    std::cout << "Node details:\n";
    std::cout << "Type: " << node->type << "\n";
    std::cout << "Kind: " << node->kind << "\n";
    std::cout << "USR: " << node->usr << "\n";
    std::cout << "Path: " << node->path << "\n";
    std::cout << "Line: " << node->lineNumber << "\n";
    std::cout << "Column: " << node->columnNumber << "\n";
    std::cout << "Parent USR: " << (node->parent ? node->parent->usr : "None") << "\n";
}