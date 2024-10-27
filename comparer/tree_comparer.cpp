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

        // process the node
        processNode(current);
    }
}

void TreeComparer::processNode(Node* current) {
    std::string nodeKey = getKey(current, current->type == "Declaration");

    if (isNodeInFirstAST(nodeKey) && isNodeInSecondAST(nodeKey)) {
        // node exists in both ASTs, compare them
        std::pair<Node*, bool>& secondNodePair = nodeMapSecondAST.at(nodeKey);
        std::pair<Node*, bool>& firstNodePair = nodeMapFirstAST.at(nodeKey);
        compareNodes(firstNodePair.first, secondNodePair.first);  // compare the nodes

        // mark nodes as processed
        firstNodePair.second = true;
        secondNodePair.second = true;
    } else if (isNodeInFirstAST(nodeKey)) {
        // node exists only in the first AST
        processNodeInFirstAST(current, nodeKey);
    } else if (isNodeInSecondAST(nodeKey)) {
        // node exists only in the second AST
        processNodeInSecondAST(current, nodeKey);
    } else {
        // node does not exist in either AST, should not happen
        std::cerr << "Error: Node key " << nodeKey << " not found in either AST map.\n";
    }
}

/*
Description:
    Processes a node that exists only in one of the ASTs, prints the details of the node and marks the subtree as processed
*/
void TreeComparer::processNodeInSingleAST(Node* current, const std::string& nodeKey, std::unordered_map<std::string, std::pair<Node*, bool>>& nodes, const char* astName) {
    if (isNodeProcessedInAST(nodeKey, nodes)) return;  // skip if already processed

    std::cout << "Node " << nodeKey << " only exists in the " << astName << " AST, skipping its children\n";
    printSubTree(current, 0);
    markSubTreeAsProcessed(current, nodes);  // mark entire subtree as processed
    printSeparators();
}

/*
Description:
    Processes the node that only exists in the first AST, prints the details of the node
*/
void TreeComparer::processNodeInFirstAST(Node* current, const std::string& nodeKey) {
    processNodeInSingleAST(current, nodeKey, nodeMapFirstAST, "=FIRST=");
}

/*
Description:
    Processes the node that only exists in the second AST, prints the details of the node
*/
void TreeComparer::processNodeInSecondAST(Node* current, const std::string& nodeKey) {
    processNodeInSingleAST(current, nodeKey, nodeMapSecondAST, "=SECOND=");
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
    std::string nodeKey = node->kind + "|" + node->usr;
    // for function it's important to make the key unique by adding the source location in the code
    if (node->kind == "Function") {
        nodeKey += "|" + node->path + "|" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber);
    } else if (!isDeclaration) { 
        // for statements we use the kind, path, line and column
        nodeKey += "|" + node->kind + "|" + node->path + "|" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber) + "|" + node->parent->usr; 
    }

    return nodeKey;
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
    if (firstNode->kind == "Function") {
        // if both nodes are FUNCTIONS, comparing them accordingly
        compareFunctions(firstNode, secondNode);
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
    Comparison logic of two function declarations, the function declarations get compared if they exist in both ASTs
*/
void TreeComparer::compareFunctions(Node* firstNode, Node* secondNode) {
    size_t firstChildrenSize = firstNode->children.size();
    size_t secondChildrenSize = secondNode->children.size();

    // if the number of children is different, print the information
    if (firstChildrenSize != secondChildrenSize) {
        std::cout << "Function " << firstNode->usr << " has a different number of children in the trees.\n";
        std::cout << "First AST children: " << firstChildrenSize << ", Second AST children: " << secondChildrenSize << '\n';
        printSeparators();

        // find the differing children
        size_t minSize = std::min(firstChildrenSize, secondChildrenSize);
        for (size_t i = 0; i < minSize; ++i) {
            compareNodes(firstNode->children[i], secondNode->children[i]);
        }

        // if first AST has more children
        if (firstChildrenSize > secondChildrenSize) {
            std::cout << "Extra children in the first AST:\n";
            for (size_t i = secondChildrenSize; i < firstChildrenSize; ++i) {
                printSubTree(firstNode->children[i], 1);
                markSubTreeAsProcessed(firstNode->children[i], nodeMapFirstAST); 
            }
        }
        // if second AST has more children
        else if (secondChildrenSize > firstChildrenSize) {
            std::cout << "Extra children in the second AST:\n";
            for (size_t i = firstChildrenSize; i < secondChildrenSize; ++i) {
                printSubTree(secondNode->children[i], 1); 
                markSubTreeAsProcessed(secondNode->children[i], nodeMapSecondAST);
            }
        }
    } else {
        // if the number of children is the same, compare them one by one
        for (size_t i = 0; i < firstChildrenSize; ++i) {
            compareNodes(firstNode->children[i], secondNode->children[i]);
        }
    }
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

    // at this point if they exist in both ASTs, their type must be the same, therefore we can decide how to compare them
    if (firstNode->type == "Declaration") {
        compareDeclarations(firstNode, secondNode);
    } else if (firstNode->type == "Statement") {
        compareStatements(firstNode, secondNode);
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

/*
Description:
    Helper method to check if a given node is in the AST based on the map of nodes
*/
bool TreeComparer::isNodeInAST(const std::string& nodeKey, const std::unordered_map<std::string, std::pair<Node*, bool>>& nodes) const {
    return nodes.count(nodeKey) > 0;
}

/*
Description:
    Checks if a given node is in the first AST
*/
bool TreeComparer::isNodeInFirstAST(const std::string& nodeKey) const {
    return isNodeInAST(nodeKey, nodeMapFirstAST);
}

/*
Description:
    Checks if a given node is in the second AST
*/
bool TreeComparer::isNodeInSecondAST(const std::string& nodeKey) const {
    return isNodeInAST(nodeKey, nodeMapSecondAST);
}

/*
Description:
    Checks if a given node has been processed in the specified AST map
*/
bool TreeComparer::isNodeProcessedInAST(const std::string& nodeKey, const std::unordered_map<std::string, std::pair<Node*, bool>>& nodeMap) const {
    return nodeMap.at(nodeKey).second;
}