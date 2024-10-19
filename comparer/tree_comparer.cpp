#include <iostream>
#include <queue>
#include "./headers/tree_comparer.h"

TreeComparer::TreeComparer(Node* firstAST, Node* secondAST) 
    : firstASTTree(firstAST), secondASTTree(secondAST), nodeMapFirstAST(createNodeMap(firstAST)), nodeMapSecondAST(createNodeMap(secondAST)) {} 

/*
Public method that starts the comparison process by iterating over the two maps of nodes and comparing them,
prints the necessary information about the differences to the console
*/
void TreeComparer::printDifferences() {
    // iterating through the first AST nodes and comparing them with the second AST nodes,
    // first checking if the node exists in the second AST, if not, print the details of the node,
    // otherwise compare the nodes
    for (const auto& pair : nodeMapFirstAST) {
        if (nodeMapSecondAST.count(pair.first) == 0) {
            std::cout << "Node " << pair.first << " only exists in first AST\n";
            printNodeDetails(pair.second);
        } else {
            // if the nodes exist in both trees, compare them
            compareNodes(pair.second, nodeMapSecondAST[pair.first]);
        } 
    }
    // similarly for the seoncd AST nodes, checking if the node exists in the first AST, if not, print the details of the node
    for (const auto& pair : nodeMapSecondAST) {
        if (nodeMapFirstAST.count(pair.first) == 0) {
            std::cout << "Node " << pair.first << " only exists in second AST\n";
            printNodeDetails(pair.second);
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

/*
Comparison logic of two source locations
*/
void TreeComparer::compareSourceLocations(Node* firstNode, Node* secondNode) {
    // if the source is different, print the details of the locations for each node
    if (firstNode->path != secondNode->path || 
        firstNode->lineNumber != secondNode->lineNumber || 
        firstNode->columnNumber != secondNode->columnNumber) {
        std::cout << "Declaration node " << firstNode->usr << " has different source locations in the trees.\n";
        std::cout << "First AST location: " << firstNode->path << ":" << firstNode->lineNumber << ":" << firstNode->columnNumber << '\n';
        std::cout << "Second AST location: " << secondNode->path << ":" << secondNode->lineNumber << ":" << secondNode->columnNumber << '\n';

        std::cout << "**********************************************************\n";
    }
}

/*
Comparison logic of two declarations
*/
void TreeComparer::compareDeclarations(Node* firstNode, Node* secondNode) {
    // in case of declaration types it is worth comparing the USRs
    // if the usr is different, print the details of the nodes and their USRs
    if (firstNode->usr != secondNode->usr) {
        std::cout << "Node:\n";
        printNodeDetails(firstNode);
        std::cout << "Has different USRs - first AST USR: " << firstNode->usr << ", Second AST USR: " << secondNode->usr << '\n';

        std::cout << "**********************************************************\n";
    }

    // if the kinds are different, print the details of the nodes and their kinds
    if (firstNode->kind != secondNode->kind) {
        std::cout << "Declaration node " << secondNode->usr << " | type " << secondNode->type << " has different kinds in the trees. In first AST: "
                  << firstNode->kind << ", in second AST: " << secondNode->kind << '\n';

        std::cout << "**********************************************************\n";
    }

    // comparing the source locations of the nodes
    compareSourceLocations(firstNode, secondNode);
}

/*
Comparison logic of two statements
*/
void TreeComparer::compareStatements(Node* firstNode, Node* secondNode) {
    // if the kinds are different, print the details of the nodes and their kinds
    if (firstNode->kind != secondNode->kind) {
        std::cout << "Declaration node " << secondNode->usr << " | type " << secondNode->type << " has different kinds in the trees. In first AST: "
                  << firstNode->kind << ", in second AST: " << secondNode->kind << '\n';
        
        std::cout << "**********************************************************\n";
    }

    // comparing the source locations of the nodes
    compareSourceLocations(firstNode, secondNode);
}

/*
Main comparison method for comparing two nodes taking into account many aspects and printing the differences
*/
void TreeComparer::compareNodes(Node* firstNode, Node* secondNode) {
    // checking for parents, if the first node has parent, but not the same as the second one,
    // print the details of the nodes and their parents
    if (firstNode->parent && (!secondNode->parent || firstNode->parent->usr != secondNode->parent->usr)) {
        std::cout << "Node " << firstNode->usr << " | type " << firstNode->type << " has a different parent in second AST: "
                  << secondNode->parent->usr << "\n";
        std::cout << "First AST parent details:\n";
        printNodeDetails(firstNode->parent);
        std::cout << "Second AST parent details:\n";
        printNodeDetails(secondNode->parent);

        std::cout << "**********************************************************\n";
    }
    // checking if their types are different, in case of different types we don't want to compare any values,
    // just print the details of the differring nodes. However, in case of similar types, we can compare the nodes
    if (firstNode->type != secondNode->type) {
        std::cout << "Node " << firstNode->usr << " | type " << firstNode->type << " has different types in the trees. In first AST: "
                  << firstNode->type << ", in second AST: " << secondNode->type << '\n';

        std::cout << "**********************************************************\n";
    } else if (firstNode->type == "Declaration") {
        // if both nodes are DECLARATIONS, comparing them accordingly
        compareDeclarations(firstNode, secondNode);
    } else if (firstNode->type == "Statement") {
        // if both nodes are STATEMENTS, comparing them accordingly
        compareStatements(firstNode, secondNode);
    }
}

/*
Prints the details of a given node to the standard output
*/
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