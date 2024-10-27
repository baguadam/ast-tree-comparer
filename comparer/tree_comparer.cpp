#include <iostream>
#include <stack>
#include "./headers/tree_comparer.h"
#include "./headers/utils.h"

TreeComparer::TreeComparer(Tree& firstTree, Tree& secondTree) : firstASTTree(firstTree), secondASTTree(secondTree) {} 

/*
Description:
    Public method that starts the comparison process by using BFS approach for comparing the nodes, ensuring that parents are processed before children,
    prints the necessary information about the differences to the console. 
*/
void TreeComparer::printDifferences() {
    std::queue<Node*> queue;

    // start with the root nodes of both ASTs
    if (firstASTTree.getRoot()) queue.push(firstASTTree.getRoot());
    if (secondASTTree.getRoot()) queue.push(secondASTTree.getRoot());

    while (!queue.empty()) {
        Node* current = queue.front();
        queue.pop();

        // add children to the queue for further processing
        enqueueChildren(current, queue);

        // process the node
        processNode(current);
    }
}

/*
Descpirion:
    Processes a node by comparing it with the corresponding node in the other AST, if the node exists in both ASTs, 
    compares them, otherwise processes the node that exists only in one of the ASTs.
*/
void TreeComparer::processNode(Node* current) {
    std::string nodeKey = Utils::getKey(current, current->type == "Declaration");

    if (firstASTTree.isNodeInAST(nodeKey) && secondASTTree.isNodeInAST(nodeKey)) {
        // node exists in both ASTs, compare them
        const Node* firstASTNode = firstASTTree.getNodeFromNodeMap(nodeKey);
        const Node* secondASTNode = secondASTTree.getNodeFromNodeMap(nodeKey);
        compareSimilarNodes(firstASTNode, secondASTNode);

        // mark nodes as processed
        firstASTTree.markNodeAsProcessed(nodeKey);
        secondASTTree.markNodeAsProcessed(nodeKey);
    } else if (firstASTTree.isNodeInAST(nodeKey)) {
        // node exists only in the first AST
        processNodeInFirstAST(current, nodeKey);
    } else if (secondASTTree.isNodeInAST(nodeKey)) {
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
void TreeComparer::processNodeInSingleAST(Node* current, const std::string& nodeKey, Tree& tree, const char* astName) {
    if (tree.isNodeProcessed(nodeKey)) return;  // skip if already processed

    std::cout << "Node " << nodeKey << " only exists in the " << astName << " AST, skipping its children\n";
    printSubTree(current, 0);
    tree.markSubTreeAsProcessed(current);  // mark entire subtree as processed
    printSeparators();
}

/*
Description:
    Processes the node that only exists in the first AST, prints the details of the node
*/
void TreeComparer::processNodeInFirstAST(Node* current, const std::string& nodeKey) {
    processNodeInSingleAST(current, nodeKey, firstASTTree, "=FIRST=");
}

/*
Description:
    Processes the node that only exists in the second AST, prints the details of the node
*/
void TreeComparer::processNodeInSecondAST(Node* current, const std::string& nodeKey) {
    processNodeInSingleAST(current, nodeKey, firstASTTree, "=SECOND=");
}

/*
Description:
    Comparison logic of two source locations
*/
void TreeComparer::compareSourceLocations(const Node* firstNode, const Node* secondNode) {
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
    Main comparison method for comparing two nodes, that exist in both ASTs, taking into account many aspects and printing the differences
*/
void TreeComparer::compareSimilarNodes(const Node* firstNode, const Node* secondNode) {
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

    // compare children of the nodes
    size_t firstChildrenSize = firstNode->children.size();
    size_t secondChildrenSize = secondNode->children.size();
    size_t minSize = std::min(firstChildrenSize, secondChildrenSize);

    for (size_t i = 0; i < minSize; ++i) {
        processNode(firstNode->children[i]);
        processNode(secondNode->children[i]);
    }

    // comparing the source locations of the nodes
    compareSourceLocations(firstNode, secondNode);
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
    Prints the subtree of a given node, recursively calling the function for its children
*/
void TreeComparer::printSubTree(const Node* node, int depth = 0) const {
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
void TreeComparer::printNodeDetails(const Node* node, std::string indent = " ") const {
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