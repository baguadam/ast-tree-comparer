#include <iostream>
#include <queue>
#include "tree_comparer.h"

TreeComparer::TreeComparer(Node* firstAST, Node* secondAST) 
    : firstASTTree(firstAST), secondASTTree(secondAST), nodeMapFirstAST(createNodeMap(firstAST)), nodeMapSecondAST(createNodeMap(secondAST)) {} 

void TreeComparer::printDifferences() {
    for (const auto& pair : nodeMapFirstAST) {
        if (nodeMapSecondAST.count(pair.first) == 0) {
            std::cout << "Node " << pair.first << " removed from first AST\n";
        } else if (pair.second->parent->name != nodeMapSecondAST[pair.first]->parent->name) {
            std::cout << "Node " << pair.first << " has a different parent in secont AST: " 
                      << nodeMapSecondAST[pair.first]->parent->name << "\n";
        }
    }
    for (const auto& pair : nodeMapSecondAST) {
        if (nodeMapFirstAST.count(pair.first) == 0) {
            std::cout << "Node " << pair.first << " added in second AST\n";
        }
    }
}

std::unordered_map<std::string, Node*> TreeComparer::createNodeMap(Node* root) {
    std::unordered_map<std::string, Node*> nodeMap;
    std::queue<Node*> queue;

    if (root) {
        queue.push(root);
    }
    while (!queue.empty()) {
        Node* node = queue.front();
        queue.pop();
        nodeMap[node->name] = node;
        for (Node* child : node->children) {
            queue.push(child);
        }
    }
    return nodeMap;
}