#include <iostream>
#include <stack>
#include <algorithm>
#include "../include/tree_comparer.h"
#include "../include/utils.h"

TreeComparer::TreeComparer(Tree& firstTree, Tree& secondTree, IDatabaseWrapper& db) 
    : firstASTTree(firstTree), 
      secondASTTree(secondTree), 
      dbWrapper(db),
      topologicalComparer([](const Node* a, const Node* b) { return a->topologicalOrder < b->topologicalOrder; }) {
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

    // send the remaining nodes from the batch
    dbWrapper.finalize();
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

        // logger->logNode(firstNode, DIFFERENT_SOURCE_LOCATIONS, FIRST_AST);
        // logger->logNode(secondNode, DIFFERENT_SOURCE_LOCATIONS, SECOND_AST);

        std::string differenceTypeStr = Utils::differenceTypeToString(DIFFERENT_SOURCE_LOCATIONS);

        dbWrapper.addNodeToBatch(*firstNode, true, differenceTypeStr, Utils::astIdToString(FIRST_AST));
        dbWrapper.addNodeToBatch(*secondNode, true, differenceTypeStr, Utils::astIdToString(SECOND_AST));
    }
}

/*
Description:
    Comparison logic of two parents of the nodes
*/
void TreeComparer::compareParents(const Node* firstNode, const Node* secondNode) {
    if (firstNode->parent && (!secondNode->parent || firstNode->parent->usr != secondNode->parent->usr)) {

        // logger->logNode(firstNode->parent, DIFFERENT_PARENT, FIRST_AST);
        // logger->logNode(secondNode->parent, DIFFERENT_PARENT, SECOND_AST);

        std::string differenceTypeStr = Utils::differenceTypeToString(DIFFERENT_PARENT);

        dbWrapper.addNodeToBatch(*firstNode, true, differenceTypeStr, Utils::astIdToString(FIRST_AST));
        dbWrapper.addNodeToBatch(*secondNode, true, differenceTypeStr, Utils::astIdToString(SECOND_AST));
    }
}

/*
Description:
    Main comparison method for comparing two nodes, that exist in both ASTs, taking into account many aspects and printing the differences
*/
void TreeComparer::compareSimilarDeclNodes(Node* firstNode, Node* secondNode) {
    // checking for parents
    compareParents(firstNode, secondNode);

    // comparing the source locations of the nodes
    compareSourceLocations(firstNode, secondNode);

    // compare their statement nodes
    compareStmtNodes(firstNode, secondNode);

    // mark nodes as processed
    firstNode->isProcessed = true;
    secondNode->isProcessed = true;
}

/*
Description:
    Compares the statement nodes of two declaration nodes, creates a set of nodes for each AST using unique hash and equal functions,
    then compares the nodes in the first AST with the nodes in the second AST, printing the differences.
*/
void TreeComparer::compareStmtNodes(const Node* firstNode, const Node* secondNode) {
    std::string firstNodeStmtKey = firstNode->enhancedKey + "|" + std::to_string(firstNode->topologicalOrder);
    std::string secondNodeStmtKey = secondNode->enhancedKey + "|" + std::to_string(secondNode->topologicalOrder);
    auto firstASTStmtRange = firstASTTree.getStmtNodes(firstNodeStmtKey);
    auto secondASTStmtRange = secondASTTree.getStmtNodes(secondNodeStmtKey);

    // first pass: identifying matches and marking them 
    for (auto it1 = firstASTStmtRange.first; it1 != firstASTStmtRange.second; ++it1) {
        Node* stmtNodeFirst = *it1;

        if (stmtNodeFirst->isProcessed) {
            continue; // skip if already processed
        }

        bool foundMatch = false;
        for (auto it2 = secondASTStmtRange.first; it2 != secondASTStmtRange.second; ++it2) {
            Node* stmtNodeSecond = *it2;

            if (stmtNodeSecond->isProcessed) {
                continue; // skip from second if already processed
            }

            if (stmtNodeFirst->enhancedKey == stmtNodeSecond->enhancedKey) {
                compareParents(stmtNodeFirst, stmtNodeSecond);
                compareSourceLocations(stmtNodeFirst, stmtNodeSecond);

                stmtNodeFirst->isProcessed = true;
                stmtNodeSecond->isProcessed = true;
                foundMatch = true;
                break; // exit inner loop if found
            }
        }

        if (!foundMatch) {
            processNodesInSingleAST(stmtNodeFirst, firstASTTree, FIRST_AST, false);
        }
    }

    // second pass: unmatched nodes in second AST
    for (auto it = secondASTStmtRange.first; it != secondASTStmtRange.second; ++it) {
        Node* stmtNode = *it;
        if (!stmtNode->isProcessed) {
            processNodesInSingleAST(stmtNode, secondASTTree, SECOND_AST, false);
        }
    }
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

        compareSimilarDeclNodes(firstNode, secondNode);      
        return; // no need for further processing
    } else {
        // RARE CASE: multiple nodes with the same key
        processMultiDeclNodes(firstASTRange, secondASTRange);
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
                                                std::unordered_multimap<std::string, Node*>::const_iterator>& secondASTRange) {
    std::vector<Node*> firstASTDeclNodes;
    std::vector<Node*> secondASTDeclNodes;

    // copy nodes from the iterator ranges to vectors for easier processing and sorting
    std::transform(firstASTRange.first, firstASTRange.second, std::back_inserter(firstASTDeclNodes), [](const auto& pair) { return pair.second; });
    std::transform(secondASTRange.first, secondASTRange.second, std::back_inserter(secondASTDeclNodes), [](const auto& pair) { return pair.second; });

    // sort the nodes based on their topological order for proper comparison
    if (firstASTDeclNodes.size() > 1) {
        std::sort(firstASTDeclNodes.begin(), firstASTDeclNodes.end(), topologicalComparer);
    }
    if (secondASTDeclNodes.size() > 1) {
        std::sort(secondASTDeclNodes.begin(), secondASTDeclNodes.end(), topologicalComparer);
    }

    // compare nodes from both ASTs using the sorted vectors
    size_t minSize = std::min(firstASTDeclNodes.size(), secondASTDeclNodes.size());
    for (size_t i = 0; i < minSize; ++i) {
        Node* firstNode = firstASTDeclNodes[i];
        Node* secondNode = secondASTDeclNodes[i];
            
        if (firstNode->isProcessed || secondNode->isProcessed) {
            continue;  // Skip already processed nodes
        }

        compareSimilarDeclNodes(firstNode, secondNode);
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

    // curresponsing AST for node checking in the lambda
    Tree& correspondingASTTree = (ast == FIRST_AST) ? secondASTTree : firstASTTree;

    // Lambda for processing the node
    auto processNode = [this, ast, &correspondingASTTree](Node* currentNode, int depth) {
        std::string currentNodeKey = currentNode->enhancedKey;
        bool existsInCorrespondingAST = correspondingASTTree.isDeclNodeInAST(currentNodeKey);

        // don't mark and print nodes in the subtree that exists in both AST, leave them for further comparison
        if (existsInCorrespondingAST && currentNode->isProcessed) {
            return;  // skip
        } 

        // if the node does not exist in the other AST, log it and mark it as processed as part of the subtree
        currentNode->isProcessed = true;
        const DifferenceType diffType = (ast == FIRST_AST) ? ONLY_IN_FIRST_AST : ONLY_IN_SECOND_AST;

        // logger->logNode(currentNode, diffType, ast, depth); // log the node
        this->dbWrapper.addNodeToBatch(*currentNode, depth == 0, Utils::differenceTypeToString(diffType), Utils::astIdToString(ast)); // set it as part of the subtree (at this point cannot be hightest level node)

        // parent-child relationships for the subtree, skipping the "root" node
        if (currentNode->parent && depth != 0) {
            dbWrapper.addRelationshipToBatch(*currentNode->parent, *currentNode);
        }
    };

    // traverse the subtree and process nodes accordingly
    tree.processSubTree(current, processNode);
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