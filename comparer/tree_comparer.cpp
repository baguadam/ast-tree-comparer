#include <iostream>
#include <stack>
#include "./headers/tree_comparer.h"
#include "./headers/utils.h"

TreeComparer::TreeComparer(Tree& firstTree, Tree& secondTree, std::unique_ptr<TreeComparerLogger> logger) : firstASTTree(firstTree), 
                                                                                                            secondASTTree(secondTree), 
                                                                                                            logger(std::move(logger)) {
    if (!firstTree.getRoot() || !secondTree.getRoot()) {
        throw std::invalid_argument("Invalid Tree object passed to TreeComparer: Root node is null.");
    }
} 

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

        if (!current) {
            std::cerr << "Error: Encountered null node during traversal in printDifferences().\n";
            continue;
        }

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
    std::string nodeKey = current->id;

    if (firstASTTree.isDeclNodeInAST(nodeKey) && secondASTTree.isDeclNodeInAST(nodeKey)) {
        // node exists in both ASTs, compare them
        if (firstASTTree.isDeclNodeProcessed(nodeKey) && secondASTTree.isDeclNodeProcessed(nodeKey)) return;  // skip if already processed

        const Node* firstASTNode = firstASTTree.getDeclNode(nodeKey);
        const Node* secondASTNode = secondASTTree.getDeclNode(nodeKey);
        compareSimilarDeclNodes(firstASTNode, secondASTNode, nodeKey);

        // compare their direct children statements
        compareStmtNodes(nodeKey);
    } else if (firstASTTree.isDeclNodeInAST(nodeKey)) {
        // node exists only in the first AST
        processNodeInSingleAST(current, firstASTTree, FIRST_AST, true);
    } else if (secondASTTree.isDeclNodeInAST(nodeKey)) {
        // node exists only in the second AST
        processNodeInSingleAST(current, secondASTTree, SECOND_AST, true);
    } else {
        // node does not exist in either AST, should not happen
        std::cerr << "Error: Node key " << nodeKey << " not found in either AST map.\n";
    }
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

        logger->logNode(firstNode, DIFFERENT_SOURCE_LOCATIONS, FIRST_AST, "  ");
        logger->logNode(secondNode, DIFFERENT_SOURCE_LOCATIONS, SECOND_AST, "  ");
    }
}

/*
Description:
    Comparison logic of two parents of the nodes
*/
void TreeComparer::compareParents(const Node* firstNode, const Node* secondNode) {
    if (firstNode->parent && (!secondNode->parent || firstNode->parent->usr != secondNode->parent->usr)) {
        logger->logNode(firstNode->parent, DIFFERENT_PARENT, FIRST_AST, "  ");
        logger->logNode(secondNode->parent, DIFFERENT_PARENT, SECOND_AST, "  ");
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

    // mark nodes as processed
    firstASTTree.markDeclNodeAsProcessed(nodeKey);
    secondASTTree.markDeclNodeAsProcessed(nodeKey);
}

/*
Description:
    Compares the statement nodes of two declaration nodes, creates a set of nodes for each AST using unique hash and equal functions,
    then compares the nodes in the first AST with the nodes in the second AST, printing the differences
*/
void TreeComparer::compareStmtNodes(const std::string& nodeKey) {
    std::vector<std::pair<std::string, Node*>> firstASTStmtVec = firstASTTree.getStmtNodes(nodeKey);
    std::vector<std::pair<std::string, Node*>> secondASTStmtVec = secondASTTree.getStmtNodes(nodeKey);

    // processed statement keys
    std::unordered_set<std::string> processedKeys;

    // map of second AST statement nodes for faster lookup
    std::unordered_map<std::string, Node*> secondASTStmtMap;
    for (const auto& [stmtKey, stmtNode] : secondASTStmtVec) {
        secondASTStmtMap[stmtKey] = stmtNode;
    }

    // iterate through the first AST vector
    for (auto& [stmtKey, stmtNode] : firstASTStmtVec) {
        if (processedKeys.find(stmtKey) != processedKeys.end()) {
            continue; // skip if already processed
        }

        auto it = secondASTStmtMap.find(stmtKey);
        if (it == secondASTStmtMap.end()) {
            // node exists only in the first AST
            processNodeInSingleAST(stmtNode, firstASTTree, FIRST_AST, false, &processedKeys);
        } else {
            // node exists in both ASTs, compare them
            compareSimilarStmtNodes(stmtNode, it->second);

            // mark nodes as processed
            processedKeys.insert(stmtKey);
        }
    }

    // iterate through the statements in the second AST that were not processed
    for (auto& [stmtKey, stmtNode] : secondASTStmtMap) {
        if (processedKeys.find(stmtKey) == processedKeys.end()) {
            processNodeInSingleAST(stmtNode, secondASTTree, SECOND_AST, false, &processedKeys);
        }
    }
}

/*
Description:
    Comparison logic of two similar statement nodes, checking for parents and source locations
*/
void TreeComparer::compareSimilarStmtNodes(const Node* firstNode, const Node* secondNode) {
    // checking for parents
    compareParents(firstNode, secondNode);
}

/*
Description:
    Processes a node that exists only in one of the ASTs, prints the details of the node and marks the subtree as processed, handles both DECLARATIONS and STATEMENTS
*/
void TreeComparer::processNodeInSingleAST(Node* current, Tree& tree, ASTId ast, bool isDeclaration, std::unordered_set<std::string>* processedKeys) {
    std::string nodeKey = isDeclaration ? current->id : Utils::getKey(current, false);

    // Check if the node is already processed for declarations
    if (isDeclaration && tree.isDeclNodeProcessed(nodeKey)) {
        return;  // Skip
    }

    // Check if the node is already processed for statements
    if (!isDeclaration && processedKeys && processedKeys->find(nodeKey) != processedKeys->end()) {
        return;  // Skip
    }

    // Lambda for processing the node
    auto processNode = [this, &tree, ast, processedKeys, isDeclaration](Node* currentNode, int depth) {
        std::string currentNodeKey = currentNode->type == DECLARATION ? currentNode->id : Utils::getKey(currentNode, false);
        if (!currentNodeKey.empty()) {
            std::string indent(depth * 3, ' ');
            DifferenceType diffType = (ast == FIRST_AST) ? ONLY_IN_FIRST_AST : ONLY_IN_SECOND_AST;

            // log the node
            logger->logNode(currentNode, diffType, ast, indent);

            // log the edge (so that the relation) between the nodes
            if (currentNode->parent) {
                logger->logEdge(currentNode, currentNode->parent, indent);
            }

            // Mark the node as processed
            if (isDeclaration) {
                if (currentNode->type == DECLARATION) {
                    tree.markDeclNodeAsProcessed(currentNodeKey);
                }
            } else if (currentNode->type == STATEMENT) {
                processedKeys->insert(currentNodeKey);
            }
        }
    };

    // traverse the subtree and process nodes accordingly
    tree.processSubTree(current, processNode);
}

/*
Description:
    Enqueues the children of a given node to the queue, only storing the Declaration types for processing
*/
void TreeComparer::enqueueChildren(Node* current, std::queue<Node*>& queue) {
    for (Node* child : current->children) {
        if (child && !child->isProcessed && child->type == DECLARATION) {
            queue.push(child);
        }
    }
}