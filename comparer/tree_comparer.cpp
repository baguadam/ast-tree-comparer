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
        processDeclNodes(current);

        // add children to the queue for further processing
        enqueueChildren(current, queue);
    }
}

/*
Descpirion:
    Processes a declaration node by comparing it with the corresponding node in the other AST, if the node exists in both ASTs, 
    compares them, otherwise processes the node that exists only in one of the ASTs.
*/
void TreeComparer::processDeclNodes(Node* current) {
    std::string nodeKey = current->enhancedKey;

    bool existsInFirstAST = firstASTTree.isDeclNodeInAST(nodeKey);
    bool existsInSecondAST = secondASTTree.isDeclNodeInAST(nodeKey);

    if (existsInFirstAST && existsInSecondAST) {
        processDeclNodesInBothASTs(nodeKey);
    } else if (existsInFirstAST) {
        processNodesInSingleAST(current, firstASTTree, FIRST_AST, true);
    } else if (existsInSecondAST) {
        processNodesInSingleAST(current, secondASTTree, SECOND_AST, true);
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
    auto firstASTStmtRange = firstASTTree.getStmtNodes(nodeKey);
    auto secondASTStmtRange = secondASTTree.getStmtNodes(nodeKey);

    // Map of second AST statement nodes for lookup
    std::unordered_map<std::string, Node*> secondASTMap;
    for (auto it = secondASTStmtRange.first; it != secondASTStmtRange.second; ++it) {
        Node* node = it->second;
        secondASTMap.emplace(it->second->enhancedKey, node);
    }

    // first pass: Identify matches and mark them
    for (auto it = firstASTStmtRange.first; it != firstASTStmtRange.second; ++it) {
        Node* stmtNode = it->second;

        if (stmtNode->isProcessed) {
            continue; // skip if already processed
        }

        auto secondNodeIt = secondASTMap.find(stmtNode->enhancedKey);
        if (secondNodeIt == secondASTMap.end()) {
            processNodesInSingleAST(stmtNode, firstASTTree, FIRST_AST, false);
        } else {
            Node* secondNode = secondNodeIt->second;

            if (!secondNode->isProcessed) { 
                compareSimilarStmtNodes(stmtNode, secondNode);
            }
        }
    }

    // second pass: process unmatched nodes in secont AST
    for (auto it = secondASTStmtRange.first; it != secondASTStmtRange.second; ++it) {
        Node* stmtNode = it->second;
        if (!stmtNode->isProcessed) {
            processNodesInSingleAST(stmtNode, secondASTTree, SECOND_AST, false);
        }
    } 
}

/*
Description:
    Comparison logic of two similar statement nodes, checking for parents and source locations
*/
void TreeComparer::compareSimilarStmtNodes(Node* firstNode, Node* secondNode) {
    // checking for parents
    compareParents(firstNode, secondNode);
    firstNode->isProcessed = true;
    secondNode->isProcessed = true;
}

/*
Description:
    Processes the declaration nodes that exist in both ASTs, by comparing them and marking them as processed, uses the iterator ranges 
    that are returned by the getDeclNodes method of the Tree class, sorts the nodes based on their topological order for proper comparison.
*/
void TreeComparer::processDeclNodesInBothASTs(const std::string& nodeKey) {
    auto firstASTRange = firstASTTree.getDeclNodes(nodeKey);
    auto secondASTRange = secondASTTree.getDeclNodes(nodeKey);

    // Check the number of nodes in both ranges
    bool isFirstSingleNode = std::distance(firstASTRange.first, firstASTRange.second) == 1;
    bool isSecondSingleNode = std::distance(secondASTRange.first, secondASTRange.second) == 1;

    if (isFirstSingleNode && isSecondSingleNode) {
        bool isFirstNodeProcessed = firstASTRange.first->second->isProcessed;
        bool isSecondNodeProcessed = secondASTRange.first->second->isProcessed;
        
        if (isFirstNodeProcessed || isSecondNodeProcessed) {
            return;  // skip already processed nodes
        }

        // GENERAL CASE: both ranges have only one node
        Node* firstNode = firstASTRange.first->second;
        Node* secondNode = secondASTRange.first->second;

        checkNodeFingerprints(firstNode, secondNode, nodeKey);
        return; // no need for further processing
    } else {
        // RARE CASE: multiple nodes with the same key
        processMultiDeclNodes(firstASTRange, secondASTRange, nodeKey);
    }
}

/*
Description:
    Processes the declaration nodes that exist in both ASTs multiple times with the same key, by comparing them and marking them as processed, uses the iterator ranges 
    that are returned by the getDeclNodes method of the Tree class, sorts the nodes based on their topological order for proper comparison.
*/
void TreeComparer::processMultiDeclNodes(const std::pair<std::unordered_multimap<std::string, Node*>::const_iterator,
                                                std::unordered_multimap<std::string, Node*>::const_iterator>& firstASTRange,
                                         const std::pair<std::unordered_multimap<std::string, Node*>::const_iterator,
                                                std::unordered_multimap<std::string, Node*>::const_iterator>& secondASTRange,
                                         const std::string& nodeKey) {
    std::vector<Node*> firstASTDeclNodes;
    std::vector<Node*> secondASTDeclNodes;

    // copy nodes from the iterator ranges to vectors for easier processing and sorting
    std::transform(firstASTRange.first, firstASTRange.second, std::back_inserter(firstASTDeclNodes), [](const auto& pair) { return pair.second; });
    std::transform(secondASTRange.first, secondASTRange.second, std::back_inserter(secondASTDeclNodes), [](const auto& pair) { return pair.second; });

    // sort the nodes based on their topological order for proper comparison
    auto comparer = [](const Node* a, const Node* b) { return a->topologicalOrder < b->topologicalOrder; };
    if (firstASTDeclNodes.size() > 1) {
        std::sort(firstASTDeclNodes.begin(), firstASTDeclNodes.end(), comparer);
    }
    if (secondASTDeclNodes.size() > 1) {
        std::sort(secondASTDeclNodes.begin(), secondASTDeclNodes.end(), comparer);
    }

    // compare nodes from both ASTs using the sorted vectors
    size_t minSize = std::min(firstASTDeclNodes.size(), secondASTDeclNodes.size());
    for (size_t i = 0; i < minSize; ++i) {
        Node* firstNode = firstASTDeclNodes[i];
        Node* secondNode = secondASTDeclNodes[i];
            
        if (firstNode->isProcessed || secondNode->isProcessed) {
            continue;  // Skip already processed nodes
        }

        checkNodeFingerprints(firstNode, secondNode, nodeKey);
    }


    // process any remaining nodes in both ASTs
    processRemainingNodes(firstASTDeclNodes.begin() + minSize, firstASTDeclNodes.end(), firstASTTree, FIRST_AST);
    processRemainingNodes(secondASTDeclNodes.begin() + minSize, secondASTDeclNodes.end(), secondASTTree, SECOND_AST);
}

/*
Description:
    Processes a node that exists only in one of the ASTs, prints the details of the node and marks the subtree as processed, handles both DECLARATIONS and STATEMENTS
*/
void TreeComparer::processNodesInSingleAST(Node* current, Tree& tree, const ASTId ast, bool isDeclaration) {
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
void TreeComparer::checkNodeFingerprints(Node* firstNode, Node* secondNode, const std::string& nodeKey) {
    if (firstNode->fingerprint == secondNode->fingerprint) {
        // additional checks in case of fingerprint collision
        if (firstNode->kind == secondNode->kind && firstNode->children.size() == secondNode->children.size()) {
            firstNode->isProcessed = true;
            secondNode->isProcessed = true;
            return;  // nodes considered identical, no further processing needed
        }
    }
    // fallback to detailed node comparison
    compareSimilarDeclNodes(firstNode, secondNode, nodeKey);
}

/*
Description:
    Processes the remaining nodes in the vector, starting from the given index, in the given AST, handles both DECLARATIONS and STATEMENTS
*/
void TreeComparer::processRemainingNodes(std::vector<Node*>::const_iterator begin, 
                                         std::vector<Node*>::const_iterator end, 
                                         Tree& tree, const ASTId ast) {
    for (auto it = begin; it != end; ++it) {
        Node* node = *it;
        if (!node->isProcessed) {
            processNodesInSingleAST(node, tree, ast, node->type == DECLARATION);
        }
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