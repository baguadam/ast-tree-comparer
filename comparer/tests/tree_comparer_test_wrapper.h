#ifndef _TREE_COMPARER_TEST_WRAPPER_H_

#define _TREE_COMPARER_TEST_WRAPPER_H
#include "../headers/tree_comparer.h"

class TreeComparerTestWrapper : public TreeComparer {
public:
    using TreeComparer::compareSourceLocations;
    using TreeComparer::compareParents;
    using TreeComparer::compareSimilarDeclNodes;
    using TreeComparer::processNodesInSingleAST;
    
    using TreeComparer::processDeclNodes;

    TreeComparerTestWrapper(Tree& t1, Tree& t2, IDatabaseWrapper& dbWrapper)
        : TreeComparer(t1, t2, dbWrapper) { }
};

#endif