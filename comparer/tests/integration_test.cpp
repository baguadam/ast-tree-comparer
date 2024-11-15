#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../headers/tree.h"
#include "../headers/tree_comparer.h"
#include "mock_database_wrapper.h"
#include "tree_comparer_test_wrapper.h"
#include <fstream>
#include <filesystem>

using ::testing::_;
using ::testing::Exactly;
using ::testing::AtLeast;

class IntegrationTest : public ::testing::Test {
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

    // helper function to check database calls:
    void checkDatabaseCalls(const char* firstAst, const char* secondAst, int addNodeCalls, int addRelationshipCalls) {
        Tree firstTree(firstAst);
        Tree secondTree(secondAst);

        MockDatabaseWrapper dbWrapper; // mock database wrapper

        EXPECT_CALL(dbWrapper, clearDatabase()).Times(Exactly(1));
        EXPECT_CALL(dbWrapper, finalize()).Times(Exactly(1));
        EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, _, _)).Times(Exactly(addNodeCalls));
        EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(addRelationshipCalls)); 

        TreeComparer comparer(firstTree, secondTree, dbWrapper);

        // run comparison
        comparer.printDifferences(); 
    }

    // accessible for all tests
    MockDatabaseWrapper dbWrapper;
};

// **********************************************
// HELPERS
// **********************************************
// equality operator for Node
bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.type == rhs.type &&
           lhs.kind == rhs.kind &&
           lhs.usr == rhs.usr &&
           lhs.path == rhs.path &&
           lhs.lineNumber == rhs.lineNumber &&
           lhs.columnNumber == rhs.columnNumber &&
           lhs.topologicalOrder == rhs.topologicalOrder &&
           lhs.enhancedKey == rhs.enhancedKey &&
           lhs.isProcessed == rhs.isProcessed &&
           lhs.children.size() == rhs.children.size();
}

// **********************************************
// DATABASE OPERATIONS
// **********************************************
TEST_F(IntegrationTest, TestDatabaseOperationCallsWithDifferingASTs) {
    checkDatabaseCalls("test_ast_1.txt", "test_ast_2.txt", 4, 2);
}

// Test for database operation calls with the same ASTs
TEST_F(IntegrationTest, TestDatabaseOperationCallsWithSameASTs) {
    checkDatabaseCalls("test_ast_1.txt", "test_ast_1.txt", 0, 0);
}

// **********************************************
// compareStmtNodes
// **********************************************
// **********************************************
// Integration Test for compareStmtNodes
// **********************************************
TEST_F(IntegrationTest, CompareStmtNodesDifferentTrees) {
    Tree firstAstTree("test_ast_1.txt");
    Tree secondAstTree("test_ast_2.txt");

    TreeComparerTestWrapper comparer(firstAstTree, secondAstTree, dbWrapper);

    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|350|5";
    auto declNodeFirst = firstAstTree.getDeclNodes(enhancedKey); // get the decl node related to the key
    auto declNodeSecond = secondAstTree.getDeclNodes(enhancedKey); // get the decl node related to the key
    ASSERT_NE(declNodeFirst.first, declNodeSecond.second);
    

    std::string stmtKey = enhancedKey + "|" + std::to_string(declNode.first->second->topologicalOrder);
    auto stmtNodes = testTree.getStmtNodes(stmtKey);

    // Get nodes from the first AST and second AST for the given key.
    auto firstASTStmtRange = firstAstTree.getStmtNodes(nodeKey);
    auto secondASTStmtRange = secondAstTree.getStmtNodes(nodeKey);

    // Find nodes from each tree that we are expecting to match or be different.
    // Node 1 - CompoundStmt node in both ASTs, but located at different line numbers (351 vs 401).
    Node* firstNode1 = nullptr;
    Node* secondNode1 = nullptr;
    if (firstASTStmtRange.first != firstASTStmtRange.second) {
        firstNode1 = *firstASTStmtRange.first;
    }
    if (secondASTStmtRange.first != secondASTStmtRange.second) {
        secondNode1 = *secondASTStmtRange.first;
    }

    // Node 2 - ExprStmt node in the first AST, with no matching node in the second AST.
    Node* firstNode2 = nullptr;
    if (std::next(firstASTStmtRange.first) != firstASTStmtRange.second) {
        firstNode2 = *std::next(firstASTStmtRange.first);
    }

    // Node 3 - ReturnStmt node in the second AST, with no matching node in the first AST.
    Node* secondNode3 = nullptr;
    if (std::next(secondASTStmtRange.first) != secondASTStmtRange.second) {
        secondNode3 = *std::next(secondASTStmtRange.first);
    }

    // Set up expectations for the database wrapper.
    // First node should be found in both ASTs, thus no call to process as a single AST node.
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, _, _)).Times(Exactly(0));

    // Second node (firstNode2) is only in the first AST, should be processed as part of FIRST_AST.
    if (firstNode2) {
        EXPECT_CALL(dbWrapper, addNodeToBatch(*firstNode2, false, "ONLY_IN_FIRST_AST", "FIRST_AST")).Times(Exactly(1));
    }

    // Third node (secondNode3) is only in the second AST, should be processed as part of SECOND_AST.
    if (secondNode3) {
        EXPECT_CALL(dbWrapper, addNodeToBatch(*secondNode3, false, "ONLY_IN_SECOND_AST", "SECOND_AST")).Times(Exactly(1));
    }

    // Invoke the method to test
    comparer.compareStmtNodes(nodeKey);

    // Assertions: Verify that nodes are marked as processed properly.
    if (firstNode1) {
        ASSERT_TRUE(firstNode1->isProcessed);
    }
    if (secondNode1) {
        ASSERT_TRUE(secondNode1->isProcessed);
    }
    if (firstNode2) {
        ASSERT_TRUE(firstNode2->isProcessed);
    }
    if (secondNode3) {
        ASSERT_TRUE(secondNode3->isProcessed);
    }
}
