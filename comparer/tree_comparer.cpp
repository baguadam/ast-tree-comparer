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
        processDeclNode(current);
    }
}

/*
Descpirion:
    Processes a declaration node by comparing it with the corresponding node in the other AST, if the node exists in both ASTs, 
    compares them, otherwise processes the node that exists only in one of the ASTs.
*/
void TreeComparer::processDeclNode(Node* current) {
    std::string nodeKey = Utils::getKey(current, current->type == "Declaration");

    if (firstASTTree.isDeclNodeInAST(nodeKey) && secondASTTree.isDeclNodeInAST(nodeKey)) {
        // node exists in both ASTs, compare them
        if (firstASTTree.isDeclNodeProcessed(nodeKey) && secondASTTree.isDeclNodeProcessed(nodeKey)) return;  // skip if already processed

        const Node* firstASTNode = firstASTTree.getDeclNode(nodeKey);
        const Node* secondASTNode = secondASTTree.getDeclNode(nodeKey);
        compareSimilarDeclNodes(firstASTNode, secondASTNode, nodeKey);

        // mark nodes as processed
        firstASTTree.markDeclNodeAsProcessed(nodeKey);
        secondASTTree.markDeclNodeAsProcessed(nodeKey);
    } else if (firstASTTree.isDeclNodeInAST(nodeKey)) {
        // node exists only in the first AST
        processDeclNodeInFirstAST(current, nodeKey);
    } else if (secondASTTree.isDeclNodeInAST(nodeKey)) {
        // node exists only in the second AST
        processDeclNodeInSecondAST(current, nodeKey);
    } else {
        // node does not exist in either AST, should not happen
        std::cerr << "Error: Node key " << nodeKey << " not found in either AST map.\n";
    }
}

/*
Description:
    Processes a node that exists only in one of the ASTs, prints the details of the node and marks the subtree as processed
*/
void TreeComparer::processDeclNodeInSingleAST(Node* current, const std::string& nodeKey, Tree& tree, const char* astName) {
    if (tree.isDeclNodeProcessed(nodeKey)) return;  // skip if already processed

    std::cout << "Node " << nodeKey << " only exists in the " << astName << " AST, skipping its children\n";
    printSubTree(current, 0);
    tree.markSubTreeAsProcessed(current);  // mark entire subtree as processed
    printSeparators();
}

/*
Description:
    Processes the node that only exists in the first AST, prints the details of the node
*/
void TreeComparer::processDeclNodeInFirstAST(Node* current, const std::string& nodeKey) {
    processDeclNodeInSingleAST(current, nodeKey, firstASTTree, "=FIRST=");
}

/*
Description:
    Processes the node that only exists in the second AST, prints the details of the node
*/
void TreeComparer::processDeclNodeInSecondAST(Node* current, const std::string& nodeKey) {
    processDeclNodeInSingleAST(current, nodeKey, secondASTTree, "=SECOND=");
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
        std::cout << "Node with key: " << Utils::getKey(firstNode, firstNode->type == "Declaration") << " has different source locations in the trees.\n";
        std::cout << "First AST location: " << firstNode->path << ":" << firstNode->lineNumber << ":" << firstNode->columnNumber << '\n';
        std::cout << "Second AST location: " << secondNode->path << ":" << secondNode->lineNumber << ":" << secondNode->columnNumber << '\n';
        
        printSeparators();
    }
}

/*
Description:
    Comparison logic of two parents of the nodes
*/
void TreeComparer::compareParents(const Node* firstNode, const Node* secondNode) {
    if (firstNode->parent && (!secondNode->parent || firstNode->parent->usr != secondNode->parent->usr)) {
        std::cout << "Node " << firstNode->kind << " " << firstNode->usr << " " << firstNode->path << " " << firstNode->lineNumber << ":" << firstNode->columnNumber
                  << " has a different parent in second AST: " << secondNode->parent->usr << "\n";
        std::cout << "First AST parent details:\n";
        printNodeDetails(firstNode->parent, " ");
        std::cout << "Second AST parent details:\n";
        printNodeDetails(secondNode->parent, " ");

        printSeparators();
    }
}

/*
Description:
    Main comparison method for comparing two nodes, that exist in both ASTs, taking into account many aspects and printing the differences
*/
void TreeComparer::compareSimilarDeclNodes(const Node* firstNode, const Node* secondNode, const std::string& nodeKey) {
    // checking for parents
    compareParents(firstNode, secondNode);

    // comparing the source locations of the nodes
    compareSourceLocations(firstNode, secondNode);

    // compare their direct children statements
    compareStmtNodes(nodeKey);
}

/*
Description:
    Compares the statement nodes of two declaration nodes, creates a set of nodes for each AST using unique hash and equal functions,
    then compares the nodes in the first AST with the nodes in the second AST, printing the differences
*/
void TreeComparer::compareStmtNodes(const std::string& nodeKey) {
    std::vector<Node*> firstStmtNodes = firstASTTree.getStmtNodes(nodeKey);
    std::vector<Node*> secondStmtNodes = secondASTTree.getStmtNodes(nodeKey);

    std::unordered_set<Node*, NodeHash, NodeEqual> firstStmtSet(firstStmtNodes.begin(), firstStmtNodes.end());
    std::unordered_set<Node*, NodeHash, NodeEqual> secondStmtSet(secondStmtNodes.begin(), secondStmtNodes.end());

    // Iterate over the first set and check for differences
    for (Node* node : firstStmtNodes) {
        if (firstStmtSet.count(node) > 0) {
            if (secondStmtSet.count(node) == 0) {
                std::cout << "Node with key: " << nodeKey << " has a statement node in the first AST that does not exist in the second AST.\n";
                printSubTree(node, 0);
                printSeparators();

                // remove the children and and the descendants from the set
                removeNodeAndDescendatsFromSet(node, firstStmtSet);
            } else {
                auto it = secondStmtSet.find(node);
                if (it != secondStmtSet.end()) {
                    compareSimilarStmtNodes(node, *it);
                    secondStmtSet.erase(it);
                }
            }
        }
    }

    // Iterate over the second set to print the remaining nodes
    for (Node* node : secondStmtSet) {
        std::cout << "Node with key: " << nodeKey << " has a statement node in the second AST that does not exist in the first AST.\n";
        printNodeDetails(node, " ");
        printSeparators();
    }
}

/*
Descpiption:
    Compares two similar statement nodes, checking for parents and source locations 
*/
void TreeComparer::compareSimilarStmtNodes(const Node* firstStmtNode, const Node* secondStmtNode) {
    // checking for parent
    compareParents(firstStmtNode, secondStmtNode);

    // compare the source locations of the nodes
    compareSourceLocations(firstStmtNode, secondStmtNode);
}

/*
Description:
    Removes a node and its descendants from the set
*/
void TreeComparer::removeNodeAndDescendatsFromSet(Node* node, std::unordered_set<Node*, NodeHash, NodeEqual>& set) {
    for (Node* child : node->children) {
        removeNodeAndDescendatsFromSet(child, set);
    }
    set.erase(node);
}

/*
Description:
    Enqueues the children of a given node to the queue, only storing the Declaration types for processing
*/
void TreeComparer::enqueueChildren(Node* current, std::queue<Node*>& queue) {
    for (Node* child : current->children) {
        if (child && child->type == "Declaration") {
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
    std::cout << indent << "Node details:\n";
    std::cout << indent << node->kind << " " << node->type << " " << node->usr << " " << node->path << " " << node->lineNumber << ":" << node->columnNumber << "\n";
    std::cout << indent << "*** Parent unique id: " << (node->parent ? Utils::getKey(node->parent, node->parent->type == "Declaration") : "None") << "\n";
    
    printSeparators();
}

/*
Description:
    Prints a separator for better readability
*/
void TreeComparer::printSeparators() const {
    std::cout << "**********************************************************\n";
}