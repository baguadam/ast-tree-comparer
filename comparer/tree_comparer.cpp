#include <iostream>
#include <queue>
#include <stack>
#include "./headers/tree_comparer.h"

TreeComparer::TreeComparer(Node* firstAST, Node* secondAST) 
    : firstASTTree(firstAST), secondASTTree(secondAST), nodeMapFirstAST(createNodeMap(firstAST)), nodeMapSecondAST(createNodeMap(secondAST)) {} 

/*
Description:
    Public method that starts the comparison process by iterating over the two maps of nodes and comparing them,
    prints the necessary information about the differences to the console. First, it iterates over the first AST nodes,
    checks if the node exists in the second AST, if not, prints the details of the node. Then, it iterates over the second AST nodes,
    checks if the node exists in the first AST, if not, prints the details of the node. If the node exists in both trees, it compares them.
*/
void TreeComparer::printDifferences() {
    std::unordered_set<std::string> processedNodes;
    // iterating through the first AST nodes and comparing them with the second AST nodes
    for (const auto& pair : nodeMapFirstAST) {
        // if the node exists in the first AST, but not in the second AST, and it hasn't been processed yet
        if (nodeMapSecondAST.count(pair.first) == 0 && processedNodes.count(pair.first) == 0) {
            std::cout << "Node " << pair.first << " only exists in first AST, see node details below\n";
            printSubTree(pair.second, 0);       
            markSubTreeAsProcessed(pair.second, processedNodes); // no need to process (print separately) the children
            printSeparators();
        } else if (nodeMapSecondAST.count(pair.first) > 0) {
            // If the nodes exist in both trees, no matter if they have been processed or not, compare them (in case of children)
            compareNodes(pair.second, nodeMapSecondAST[pair.first]);
        } 
    }
    // similarly for the second AST nodes, check if the node exists in the first AST
    for (const auto& pair : nodeMapSecondAST) {
        if (nodeMapFirstAST.count(pair.first) == 0 && processedNodes.count(pair.first) == 0) {
            std::cout << "Node " << pair.first << " only exists in second AST, see node details below\n";
            printSubTree(pair.second, 0);
            markSubTreeAsProcessed(pair.second, processedNodes); // no need to process (print separately) the children      
            printSeparators();
        }
    }
}

/*
Description:
    Generates a unique key for a node based on its type and information
*/
std::string TreeComparer::generateKey(Node* node, bool isDeclaration) {
    if (isDeclaration) {
        // unique identifier for declarations, here we can use the usr
        return node->kind + "_"  + node->usr; 
    } else {
        // for statements we use the kind, path, line and column
        return node->kind + "_" + node->path + "_" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber); 
    }
}

/*
Description:
    Creates a map of nodes based on their keys, it's essential to compare the trees and print the differences
*/
std::unordered_map<std::string, Node*> TreeComparer::createNodeMap(Node* root) {
    std::unordered_map<std::string, Node*> nodeMap;
    std::queue<Node*> queue;

    if (!root) {
        std::cerr << "Root node is missing, cannot create the node map.\n";
        return nodeMap;
    }

    queue.push(root);
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
            if (child) {
                queue.push(child);
            }
        }
    }
    
    return nodeMap;
}

/*
Description:
    Comparison logic of two source locations
*/
void TreeComparer::compareSourceLocations(Node* firstNode, Node* secondNode) {
    // if the source is different, print the details of the locations for each node
    if (firstNode->path != secondNode->path || 
        firstNode->lineNumber != secondNode->lineNumber || 
        firstNode->columnNumber != secondNode->columnNumber) {
        std::cout << "Node " << firstNode->usr << " has different source locations in the trees.\n";
        std::cout << "First AST location: " << firstNode->path << ":" << firstNode->lineNumber << ":" << firstNode->columnNumber << '\n';
        std::cout << "Second AST location: " << secondNode->path << ":" << secondNode->lineNumber << ":" << secondNode->columnNumber << '\n';
        
        printSeparators();
    }
}

/*
Description:
    Comparison logic of two declarations
*/
void TreeComparer::compareDeclarations(Node* firstNode, Node* secondNode) {
    // in case of declaration types it is worth comparing the USRs
    // if the usr is different, print the details of the nodes and their USRs
    if (firstNode->usr != secondNode->usr) {
        std::cout << "USR related difference detected at the following node:\n";
        printNodeDetails(firstNode, " ");
        std::cout << "First AST USR: " << firstNode->usr << ", Second AST USR: " << secondNode->usr << '\n';

        printSeparators();
    }

    // if the kinds are different, print the details of the nodes and their kinds
    if (firstNode->kind != secondNode->kind) {
        std::cout << "Declaration node " << secondNode->usr << " | type " << secondNode->type << " has different kinds in the trees. In first AST: "
                  << firstNode->kind << ", in second AST: " << secondNode->kind << '\n';

        printSeparators();
    }

    // comparing the source locations of the nodes
    compareSourceLocations(firstNode, secondNode);
}

/*
Descpiption:
    Comparison logic of two statements
*/
void TreeComparer::compareStatements(Node* firstNode, Node* secondNode) {
    // if the kinds are different, print the details of the nodes and their kinds
    if (firstNode->kind != secondNode->kind) {
        std::cout << "Declaration node " << secondNode->usr << " | type " << secondNode->type << " has different kinds in the trees. In first AST: "
                  << firstNode->kind << ", in second AST: " << secondNode->kind << '\n';
        
        printSeparators();
    }

    // comparing the source locations of the nodes
    compareSourceLocations(firstNode, secondNode);
}

/*
Description:
    Main comparison method for comparing two nodes taking into account many aspects and printing the differences
*/
void TreeComparer::compareNodes(Node* firstNode, Node* secondNode) {
    // checking for parents, if the first node has parent, but not the same as the second one,
    // print the details of the nodes and their parents
    if (firstNode->parent && (!secondNode->parent || firstNode->parent->usr != secondNode->parent->usr)) {
        std::cout << "Node " << firstNode->usr << " | type " << firstNode->type << " has a different parent in second AST: "
                  << secondNode->parent->usr << "\n";
        std::cout << "First AST parent details:\n";
        printNodeDetails(firstNode->parent, " ");
        std::cout << "Second AST parent details:\n";
        printNodeDetails(secondNode->parent, " ");

        printSeparators();
    }
    // checking if their types are different, in case of different types we don't want to compare any values,
    // just print the details of the differring nodes. However, in case of similar types, we can compare the nodes
    if (firstNode->type != secondNode->type) {
        std::cout << "Node " << firstNode->usr << " | type " << firstNode->type << " has different types in the trees. In first AST: "
                  << firstNode->type << ", in second AST: " << secondNode->type << '\n';

        printSeparators();
    } else {
        if (firstNode->type == "Declaration") {
            // if both nodes are DECLARATIONS, comparing them accordingly
            compareDeclarations(firstNode, secondNode);
        } else if (firstNode->type == "Statement") {
            // if both nodes are STATEMENTS, comparing them accordingly
            compareStatements(firstNode, secondNode);
        }
    }
}

/*
Description:
    Sets a flag for a given node and all its children that they have been processed, if the parent node
    can only be found in one of the ASTs, so no need to process the children again
*/
void TreeComparer::markSubTreeAsProcessed(Node* node, std::unordered_set<std::string>& processedNodes) {
    if (!node) {
        return;
    }

    std::stack<Node*> stack;
    stack.push(node);

    while (!stack.empty()) {
        Node* current = stack.top();
        stack.pop();

        // generating the key for the node and marking it as processed
        std::string nodeKey = generateKey(current, current->type == "Declaration");
        if (!nodeKey.empty()) {
            processedNodes.insert(nodeKey);
        }

        // pushing the children onto the stack
        for (Node* child : current->children) {
            if (child) {
                stack.push(child);
            }
        }
    }
}

/*
Description:
    Prints the subtree of a given node, recursively calling the function for its children
*/
void TreeComparer::printSubTree(Node* node, int depth = 0) {
    if (!node) {
        return;
    }

    // indent for better readability
    std::string indent(depth * 2, ' ');

    printNodeDetails(node, indent);
    for (Node* child : node->children) {
        printSubTree(child, depth + 1);
    }
}

/*
Description:
    Prints the details of a given node to the standard output
*/
void TreeComparer::printNodeDetails(Node* node, std::string indent = " ") {
    std::cout << indent << "* Type: " << node->type << "\n";
    std::cout << indent << "* Kind: " << node->kind << "\n";
    std::cout << indent << "* USR: " << node->usr << "\n";
    std::cout << indent << "* Location: " << node->path << " " << node->lineNumber << ":" << node->columnNumber << "\n";
    std::cout << indent << "Parent USR: " << (node->parent ? node->parent->usr : "None") << "\n";
    
    printSeparators();
}

/*
Description:
    Prints a separator for better readability
*/
void TreeComparer::printSeparators() {
    std::cout << "**********************************************************\n";
}