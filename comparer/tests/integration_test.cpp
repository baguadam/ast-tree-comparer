#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../headers/tree.h"
#include "../headers/tree_comparer.h"
#include "../headers/node_utilities.h"
#include "mock_database_wrapper.h"
#include "tree_comparer_test_wrapper.h"
#include "partial_tree_comparer.h"
#include <fstream>
#include <filesystem>
#include <iostream>

using ::testing::_;
using ::testing::Exactly;
using ::testing::AtLeast;
using ::testing::Matcher;
using ::testing::Field;

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

        std::ofstream testFile3("test_ast_3_shorter.txt");
        ASSERT_TRUE(testFile3.is_open());
        testFile3 << "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n";
        testFile3 << " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n";
        testFile3 << "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n";
        testFile3.close();

        ASSERT_TRUE(std::filesystem::exists("test_ast_1.txt"));
        ASSERT_TRUE(std::filesystem::exists("test_ast_2.txt"));
        ASSERT_TRUE(std::filesystem::exists("test_ast_3_shorter.txt"));
    }

    void TearDown() override {
        if (std::filesystem::exists("test_ast_1.txt")) {
            std::filesystem::remove("test_ast_1.txt");
        }
        if (std::filesystem::exists("test_ast_2.txt")) {
            std::filesystem::remove("test_ast_2.txt");
        }
        if (std::filesystem::exists("test_ast_3_shorter.txt")) {
            std::filesystem::remove("test_ast_3_shorter.txt");
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
// DATABASE OPERATIONS
// **********************************************
TEST_F(IntegrationTest, TestDatabaseOperation_CallsWithDifferingASTs) {
    checkDatabaseCalls("test_ast_1.txt", "test_ast_2.txt", 4, 2);
}

// Test for database operation calls with the same ASTs
TEST_F(IntegrationTest, TestDatabaseOperation_CallsWithSameASTs) {
    checkDatabaseCalls("test_ast_1.txt", "test_ast_1.txt", 0, 0);
}

// **********************************************
// processNodesInSingleAST tests
// **********************************************
TEST_F(IntegrationTest, ProcessNodesInSingleAST_NodeOnlyInFirstASTWithStmtChildren) {
    Tree firstAstTree("test_ast_1.txt");
    Tree secondAstTree("test_ast_3_shorter.txt");

    TreeComparerTestWrapper comparer(firstAstTree, secondAstTree, dbWrapper);

    // Key for the Function declaration node
    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    auto declNodeFirst = firstAstTree.getDeclNodes(nodeKey);

    ASSERT_NE(declNodeFirst.first, declNodeFirst.second);
    Node* functionNode = declNodeFirst.first->second;  // Get the declaration node (Function)

    ASSERT_FALSE(functionNode->isProcessed);  // Ensure the node is not processed

    // NODES
    EXPECT_CALL(dbWrapper, addNodeToBatch(Field(&Node::usr, functionNode->usr), _, "ONLY_IN_FIRST_AST", "FIRST_AST")).Times(1); // once for the function node
    EXPECT_CALL(dbWrapper, addNodeToBatch(Field(&Node::usr, "N/A"), _, "ONLY_IN_FIRST_AST", "FIRST_AST")).Times(2); // twice for the statement nodes

    // RELATISONSHIPS
    Node* firstChildNode = functionNode->children[0];
    Node* secondChildNode = firstChildNode->children[0];
    
    ASSERT_NE(firstChildNode, nullptr);
    ASSERT_NE(secondChildNode, nullptr);

    EXPECT_CALL(dbWrapper, addRelationshipToBatch(*functionNode, *firstChildNode)).Times(1);
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(*firstChildNode, *secondChildNode)).Times(1);

    comparer.processNodesInSingleAST(functionNode, firstAstTree, FIRST_AST, true);

    // Verify that nodes are marked as processed
    ASSERT_TRUE(functionNode->isProcessed);
    ASSERT_TRUE(firstChildNode->isProcessed);
    ASSERT_TRUE(secondChildNode->isProcessed);
}

TEST_F(IntegrationTest, ProcessNodesInSingleAST_NodeOnlyInSecondASTSingle) {
    std::ofstream testFile2("test_ast_2_more_complex.txt");
    ASSERT_TRUE(testFile2.is_open());
    testFile2 << "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n";
    testFile2 << " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n";
    testFile2 << "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n";
    testFile2 << "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++configother.h\t310\t3\n";
    testFile2 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n";
    testFile2 << "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6\n";
    testFile2 << "    Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7\n";
    testFile2.close();

    Tree firstAstTree("test_ast_1.txt");
    Tree secondAstTree("test_ast_2_more_complex.txt");

    TreeComparerTestWrapper comparer(firstAstTree, secondAstTree, dbWrapper);
    std::string nodeKey = "Typedef|c:@N@std@T@size_t|C:\\include\\bits\\c++configother.h|";
    auto declNodeFirst = secondAstTree.getDeclNodes(nodeKey);
    ASSERT_NE(declNodeFirst.first, declNodeFirst.second);

    Node* typedefNode = declNodeFirst.first->second;
    EXPECT_CALL(dbWrapper, addNodeToBatch(Field(&Node::usr, typedefNode->usr), _, "ONLY_IN_SECOND_AST", "SECOND_AST")).Times(1); // once for the function node
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(0));

    comparer.processNodesInSingleAST(typedefNode, secondAstTree, SECOND_AST, true);

    ASSERT_TRUE(typedefNode->isProcessed);
}

TEST_F(IntegrationTest, ProcessNodesInSingleAST_NodeIsProcessed) {
    Tree firstAstTree("test_ast_1.txt");
    Tree secondAstTree("test_ast_3_shorter.txt");

    TreeComparerTestWrapper comparer(firstAstTree, secondAstTree, dbWrapper);

    // Key for the Function declaration node
    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    auto declNodeFirst = firstAstTree.getDeclNodes(nodeKey);

    ASSERT_NE(declNodeFirst.first, declNodeFirst.second);
    Node* functionNode = declNodeFirst.first->second;

    ASSERT_FALSE(functionNode->isProcessed);

    // mark the node as processed, nothing should happen
    functionNode->isProcessed = true;

    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, _, _)).Times(Exactly(0));
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(0));

    comparer.processNodesInSingleAST(functionNode, firstAstTree, FIRST_AST, true);
}

TEST_F(IntegrationTest, ProcessNodesInSingleAST_ChildNodeIsProcessed) {
    std::ofstream testFile2("test_ast_2_more_complex.txt");
    ASSERT_TRUE(testFile2.is_open());
    testFile2 << "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n";
    testFile2 << " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n";
    testFile2 << "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n";
    testFile2 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n";
    testFile2 << "   Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n";
    testFile2 << "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6\n";
    testFile2 << "    Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7\n";
    testFile2.close();

    Tree firstAstTree("test_ast_1.txt");
    Tree secondAstTree("test_ast_2_more_complex.txt");

    TreeComparerTestWrapper comparer(firstAstTree, secondAstTree, dbWrapper);

    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    auto declNodeFirst = secondAstTree.getDeclNodes(nodeKey);

    ASSERT_NE(declNodeFirst.first, declNodeFirst.second);
    Node* functionNode = declNodeFirst.first->second; 
    Node* firstChildNode = functionNode->children[1];               // CompoundStmt
    Node* secondChildNode = functionNode->children[0];              // Typedef
    Node* firstChildNodeChildNode = firstChildNode->children[0];    // ExprStmt

    std::cout << "Function node: " << functionNode->kind << std::endl;
    std::cout << "First child node: " << firstChildNode->kind << std::endl;
    std::cout << "Second child node: " << secondChildNode->kind << std::endl;
    std::cout << "First child node child node: " << firstChildNodeChildNode->kind << std::endl;

    ASSERT_NE(firstChildNode, nullptr);
    ASSERT_NE(secondChildNode, nullptr);
    ASSERT_NE(firstChildNodeChildNode, nullptr);

    secondChildNode->isProcessed = true; // set Typedef as processed, also exists in first ast, should be skipped

    // NODES and RELATIONSHIPS
    // EXPECT:
    // - Function node + two statement nodes with their relationships
    EXPECT_CALL(dbWrapper, addNodeToBatch(Field(&Node::usr, functionNode->usr), _, "ONLY_IN_SECOND_AST", "SECOND_AST")).Times(1); // once for the function node
    EXPECT_CALL(dbWrapper, addNodeToBatch(Field(&Node::usr, "N/A"), _, "ONLY_IN_SECOND_AST", "SECOND_AST")).Times(2); // twice for the statement nodes
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(*functionNode, *firstChildNode)).Times(1);
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(*firstChildNode, *firstChildNodeChildNode)).Times(1);

    comparer.processNodesInSingleAST(functionNode, secondAstTree, SECOND_AST, true);

    // Verify that nodes are marked as processed
    ASSERT_TRUE(functionNode->isProcessed);
    ASSERT_TRUE(firstChildNode->isProcessed);
    ASSERT_TRUE(secondChildNode->isProcessed);
    ASSERT_TRUE(firstChildNodeChildNode->isProcessed);
}

// **********************************************
// processMultiDeclNodes tests
// **********************************************
TEST_F(IntegrationTest, ProcessMultiDeclNodes_MultipleNodesInBothASTsNoStmts) {
    std::ofstream testFile1("test_ast_1_multi_decl.txt");
    ASSERT_TRUE(testFile1.is_open());
    testFile1 << "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n";
    testFile1 << " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n";
    testFile1 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n";
    testFile1 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t355\t6\n"; // two duplicate keys
    testFile1.close();

    std::ofstream testFile2("test_ast_2_multi_decl.txt");
    ASSERT_TRUE(testFile2.is_open());
    testFile2 << "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n";
    testFile2 << " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n";
    testFile2 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n";
    testFile2 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t360\t7\n"; // 
    testFile2 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t365\t8\n"; // three duplicate keys
    testFile2.close();

    Tree firstAstTree("test_ast_1_multi_decl.txt");
    Tree secondAstTree("test_ast_2_multi_decl.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);
}