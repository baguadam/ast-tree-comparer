#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../headers/tree.h"
#include "../headers/tree_comparer.h"
#include "../headers/node_utilities.h"
#include "mock_database_wrapper.h"
#include "partial_tree_comparer.h"
#include "tree_comparer_test_wrapper.h"
#include <fstream>
#include <filesystem>

using ::testing::_;
using ::testing::Exactly;
using ::testing::AtLeast;

class TreeComparerTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::ofstream testFile1("test_ast_1.txt");
        ASSERT_TRUE(testFile1.is_open());
        testFile1 << "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n";
        testFile1 << " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n";
        testFile1 << "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n";     
        testFile1 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n";
        testFile1 << "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6\n";
        testFile1 << "    Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7\n";
        testFile1.close();

        std::ofstream testFile2("test_ast_2.txt");
        ASSERT_TRUE(testFile2.is_open());
        testFile2 << "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n";
        testFile2 << " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n";
        testFile2 << "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n";
        testFile2 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n";
        testFile2 << "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t401\t6\n";
        testFile2 << "    Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t404\t7\n";
        testFile2.close();

        ASSERT_TRUE(std::filesystem::exists("test_ast_1.txt"));
        ASSERT_TRUE(std::filesystem::exists("test_ast_2.txt"));
    }

    void TearDown() override {
        if (std::filesystem::exists("test_ast_1.txt")) {
            std::filesystem::remove("test_ast_1.txt");
        }
        if (std::filesystem::exists("test_ast_2.txt")) {
            std::filesystem::remove("test_ast_2.txt");
        }
    }
  
    // accessible for all tests
    MockDatabaseWrapper dbWrapper;
};
// **********************************************
// HELPERS
// **********************************************
// helper function to create a node
Node createNode(NodeType type, const std::string& kind, const std::string& usr,
                const std::string& path, int lineNumber, int columnNumber,
                Node* parent = nullptr) {
    Node node;
    node.type = type;
    node.kind = kind;
    node.usr = usr;
    node.path = path;
    node.lineNumber = lineNumber;
    node.columnNumber = columnNumber;
    node.parent = parent;
    node.enhancedKey = kind + "|" + usr + "|" + path + "|";
    return node;
}

// **********************************************
// TreeComparer unit tests - printDifferences
// **********************************************
TEST_F(TreeComparerTest, PrintDifferencesFinalizesDatabase) {
    Tree tree1("test_ast_1.txt");
    Tree tree2("test_ast_2.txt");

    EXPECT_CALL(dbWrapper, finalize()).Times(Exactly(1));
    TreeComparer comparer(tree1, tree2, dbWrapper);

    comparer.printDifferences();
}

// **********************************************
// TreeComparer unit tests - compareSourceLocations
// **********************************************
// Test for comparing source locations when nodes have differing paths
TEST_F(TreeComparerTest, CompareDeclSourceLocations_Same) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    // nodes with the same source
    Node firstNode = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\other_config.h", 308, 1);
    Node secondNode = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\other_config.h", 308, 1);

    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, _, _)).Times(Exactly(0));

    comparer.compareSourceLocations(&firstNode, &secondNode);
}

TEST_F(TreeComparerTest, CompareSourceLocations_DifferingPaths) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    // nodes with differing paths
    Node firstNode = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\c++config.h", 308, 1);
    Node secondNode = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\other_config.h", 308, 1);

    EXPECT_CALL(dbWrapper, addNodeToBatch(firstNode, true, "DIFFERENT_SOURCE_LOCATIONS", "FIRST_AST")).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, addNodeToBatch(secondNode, true, "DIFFERENT_SOURCE_LOCATIONS", "SECOND_AST")).Times(Exactly(1));

    comparer.compareSourceLocations(&firstNode, &secondNode);
}

// Test for comparing source locations when nodes have differing paths
TEST_F(TreeComparerTest, CompareSourceLocations_DifferingLineNumbers) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    // nodes with differing line numbers
    Node firstNode = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\other_config.h", 310, 1);
    Node secondNode = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\other_config.h", 308, 1);

    EXPECT_CALL(dbWrapper, addNodeToBatch(firstNode, true, "DIFFERENT_SOURCE_LOCATIONS", "FIRST_AST")).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, addNodeToBatch(secondNode, true, "DIFFERENT_SOURCE_LOCATIONS", "SECOND_AST")).Times(Exactly(1));

    comparer.compareSourceLocations(&firstNode, &secondNode);
}

// Test for comparing source locations when nodes have differing paths
TEST_F(TreeComparerTest, CompareSourceLocations_DifferingColumnNumbers) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    // nodes with differing column numbers
    Node firstNode = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\other_config.h", 308, 1);
    Node secondNode = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\other_config.h", 308, 2);

    EXPECT_CALL(dbWrapper, addNodeToBatch(firstNode, true, "DIFFERENT_SOURCE_LOCATIONS", "FIRST_AST")).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, addNodeToBatch(secondNode, true, "DIFFERENT_SOURCE_LOCATIONS", "SECOND_AST")).Times(Exactly(1));

    comparer.compareSourceLocations(&firstNode, &secondNode);
}

// Test for comparing source locations when nodes have differing paths
TEST_F(TreeComparerTest, CompareSourceLocations_DifferingEverything) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    // nodes with differing path, line number and column number
    Node firstNode = createNode(STATEMENT, "CompoundStmt", "N/A", "C:\\include\\bits\\first.h", 10, 1);
    Node secondNode = createNode(STATEMENT, "CompoundStmt", "N/A", "C:\\include\\bits\\other_config.h", 308, 2);

    EXPECT_CALL(dbWrapper, addNodeToBatch(firstNode, true, "DIFFERENT_SOURCE_LOCATIONS", "FIRST_AST")).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, addNodeToBatch(secondNode, true, "DIFFERENT_SOURCE_LOCATIONS", "SECOND_AST")).Times(Exactly(1));

    comparer.compareSourceLocations(&firstNode, &secondNode);
}

// **********************************************
// TreeComparer unit tests - compareParents 
// **********************************************
// Test for comparing parent nodes when both nodes have the same parent (using Declaration nodes)
TEST_F(TreeComparerTest, CompareParents_SameParentDeclaration) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    Node parentNode = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\c++config.h", 308, 1);

    Node firstNode = createNode(DECLARATION, "Typedef", "c:@N@std@T@size_t", "C:\\include\\bits\\c++config.h", 310, 3, &parentNode);
    Node secondNode = createNode(DECLARATION, "Typedef", "c:@N@std@T@size_t", "C:\\include\\bits\\c++config.h", 310, 3, &parentNode);

    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, _, _)).Times(Exactly(0));

    comparer.compareParents(&firstNode, &secondNode);
}

// Test for comparing parent nodes when one node has no parent (using Statement nodes)
TEST_F(TreeComparerTest, CompareParents_OneNodeHasNoParentStatement) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    Node parentNode = createNode(STATEMENT, "IfStmt", "N/A", "C:\\include\\bits\\c++config.h", 308, 1);

    Node firstNode = createNode(STATEMENT, "CompoundStmt", "N/A", "C:\\include\\bits\\c++config.h", 310, 3, &parentNode);
    Node secondNode = createNode(STATEMENT, "CompoundStmt", "N/A", "C:\\include\\bits\\c++config.h", 310, 3);

    EXPECT_CALL(dbWrapper, addNodeToBatch(firstNode, true, "DIFFERENT_PARENTS", "FIRST_AST")).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, addNodeToBatch(secondNode, true, "DIFFERENT_PARENTS", "SECOND_AST")).Times(Exactly(1));

    comparer.compareParents(&firstNode, &secondNode);
}

// Test for comparing parent nodes when both nodes have different parents (using Declaration and Statement nodes)
TEST_F(TreeComparerTest, CompareParents_DifferentParentsMixed) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    Node parentNode1 = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\c++config.h", 308, 1);
    Node parentNode2 = createNode(STATEMENT, "IfStmt", "N/A", "C:\\include\\bits\\other_config.h", 310, 1);

    Node firstNode = createNode(DECLARATION, "Typedef", "c:@N@std@T@size_t", "C:\\include\\bits\\c++config.h", 310, 3, &parentNode1);
    Node secondNode = createNode(STATEMENT, "CompoundStmt", "N/A", "C:\\include\\bits\\other_config.h", 310, 3, &parentNode2);

    EXPECT_CALL(dbWrapper, addNodeToBatch(firstNode, true, "DIFFERENT_PARENTS", "FIRST_AST")).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, addNodeToBatch(secondNode, true, "DIFFERENT_PARENTS", "SECOND_AST")).Times(Exactly(1));

    comparer.compareParents(&firstNode, &secondNode);
}

// Test for comparing parent nodes when both nodes have no parents (using Statement nodes)
TEST_F(TreeComparerTest, CompareParents_NoParentsStatement) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    Node firstNode = createNode(STATEMENT, "ReturnStmt", "N/A", "C:\\include\\bits\\c++config.h", 310, 3);
    Node secondNode = createNode(STATEMENT, "ReturnStmt", "N/A", "C:\\include\\bits\\c++config.h", 310, 3);

    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, _, _)).Times(Exactly(0));

    comparer.compareParents(&firstNode, &secondNode);
}

// Test for comparing parent nodes when both nodes have different parents (using mixed Statement and Declaration nodes)
TEST_F(TreeComparerTest, CompareParents_DifferentParentsMixedStatementDeclaration) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    TreeComparerTestWrapper comparer(dummyTree1, dummyTree2, dbWrapper);

    Node parentNode1 = createNode(DECLARATION, "Namespace", "c:@N@std", "C:\\include\\bits\\first.h", 10, 1);
    Node parentNode2 = createNode(STATEMENT, "WhileStmt", "N/A", "C:\\include\\bits\\other_config.h", 308, 1);

    Node firstNode = createNode(STATEMENT, "CompoundStmt", "N/A", "C:\\include\\bits\\first.h", 12, 1, &parentNode1);
    Node secondNode = createNode(STATEMENT, "ExprStmt", "N/A", "C:\\include\\bits\\other_config.h", 310, 2, &parentNode2);

    EXPECT_CALL(dbWrapper, addNodeToBatch(firstNode, true, "DIFFERENT_PARENTS", "FIRST_AST")).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, addNodeToBatch(secondNode, true, "DIFFERENT_PARENTS", "SECOND_AST")).Times(Exactly(1));

    comparer.compareParents(&firstNode, &secondNode);
}