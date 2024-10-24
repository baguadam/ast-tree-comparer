#include <iostream>
#include <stack>
#include "./headers/tree_comparer.h"

TreeComparer::TreeComparer(Node* firstAST, Node* secondAST) 
    : firstASTTree(firstAST), secondASTTree(secondAST), nodeMapFirstAST(createNodeMap(firstAST)), nodeMapSecondAST(createNodeMap(secondAST)) {} 

/*
Description:
    Public method that starts the comparison process by using BFS approach for comparing the nodes, ensuring that parents are processed before children,
    prints the necessary information about the differences to the console. 
*/
void TreeComparer::printDifferences() {
    std::queue<Node*> queue;

    // wtart with the root nodes of both ASTs
    if (firstASTTree) queue.push(firstASTTree);
    if (secondASTTree) queue.push(secondASTTree);

    while (!queue.empty()) {
        Node* current = queue.front();
        queue.pop();

        // add children to the queue for further processing
        enqueueChildren(current, queue);

        // generate the key for the node
        std::string nodeKey = getKey(current, current->type == "Declaration");

        if (nodeMapFirstAST.count(nodeKey) > 0) {
            processNodeInFirstAST(current, nodeKey);
        } else if (nodeMapSecondAST.count(nodeKey) > 0) {
            processNodeInSecondAST(current, nodeKey);
        } else {
            // node does not exist in either AST, should not happen
            std::cerr << "Error: Node key " << nodeKey << " not found in either AST map.\n";
        }
    }
}

/*
Description:
    Processes a node in the first AST, checks if the node is already processed, if not, checks if the node exists in the second AST,
    if not, prints the details of the node. If the node exists in both ASTs, compares them.
*/
void TreeComparer::processNodeInFirstAST(Node* current, const std::string& nodeKey) {
    if (nodeMapFirstAST.at(nodeKey).second) return;  // skip if already processed

    if (nodeMapSecondAST.count(nodeKey) == 0) {
        // Node exists ONLY in the first AST
        std::cout << "Node " << nodeKey << " only exists in first AST, skipping its children\n";
        printSubTree(current, 0);
        markSubTreeAsProcessed(current, nodeMapFirstAST);  // mark entire subtree as processed
        printSeparators();
    } else {
        // Node exists in both ASTs, compare them
        try {
            std::pair<Node*, bool>& secondNodePair = nodeMapSecondAST.at(nodeKey);
            std::pair<Node*, bool>& firstNodePair = nodeMapFirstAST.at(nodeKey);
            compareNodes(firstNodePair.first, secondNodePair.first);  // compare the nodes

            // mark nodes as processed
            firstNodePair.second = true;
            secondNodePair.second = true;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Node key " << nodeKey << " not found in one of the AST maps.\n";
        }
    }
}

/*
Description:
    Processes a node in the second AST, checks if the node is already processed, if not, checks if the node exists in the first AST,
    if not, prints the details of the node. 
*/
void TreeComparer::processNodeInSecondAST(Node* current, const std::string& nodeKey) {
    if (nodeMapSecondAST.at(nodeKey).second) return;  // skip if already processed

    if (nodeMapFirstAST.count(nodeKey) == 0) {
        // Node exists ONLY in the second AST
        std::cout << "Node " << nodeKey << " only exists in second AST, skipping its children\n";
        printSubTree(current, 0);
        markSubTreeAsProcessed(current, nodeMapSecondAST);  // mark entire subtree as processed
        printSeparators();
    }
}

/*
Description:
    Enqueues the children of a given node to the queue
*/
void TreeComparer::enqueueChildren(Node* current, std::queue<Node*>& queue) {
    for (Node* child : current->children) {
        if (child) {
            queue.push(child);
        }
    }
}

/*
Description:
    Generates a unique key for a node based on its type and information
*/
std::string TreeComparer::getKey(Node* node, bool isDeclaration) const {
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
    Creates a map of nodes based on their keys, it's essential to compare the trees and print the differences, also the nodes are stored in a pair
    with the values of the nodes and a flag indicating if the node has been processed or not
*/
std::unordered_map<std::string, std::pair<Node*, bool>> TreeComparer::createNodeMap(Node* root) {
    std::unordered_map<std::string, std::pair<Node*, bool>> nodeMap;
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
        std::string nodeKey = getKey(node, node->type == "Declaration");
        if (!nodeKey.empty()) {
            nodeMap[nodeKey] = std::pair<Node*, bool>(node, false); // marking the node as not processed
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
void TreeComparer::markSubTreeAsProcessed(Node* node, std::unordered_map<std::string, std::pair<Node*, bool>>& nodes) {
    if (!node) {
        return;
    }

    std::stack<Node*> stack;
    stack.push(node);

    while (!stack.empty()) {
        Node* current = stack.top();
        stack.pop();

        // generating the key for the node and marking it as processed
        std::string nodeKey = getKey(current, current->type == "Declaration");
        if (!nodeKey.empty()) {
            nodes.at(nodeKey).second = true;
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
void TreeComparer::printSubTree(Node* node, int depth = 0) const {
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
void TreeComparer::printNodeDetails(Node* node, std::string indent = " ") const {
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
void TreeComparer::printSeparators() const {
    std::cout << "**********************************************************\n";
}