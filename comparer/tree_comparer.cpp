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

        // process the node
        processDeclNode(current);

        // add children to the queue for further processing
        enqueueChildren(current, queue);
    }
}

/*
Descpirion:
    Processes a declaration node by comparing it with the corresponding node in the other AST, if the node exists in both ASTs, 
    compares them, otherwise processes the node that exists only in one of the ASTs.
*/
void TreeComparer::processDeclNode(Node* current) {
    std::string nodeKey = current->enhancedKey;

    bool existsInFirstAST = firstASTTree.isDeclNodeInAST(nodeKey);
    bool existsInSecondAST = secondASTTree.isDeclNodeInAST(nodeKey);

    if (existsInFirstAST && existsInSecondAST) {
        processDeclNodeInBothASTs(nodeKey);
    } else if (existsInFirstAST) {
        processNodeInSingleAST(current, firstASTTree, FIRST_AST, true);
    } else if (existsInSecondAST) {
        processNodeInSingleAST(current, secondASTTree, SECOND_AST, true);
    } else {
        // should not happen! 
        std::cerr << "Error: Node with key " << nodeKey << " does not exist in any of the ASTs.\n";
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

        logger->logNode(firstNode, DIFFERENT_SOURCE_LOCATIONS, FIRST_AST);
        logger->logNode(secondNode, DIFFERENT_SOURCE_LOCATIONS, SECOND_AST);
    }
}

/*
Description:
    Comparison logic of two parents of the nodes
*/
void TreeComparer::compareParents(const Node* firstNode, const Node* secondNode) {
    if (firstNode->parent && (!secondNode->parent || firstNode->parent->usr != secondNode->parent->usr)) {
        logger->logNode(firstNode->parent, DIFFERENT_PARENT, FIRST_AST);
        logger->logNode(secondNode->parent, DIFFERENT_PARENT, SECOND_AST);
    }
}

/*
Description:
    Main comparison method for comparing two nodes, that exist in both ASTs, taking into account many aspects and printing the differences
*/
void TreeComparer::compareSimilarDeclNodes(Node* firstNode, Node* secondNode, const std::string& nodeKey) {
    // checking for parents
    compareParents(firstNode, secondNode);

    // comparing the source locations of the nodes
    compareSourceLocations(firstNode, secondNode);

    // compare their statement nodes
    compareStmtNodes(nodeKey);

    // mark nodes as processed
    firstNode->isProcessed = true;
    secondNode->isProcessed = true;
}

/*
Description:
    Compares the statement nodes of two declaration nodes, creates a set of nodes for each AST using unique hash and equal functions,
    then compares the nodes in the first AST with the nodes in the second AST, printing the differences.
*/
void TreeComparer::compareStmtNodes(const std::string& nodeKey) {
    std::vector<Node*> firstAstStmtNodes = firstASTTree.getStmtNodes(nodeKey);
    std::vector<Node*> secondAstStmtNodes = secondASTTree.getStmtNodes(nodeKey);    

    // Map of second AST statement nodes for lookup
    std::unordered_multimap<std::string, Node*> secondASTStmtMultiMap;
    for (Node* node : secondAstStmtNodes) {
        secondASTStmtMultiMap.emplace(node->enhancedKey, node);
    }

    // first pass: Identify matches and mark them
    for (Node* stmtNode : firstAstStmtNodes) {
        if (stmtNode->isProcessed) {
            continue; // skip if already processed
        }

        auto range = secondASTStmtMultiMap.equal_range(stmtNode->enhancedKey);
        for (auto it = range.first; it != range.second; ++it) {
            Node* matchingNode = it->second;
            if (stmtNode->fingerprint == matchingNode->fingerprint) {
                stmtNode->isProcessed = true;
                matchingNode->isProcessed = true;
                break; // one match is needed
            }
        }
    }

    // second pass: Process unmatched nodes
    for (Node* stmtNode : firstAstStmtNodes) {
        if (!stmtNode->isProcessed) {
            processNodeInSingleAST(stmtNode, firstASTTree, FIRST_AST, false);
        }
    }

    for (Node* stmtNode : secondAstStmtNodes) {
        if (!stmtNode->isProcessed) {
            processNodeInSingleAST(stmtNode, secondASTTree, SECOND_AST, false);
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

*/
void TreeComparer::processDeclNodeInBothASTs(const std::string& nodeKey) {
    std::vector<Node*> firstASTDeclNodes = firstASTTree.getDeclNodes(nodeKey);
    std::vector<Node*> secondASTDeclNodes = secondASTTree.getDeclNodes(nodeKey);

    // sort the nodes based on their topological order
    auto comparer = [](const Node* a, const Node* b) { return a->topologicalOrder < b->topologicalOrder; };
    std::sort(firstASTDeclNodes.begin(), firstASTDeclNodes.end(), comparer);
    std::sort(secondASTDeclNodes.begin(), secondASTDeclNodes.end(), comparer);
    
    // comparing the nodes one by one based on the topological order
    size_t minSize = std::min(firstASTDeclNodes.size(), secondASTDeclNodes.size());
    for (size_t i = 0; i < minSize; ++i) {
        Node* firstNode = firstASTDeclNodes[i];
        Node* secondNode = secondASTDeclNodes[i];

        if (firstNode->fingerprint == secondNode->fingerprint) {
            // additional check for the same fingerprint (for key collisions)
            if (firstNode->kind == secondNode->kind && firstNode->children.size() == secondNode->children.size()) {
                firstNode->isProcessed = true;
                secondNode->isProcessed = true;
                continue; // skip if they have the same fingerprint
            }
        }

        compareSimilarDeclNodes(firstASTDeclNodes[i], secondASTDeclNodes[i], nodeKey);
    }

    if (firstASTDeclNodes.size() > minSize) {
        processRemainingNodes(firstASTDeclNodes, firstASTTree, FIRST_AST, minSize, true);
    }

    if (secondASTDeclNodes.size() > minSize) {
        processRemainingNodes(secondASTDeclNodes, secondASTTree, SECOND_AST, minSize, true);
    }
}

/*
Description:
    Processes a node that exists only in one of the ASTs, prints the details of the node and marks the subtree as processed, handles both DECLARATIONS and STATEMENTS
*/
void TreeComparer::processNodeInSingleAST(Node* current, Tree& tree, const ASTId ast, bool isDeclaration) {
    if (current->isProcessed) {
        return;  // skip
    }

    std::string nodeKey = current-> enhancedKey;

    // Lambda for processing the node
    auto processNode = [this, ast](Node* currentNode, int depth) {
        const DifferenceType diffType = (ast == FIRST_AST) ? ONLY_IN_FIRST_AST : ONLY_IN_SECOND_AST;
        logger->logNode(currentNode, diffType, ast, depth); // log the node
        currentNode->isProcessed = true;
    };

    // traverse the subtree and process nodes accordingly
    tree.processSubTree(current, processNode);
}

/*
Description:
    Processes the remaining nodes in the vector, starting from the given index, in the given AST, handles both DECLARATIONS and STATEMENTS
*/
void TreeComparer::processRemainingNodes(const std::vector<Node*>& nodes, Tree& tree, const ASTId ast, size_t index, bool isDeclaration) {
    for (size_t i = index; i < nodes.size(); ++i) {
        processNodeInSingleAST(nodes[i], tree, ast, isDeclaration);
    }
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