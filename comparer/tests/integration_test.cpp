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

    // method to create a node
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

    // helper method to check database calls:
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

    // helper method for processDeclNodes
    void checkDeclNodeCalls(
        const std::string& firstAstFile,
        const std::string& secondAstFile,
        const std::string& nodeKey,
        int processBothASTsCalls,
        int processSingleASTCalls,
        bool existsInBothASTs) {

        Tree firstAstTree(firstAstFile);
        Tree secondAstTree(secondAstFile);

        PartialMockTreeComparerForDeclNodes mockComparer(firstAstTree, secondAstTree, dbWrapper);

        auto firstASTRange = firstAstTree.getDeclNodes(nodeKey);
        Node* functionNode = nullptr;

        if (firstASTRange.first != firstASTRange.second) {
            functionNode = firstASTRange.first->second;
        } else {
            auto secondASTRange = secondAstTree.getDeclNodes(nodeKey);
            ASSERT_NE(secondASTRange.first, secondASTRange.second);
            functionNode = secondASTRange.first->second;
        }

        // expected calls
        EXPECT_CALL(mockComparer, processDeclNodesInBothASTs(nodeKey)).Times(processBothASTsCalls);
        if (existsInBothASTs) {
            EXPECT_CALL(mockComparer, processNodesInSingleAST(_, _, _, _)).Times(0);
        } else {
            EXPECT_CALL(mockComparer, processNodesInSingleAST(functionNode, _, _, true)).Times(processSingleASTCalls);
        }

        // invoke the method
        mockComparer.processDeclNodes(functionNode);
    }

    // utility function to create an AST file.
    void createASTFile(const std::string& filename, const std::vector<std::string>& lines) {
        std::ofstream testFile(filename);
        ASSERT_TRUE(testFile.is_open());
        for (const auto& line : lines) {
            testFile << line << "\n";
        }
        testFile.close();
    }

    // utility function to retrieve nodes with a given key.
    std::pair<Node*, Node*> getMatchingNodes(Tree& firstAstTree, Tree& secondAstTree, const std::string& nodeKey) {
        auto firstASTRange = firstAstTree.getDeclNodes(nodeKey);
        auto secondASTRange = secondAstTree.getDeclNodes(nodeKey);

        EXPECT_NE(firstASTRange.first, firstASTRange.second);
        EXPECT_NE(secondASTRange.first, secondASTRange.second);

        Node* firstNode = firstASTRange.first->second;
        Node* secondNode = secondASTRange.first->second;

        return {firstNode, secondNode};
    }

    // accessible for all tests
    MockDatabaseWrapper dbWrapper;
};

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
    createASTFile("test_ast_2_more_complex.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3",
        "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++configother.h\t310\t3",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6",
        "    Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7"
    });

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

TEST_F(IntegrationTest, ProcessNodesInSingleAST_ChildNodeIsProcessedAndExistsInOtherAST) {
    createASTFile("test_ast_2_more_complex.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n",
        "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n",
        "   Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n",
        "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6\n",
        "    Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7\n"
    });

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

    // verify that nodes are marked as processed
    ASSERT_TRUE(functionNode->isProcessed);
    ASSERT_TRUE(firstChildNode->isProcessed);
    ASSERT_TRUE(secondChildNode->isProcessed);
    ASSERT_TRUE(firstChildNodeChildNode->isProcessed);
}

// **********************************************
// compareStmtNodes tests
// **********************************************
TEST_F(IntegrationTest, CompareStmtNodes_MatchingNodesInBothASTs) {
    createASTFile("test_ast_1_stmt_match.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6",  // Statement to be matched
        "  Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7"    // Statement to be matched
    });

    createASTFile("test_ast_2_stmt_match.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6",  // Statement to be matched
        "  Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7"    // Statement to be matched
    });

    Tree firstAstTree("test_ast_1_stmt_match.txt");
    Tree secondAstTree("test_ast_2_stmt_match.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);
    auto [firstNode, secondNode] = getMatchingNodes(firstAstTree, secondAstTree, "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|");

    EXPECT_CALL(mockComparer, compareParents(_, _)).Times(2); // comparing the two similar nodes
    EXPECT_CALL(mockComparer, processNodesInSingleAST(_, _, _, _)).Times(0); // no calls are expected here

    // invoke method
    mockComparer.compareStmtNodes(firstNode, secondNode);
}

TEST_F(IntegrationTest, CompareStmtNodes_NodesOnlyInOneAST) {
    createASTFile("test_ast_1_stmt_partial.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6"  // Only in first AST
    });

    createASTFile("test_ast_2_stmt_partial.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7"  // Only in second AST
    });

    Tree firstAstTree("test_ast_1_stmt_partial.txt");
    Tree secondAstTree("test_ast_2_stmt_partial.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);

    auto [firstNode, secondNode] = getMatchingNodes(firstAstTree, secondAstTree, "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|");

    EXPECT_CALL(mockComparer, compareParents(_, _)).Times(0); // no similar nodes
    EXPECT_CALL(mockComparer, processNodesInSingleAST(_, _, FIRST_AST, false)).Times(1); // only in first AST
    EXPECT_CALL(mockComparer, processNodesInSingleAST(_, _, SECOND_AST, false)).Times(1); // only in second AST

    // invoke method
    mockComparer.compareStmtNodes(firstNode, secondNode);
}

TEST_F(IntegrationTest, CompareStmtNodes_PartiallyMatchingNodes) {
    createASTFile("test_ast_1_stmt_mixed.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6",  // Matches
        "  Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t355\t7"       // Only in first AST
    });

    createASTFile("test_ast_2_stmt_mixed.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6",  // Matches
        "  Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t357\t8"     // Only in second AST
    });

    Tree firstAstTree("test_ast_1_stmt_mixed.txt");
    Tree secondAstTree("test_ast_2_stmt_mixed.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);

    auto [firstNode, secondNode] = getMatchingNodes(firstAstTree, secondAstTree, "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|");

    EXPECT_CALL(mockComparer, compareParents(_, _)).Times(1); // once for the matching nodes
    EXPECT_CALL(mockComparer, processNodesInSingleAST(_, _, FIRST_AST, false)).Times(1); // only in first AST
    EXPECT_CALL(mockComparer, processNodesInSingleAST(_, _, SECOND_AST, false)).Times(1); // only in second AST

    // invoke method
    mockComparer.compareStmtNodes(firstNode, secondNode);
}

// **********************************************
// compareSimilarDeclNodes tests
// **********************************************
TEST_F(IntegrationTest, CompareSimilarDeclNodes_SameNodesNoDifference) {
    Tree dummyTree1("test_ast_1.txt");
    Tree dummyTree2("test_ast_2.txt");

    PartialMockTreeComparerForDeclNodes comparer(dummyTree1, dummyTree2, dbWrapper);

    Node firstNode = createNode(DECLARATION, "FunctionDecl", "c:@N@func1", "C:\\project\\file1.cpp", 100, 10);
    Node secondNode = createNode(DECLARATION, "FunctionDecl", "c:@N@func1", "C:\\project\\file1.cpp", 100, 10);

    EXPECT_CALL(comparer, compareParents(_, _)).Times(Exactly(1));
    EXPECT_CALL(comparer, compareSourceLocations(_, _)).Times(Exactly(1));
    EXPECT_CALL(comparer, compareStmtNodes(_, _)).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, _, _)).Times(Exactly(0));
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(0));

    comparer.compareSimilarDeclNodes(&firstNode, &secondNode);

    EXPECT_TRUE(firstNode.isProcessed);
    EXPECT_TRUE(secondNode.isProcessed);
}

// **********************************************
// processMultiDeclNodes tests
// **********************************************
TEST_F(IntegrationTest, ProcessMultiDeclNodes_AllNodesAlreadyProcessed) {
    createASTFile("test_ast_1_processed.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t355\t6"
    });

    createASTFile("test_ast_2_processed.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t355\t6"
    });

    Tree firstAstTree("test_ast_1_processed.txt");
    Tree secondAstTree("test_ast_2_processed.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);

    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    auto firstASTRange = firstAstTree.getDeclNodes(nodeKey);
    auto secondASTRange = secondAstTree.getDeclNodes(nodeKey);

    for (auto it = firstASTRange.first; it != firstASTRange.second; ++it) {
        it->second->isProcessed = true;
    }
    for (auto it = secondASTRange.first; it != secondASTRange.second; ++it) {
        it->second->isProcessed = true;
    }

    // no nodes should be added to the database
    EXPECT_CALL(mockComparer, compareSimilarDeclNodes(_, _)).Times(0);
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, _, _)).Times(0);
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(0);

    mockComparer.processMultiDeclNodes(firstASTRange, secondASTRange);
}

TEST_F(IntegrationTest, ProcessMultiDeclNodes_MultipleNodesInBothASTsNoStmts) {
    createASTFile("test_ast_1_multi_decl.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t355\t6"
    });

    createASTFile("test_ast_2_multi_decl.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t355\t6\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t365\t8\n" // three duplicate keys
    });

    Tree firstAstTree("test_ast_1_multi_decl.txt");
    Tree secondAstTree("test_ast_2_multi_decl.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);

    // range of the nodes
    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    auto firstASTRange = firstAstTree.getDeclNodes(nodeKey);
    auto secondASTRange = secondAstTree.getDeclNodes(nodeKey);

    // more than one node in both
    ASSERT_EQ(std::distance(firstASTRange.first, firstASTRange.second), 2);
    ASSERT_EQ(std::distance(secondASTRange.first, secondASTRange.second), 3);

    // compare similar nodes should be compared twice
    EXPECT_CALL(mockComparer, compareSimilarDeclNodes(_, _)).Times(2);

    // remaining node
    EXPECT_CALL(mockComparer, processNodesInSingleAST(_, _, _, _)).Times(1); 

    // invoke method call
    mockComparer.processMultiDeclNodes(firstASTRange, secondASTRange);
}

TEST_F(IntegrationTest, ProcessMultiDeclNodes_MultipleNodesWithChildrenInBothASTs) {
    createASTFile("test_ast_1_with_children.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "   Declaration\tVariable\tc:@V@var1\tC:\\include\\bits\\c++config.h\t351\t6\n"  // child of the first Function node
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t355\t7\n"
    });

    createASTFile("test_ast_2_with_children.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "   Declaration\tVariable\tc:@V@var1\tC:\\include\\bits\\c++config.h\t352\t6\n"  // child of the first Function node
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t360\t7\n" // difference in source location
    });

    Tree firstAstTree("test_ast_1_with_children.txt");
    Tree secondAstTree("test_ast_2_with_children.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);

    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    auto firstASTRange = firstAstTree.getDeclNodes(nodeKey);
    auto secondASTRange = secondAstTree.getDeclNodes(nodeKey);

    ASSERT_EQ(std::distance(firstASTRange.first, firstASTRange.second), 2);
    ASSERT_EQ(std::distance(secondASTRange.first, secondASTRange.second), 2);

    EXPECT_CALL(mockComparer, compareSimilarDeclNodes(_, _)).Times(2); // for both nodes 

    // invoke method call
    mockComparer.processMultiDeclNodes(firstASTRange, secondASTRange);
}

TEST_F(IntegrationTest, ProcessMultiDeclNodes_SingleNodeInFirstAST_ThreeNodesInSecondAST_WithChildren) {
    createASTFile("test_ast_1_single_node.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "   Declaration\tVariable\tc:@V@varInFirst\tC:\\include\\bits\\c++config.h\t351\t6\n"
    });

    createASTFile("test_ast_2_three_nodes.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "   Declaration\tVariable\tc:@V@varInSecond1\tC:\\include\\bits\\c++config.h\t352\t7\n"  
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t355\t8\n"
        "   Declaration\tVariable\tc:@V@varInSecond2\tC:\\include\\bits\\c++config.h\t356\t9\n" 
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t360\t10\n"
        "   Declaration\tVariable\tc:@V@varInSecond3\tC:\\include\\bits\\c++config.h\t361\t11\n"
    });

    Tree firstAstTree("test_ast_1_single_node.txt");
    Tree secondAstTree("test_ast_2_three_nodes.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);

    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    auto firstASTRange = firstAstTree.getDeclNodes(nodeKey);
    auto secondASTRange = secondAstTree.getDeclNodes(nodeKey);

    ASSERT_EQ(std::distance(firstASTRange.first, firstASTRange.second), 1);
    ASSERT_EQ(std::distance(secondASTRange.first, secondASTRange.second), 3);

    EXPECT_CALL(mockComparer, compareSimilarDeclNodes(_, _)).Times(1); // for the first appearance of the node
    EXPECT_CALL(mockComparer, processNodesInSingleAST(_, _, _, _)).Times(2); // for the children of the second and third nodes

    // when compareSimilarDeclodes is called, children are not set as processed
    auto firstFunctionNodeInSecond = secondASTRange.first->second;
    auto firstFunctionNodeInFirst = firstASTRange.first->second;
    Node* varInFirst = firstFunctionNodeInFirst->children[0];
    Node* varInSecond = firstFunctionNodeInSecond->children[0];

    ASSERT_FALSE(varInFirst->isProcessed);
    ASSERT_FALSE(varInSecond->isProcessed);

    // invoke method call
    mockComparer.processMultiDeclNodes(firstASTRange, secondASTRange);
}

// **********************************************
// processDeclNodesInBothASTs tests
// **********************************************
TEST_F(IntegrationTest, ProcessDeclNodesInBothASTs_SingleNodeInBothASTs) {
    createASTFile("test_ast_1_single_node_both.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "  Declaration\tVariable\tc:@V@someVar\tC:\\include\\bits\\c++config.h\t400\t8\n"
    });

    createASTFile("test_ast_2_single_node_both.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "  Declaration\tVariable\tc:@V@someVar\tC:\\include\\bits\\c++config.h\t400\t8\n"
    });

    Tree firstAstTree("test_ast_1_single_node_both.txt");
    Tree secondAstTree("test_ast_2_single_node_both.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);

    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";

    EXPECT_CALL(mockComparer, compareSimilarDeclNodes(_, _)).Times(1);

    // invoke method to be tested
    mockComparer.processDeclNodesInBothASTs(nodeKey);
}

TEST_F(IntegrationTest, ProcessDeclNodesInBothASTs_OneNodeInFirstAST_MultipleNodesInSecondAST) {
    createASTFile("test_ast_1_one_node.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "  Declaration\tVariable\tc:@V@someVar\tC:\\include\\bits\\c++config.h\t400\t8\n"
    });

    createASTFile("test_ast_2_multiple_nodes.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t355\t6\n"
        "  Declaration\tVariable\tc:@V@anotherVar\tC:\\include\\bits\\c++config.h\t405\t9\n"
    });

    Tree firstAstTree("test_ast_1_one_node.txt");
    Tree secondAstTree("test_ast_2_multiple_nodes.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);

    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    auto firstASTRange = firstAstTree.getDeclNodes(nodeKey);
    auto secondASTRange = secondAstTree.getDeclNodes(nodeKey);

    ASSERT_EQ(std::distance(firstASTRange.first, firstASTRange.second), 1);
    ASSERT_EQ(std::distance(secondASTRange.first, secondASTRange.second), 2);

    EXPECT_CALL(mockComparer, compareSimilarDeclNodes(_, _)).Times(1); // on call from the processMultiDeclNodes
    EXPECT_CALL(mockComparer, processNodesInSingleAST(_, _, _, _)).Times(1); // for the remaining node

    // invoke method to be tested
    mockComparer.processDeclNodesInBothASTs(nodeKey);
}

TEST_F(IntegrationTest, ProcessDeclNodesInBothASTs_MultipleNodesInBothASTs) {
    createASTFile("test_ast_1_multiple_nodes_both.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t355\t6\n"
        "  Declaration\tVariable\tc:@V@someVar\tC:\\include\\bits\\c++config.h\t410\t10\n"
    });

    createASTFile("test_ast_2_multiple_nodes_both.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n"
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n"
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t360\t7\n"
        "  Declaration\tVariable\tc:@V@anotherVar\tC:\\include\\bits\\c++config.h\t415\t11\n"
    });

    Tree firstAstTree("test_ast_1_multiple_nodes_both.txt");
    Tree secondAstTree("test_ast_2_multiple_nodes_both.txt");

    PartialMockTreeComparer mockComparer(firstAstTree, secondAstTree, dbWrapper);

    std::string nodeKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";

    EXPECT_CALL(mockComparer, compareSimilarDeclNodes(_, _)).Times(2); // two calls from the processMultiDeclNodes

    // invoke method to be tested
    mockComparer.processDeclNodesInBothASTs(nodeKey);
}

// **********************************************
// processDeclNodes tests
// **********************************************
TEST_F(IntegrationTest, ProcessDeclNodes_NodeExistsInBothASTs) {
    createASTFile("test_ast_1_both.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5"
    });

    createASTFile("test_ast_2_both.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5"
    });

    checkDeclNodeCalls(
        "test_ast_1_both.txt",
        "test_ast_2_both.txt",
        "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|",
        1,   // times processDeclNodesInBothASTs is expected
        0,   // times processNodesInSingleAST is expected
        true // exists in both ASTs
    );
}

TEST_F(IntegrationTest, ProcessDeclNodes_NodeExistsOnlyInFirstAST) {
    createASTFile("test_ast_1_first.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5"
    });

    createASTFile("test_ast_2_empty.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0"
    });

    checkDeclNodeCalls(
        "test_ast_1_first.txt",
        "test_ast_2_empty.txt",
        "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|",
        0,    // times processDeclNodesInBothASTs is expected
        1,    // times processNodesInSingleAST is expected
        false // exists only in first AST
    );
}

TEST_F(IntegrationTest, ProcessDeclNodes_NodeExistsOnlyInSecondASTMultipleTimes) {
    createASTFile("test_ast_1_empty.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0"
    });

    createASTFile("test_ast_2_second.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t351\t6"  // Appears again
    });

    checkDeclNodeCalls(
        "test_ast_1_empty.txt",
        "test_ast_2_second.txt",
        "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|",
        0,    // times processDeclNodesInBothASTs is expected
        1,    // times processNodesInSingleAST is expected
        false // exists only in second AST
    );
}

// **************************************************************************
// **************************************************************************
// printDifferences tests - complex scenarios, using the entire comparer
// **************************************************************************
// **************************************************************************
TEST_F(IntegrationTest, PrintDifferences_NoDifferences) {
    createASTFile("test_ast_1_same.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6",
        "    Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7"
    });

    createASTFile("test_ast_2_same.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",
        "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6",
        "    Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7"
    });

    checkDatabaseCalls("test_ast_1_same.txt", "test_ast_2_same.txt", 0, 0);
}

TEST_F(IntegrationTest, PrintDifferences_DiffSource_ChildOnlyInFirst_TwoSubtreesOnlyInSecond) {
        createASTFile("test_ast_1_mixed.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5", // Declaration with different source location
        "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6",           // Only in first AST
        "  Declaration\tVariable\tc:@N@globalVar\tC:\\include\\bits\\c++config.h\t310\t3",   // Variable with child only in the first AST
        "   Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t312\t3" // Child only in first AST
    });

    createASTFile("test_ast_2_mixed.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t450\t5", // Declaration with different source location
        "  Declaration\tFunction\tc:@F@doAnotherThing\tC:\\include\\bits\\c++config.h\t460\t8",   // Declaration with three statement nodes only in second AST
        "   Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t461\t9",
        "   Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t462\t10",
        "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t463\t11",
        "  Declaration\tClass\tc:@C@SomeClass\tC:\\include\\bits\\c++config.h\t480\t12",          // Declaration with identical statement nodes only in second AST
        "   Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t481\t13",
        "   Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t482\t14",
        "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t483\t15",
        "  Declaration\tVariable\tc:@N@globalVarAnother\tC:\\include\\bits\\c++config.h\t310\t3"  // Variable without the child node present in first AST
    });

    Tree firstAstTree("test_ast_1_mixed.txt");
    Tree secondAstTree("test_ast_2_mixed.txt");

    // usual database calls 
    EXPECT_CALL(dbWrapper, clearDatabase()).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, finalize()).Times(Exactly(1));

    // source location
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_SOURCE_LOCATIONS", "FIRST_AST")).Times(Exactly(1));  // source location from the first AST
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_SOURCE_LOCATIONS", "SECOND_AST")).Times(Exactly(1)); // source location from the second AST

    // nodes only in second AST
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_SECOND_AST", _)).Times(9); // 1 declaration + 3 statements, 1 class + 3 statements, and 1 variable

    // nodes only in first AST
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_FIRST_AST", _)).Times(3); // 1 CompountStmt, 1 Typedef, 1 Variable 

    // relationships
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(7)); // 3 relationships for the first declaration, 3 for the second, and 1 for the variable

    TreeComparer comparer(firstAstTree, secondAstTree, dbWrapper);
    comparer.printDifferences();
}

TEST_F(IntegrationTest, PrintDifferences_MultipleAppearances_DifferentParents_DifferentSourceLocations) {
    createASTFile("test_ast_1_complex.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5",         // Declaration with the same key but different source locations
        "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6",                   // Statement only in first AST
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t360\t7",         // Second appearance of the function
        "   Declaration\tVariable\tc:@N@var1\tC:\\include\\bits\\c++config.h\t362\t8",               // Variable child only in first AST
        " Declaration\tClass\tc:@C@CommonClass\tC:\\include\\bits\\c++config.h\t370\t10",            // Class node with identical statements
        "  Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t371\t11",
        "  Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t372\t12",
        "  Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t373\t13",
        " Declaration\tVariable\tc:@N@globalVar\tC:\\include\\bits\\c++config.h\t310\t3"             // Variable with child only in first AST
    });

    createASTFile("test_ast_2_complex.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1",
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++configother.h\t450\t5",      // Declaration with the same key but different source location
        "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++configother.h\t460\t7",      // Second appearance of the function, different location
        "   Declaration\tVariable\tc:@N@var2\tC:\\include\\bits\\c++config.h\t463\t8",                 // Variable child only in second AST
        "  Declaration\tClass\tc:@C@CommonClass\tC:\\include\\bits\\c++config.h\t480\t10",             // Class node with identical statements
        "   Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t481\t13",
        "   Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t482\t14",
        "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t483\t15",
        " Declaration\tVariable\tc:@N@globalVar\tC:\\include\\bits\\c++config.h\t310\t3"               // Variable without the child node present in first AST
    });

    Tree firstAstTree("test_ast_1_complex.txt");
    Tree secondAstTree("test_ast_2_complex.txt");

    // usual database calls
    EXPECT_CALL(dbWrapper, clearDatabase()).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, finalize()).Times(Exactly(1));

    // source location differences
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_SOURCE_LOCATIONS", "FIRST_AST")).Times(Exactly(4));  // Class + 3 statements
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_SOURCE_LOCATIONS", "SECOND_AST")).Times(Exactly(4)); // Class + 3 statements

    // parent differences
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_PARENTS", "FIRST_AST")).Times(Exactly(1));  // Class
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_PARENTS", "SECOND_AST")).Times(Exactly(1)); // Class

    // nodes only in second AST
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_SECOND_AST", _)).Times(3); // 1 variable, 2 function nodes

    // nodes only in first AST
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_FIRST_AST", _)).Times(4); // 1 CompoundStmt, 1 variable, 2 function nodes

    // relationships
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(3)); // 

    TreeComparer comparer(firstAstTree, secondAstTree, dbWrapper);
    comparer.printDifferences();
}

TEST_F(IntegrationTest, PrintDifferences_NamespaceWithExtraStatementsAndFunctions) {
    createASTFile("test_ast_1_namespace.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@myNamespace\tC:\\project\\namespace.cpp\t100\t1",                           // Namespace node exists in both
        "  Declaration\tFunction\tc:@N@myNamespace@F@doSomething\tC:\\project\\namespace.cpp\t110\t2",             // Function exists in both
        "   Statement\tExprStmt\tN/A\tC:\\project\\namespace.cpp\t111\t3",                                         // Shared statement
        "   Statement\tReturnStmt\tN/A\tC:\\project\\namespace.cpp\t112\t4",                                       // Statement only in the first AST
        "  Declaration\tFunction\tc:@N@myNamespace@F@onlyInFirst\tC:\\project\\namespace.cpp\t120\t5",             // Function only in the first AST
        "   Statement\tExprStmt\tN/A\tC:\\project\\namespace.cpp\t121\t6",                                         
        "  Declaration\tVariable\tc:@N@myNamespace@globalVar1\tC:\\project\\namespace.cpp\t130\t7"                 // Variable only in the first AST
    });

    createASTFile("test_ast_2_namespace.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@myNamespace\tC:\\project\\namespace.cpp\t100\t1",                           // Namespace node exists in both
        "  Statement\tExprStmt\tN/A\tC:\\project\\namespace.cpp\t105\t2",                                          // Statement only in the second AST
        "  Declaration\tFunction\tc:@N@myNamespace@F@doSomething\tC:\\project\\namespace.cpp\t110\t2",             // Function exists in both
        "   Statement\tExprStmt\tN/A\tC:\\project\\namespace.cpp\t111\t3",                                         // Shared statement
        "   Statement\tExprStmt\tN/A\tC:\\project\\namespace.cpp\t115\t5",                                         // Statement only in the second AST
        "  Declaration\tFunction\tc:@N@myNamespace@F@onlyInSecond\tC:\\project\\namespace.cpp\t140\t6",            // Function only in the second AST
        "   Statement\tReturnStmt\tN/A\tC:\\project\\namespace.cpp\t141\t7",                                       
        "  Declaration\tVariable\tc:@N@myNamespace@globalVar2\tC:\\project\\namespace.cpp\t150\t8"                 // Variable only in the second AST
    });

    Tree firstAstTree("test_ast_1_namespace.txt");
    Tree secondAstTree("test_ast_2_namespace.txt");

    // usual database calls
    EXPECT_CALL(dbWrapper, clearDatabase()).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, finalize()).Times(Exactly(1));

    // namespace differences
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_FIRST_AST", _)).Times(4);   // 1 Function + ExprStmt, 1 Variabla, 1 ReturnStmt
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_SECOND_AST", _)).Times(5);  // 1 Function + 1 ReturnStmt, 1 Variable, 2ExprStmts

    // no calls:
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_SOURCE_LOCATIONS", _)).Times(0);
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_PARENTS", _)).Times(0);

    // relationships
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(2)); // Function -> ExprStmt, Function -> ReturnStmt

    TreeComparer comparer(firstAstTree, secondAstTree, dbWrapper);
    comparer.printDifferences();
}

TEST_F(IntegrationTest, PrintDifferences_SameKeysDifferentStructures) {
    createASTFile("test_ast_1_same_key.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@commonFunc\tC:\\project\\different_structure.cpp\t100\t1",
        "  Statement\tCompoundStmt\tN/A\tC:\\project\\different_structure.cpp\t101\t2",
        "  Declaration\tVariable\tc:@N@varUniqueFirst\tC:\\project\\different_structure.cpp\t102\t3"       // Variable only in first AST
    });

    createASTFile("test_ast_2_same_key.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@commonFunc\tC:\\project\\different_structure.cpp\t100\t1",
        "  Statement\tCompoundStmt\tN/A\tC:\\project\\different_structure.cpp\t101\t2",
        "  Declaration\tTypedef\tc:@N@typedefUniqueSecond\tC:\\project\\different_structure.cpp\t105\t4"   // Typedef only in second AST
    });

    Tree firstAstTree("test_ast_1_same_key.txt");
    Tree secondAstTree("test_ast_2_same_key.txt");

    // usual database calls
    EXPECT_CALL(dbWrapper, clearDatabase()).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, finalize()).Times(Exactly(1));

    // differences in children structure
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_FIRST_AST", _)).Times(1);  // Variable node only in first AST
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_SECOND_AST", _)).Times(1); // Typedef node only in second AST

    // relationships for different children
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(0)); // no relationships, no children

    TreeComparer comparer(firstAstTree, secondAstTree, dbWrapper);
    comparer.printDifferences();
}

TEST_F(IntegrationTest, PrintDifferences_CrossNamespaceReferencesAndRecursiveDeclarations) {
    createASTFile("test_ast_1_cross_namespace.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@namespaceA\tC:\\project\\cross_namespace.cpp\t100\t1",
        "  Declaration\tFunction\tc:@N@namespaceA@F@functionA\tC:\\project\\cross_namespace.cpp\t110\t2",  // Function exists in both, references namespaceB
        "   Statement\tExprStmt\tN/A\tC:\\project\\cross_namespace.cpp\t111\t3",
        " Declaration\tNamespace\tc:@N@namespaceB\tC:\\project\\cross_namespace.cpp\t120\t4",              // NamespaceB exists only in the first AST
        "  Declaration\tFunction\tc:@N@namespaceB@F@functionB\tC:\\project\\cross_namespace.cpp\t130\t5",  // Function only in the first AST
        "   Statement\tReturnStmt\tN/A\tC:\\project\\cross_namespace.cpp\t131\t6"
    });

    createASTFile("test_ast_2_cross_namespace.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tNamespace\tc:@N@namespaceA\tC:\\project\\cross_namespace.cpp\t100\t1",
        "  Declaration\tFunction\tc:@N@namespaceA@F@functionA\tC:\\project\\cross_namespace.cpp\t110\t2",            // Function exists in both, references namespaceB differently
        "   Statement\tExprStmt\tN/A\tC:\\project\\cross_namespace.cpp\t111\t3",
        "  Declaration\tFunction\tc:@N@namespaceA@F@functionOnlyInSecond\tC:\\project\\cross_namespace.cpp\t150\t7", // Function only in the second AST
        "   Statement\tExprStmt\tN/A\tC:\\project\\cross_namespace.cpp\t151\t8",
        " Declaration\tNamespace\tc:@N@namespaceC\tC:\\project\\cross_namespace.cpp\t160\t9",                        // NamespaceC exists only in the second AST
        "  Declaration\tFunction\tc:@N@namespaceC@F@functionC\tC:\\project\\cross_namespace.cpp\t170\t10",           // Function only in second AST, references namespaceA
        "   Statement\tExprStmt\tN/A\tC:\\project\\cross_namespace.cpp\t171\t11"
    });

    Tree firstAstTree("test_ast_1_cross_namespace.txt");
    Tree secondAstTree("test_ast_2_cross_namespace.txt");

    // usual database calls
    EXPECT_CALL(dbWrapper, clearDatabase()).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, finalize()).Times(Exactly(1));

    // namespace differences
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_FIRST_AST", _)).Times(3);  // NamespaceB, FunctionB, ReturnStmt
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "ONLY_IN_SECOND_AST", _)).Times(5); // NamespaceC, FunctionC, ExprStmt, FunctionOnlyInSecond, ExprStmt

    // function call differences
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(5)); // NamespaceB -> FunctionB -> ReturnStmt, NamespaceC -> FunctionC -> ExprStmt, FunctionOnlyInSecond -> ExprStmt

    TreeComparer comparer(firstAstTree, secondAstTree, dbWrapper);
    comparer.printDifferences();
}

TEST_F(IntegrationTest, PrintDifferences_CyclicRelationships) {
    createASTFile("test_ast_1_cyclic.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@funcA\tC:\\project\\cyclic.cpp\t100\t1",                  // funcA calls funcB
        "  Statement\tCompoundStmt\tN/A\tC:\\project\\cyclic.cpp\t101\t2",
        "   Statement\tCallExpr\tfuncB\tC:\\project\\cyclic.cpp\t102\t3",                       // Calls funcB
        " Declaration\tFunction\tc:@F@funcB\tC:\\project\\cyclic.cpp\t200\t4",                  // funcB calls funcA (cycle)
        "  Statement\tCompoundStmt\tN/A\tC:\\project\\cyclic.cpp\t201\t5",
        "   Statement\tCallExpr\tfuncA\tC:\\project\\cyclic.cpp\t202\t6"                        // Calls funcA
    });

    createASTFile("test_ast_2_cyclic.txt", {
        "Declaration\tTranslationUnit\tc:\tN/A\t0\t0",
        " Declaration\tFunction\tc:@F@funcA\tC:\\project\\cyclic.cpp\t100\t1",                  // funcA calls funcB with slight change
        "  Statement\tCompoundStmt\tN/A\tC:\\project\\cyclic.cpp\t101\t2",
        "   Statement\tCallExpr\tfuncB\tC:\\project\\cyclic.cpp\t105\t3",                       // Calls funcB at different location
        " Declaration\tFunction\tc:@F@funcB\tC:\\project\\cyclic.cpp\t200\t4",                  // funcB also calls funcA
        "  Statement\tCompoundStmt\tN/A\tC:\\project\\cyclic.cpp\t201\t5",
        "   Statement\tCallExpr\tfuncA\tC:\\project\\cyclic.cpp\t202\t6"                        // Calls funcA
    });

    Tree firstAstTree("test_ast_1_cyclic.txt");
    Tree secondAstTree("test_ast_2_cyclic.txt");

    // usual database calls
    EXPECT_CALL(dbWrapper, clearDatabase()).Times(Exactly(1));
    EXPECT_CALL(dbWrapper, finalize()).Times(Exactly(1));

    // source location differences in calls
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_SOURCE_LOCATIONS", "FIRST_AST")).Times(Exactly(1));  // funcA's call to funcB
    EXPECT_CALL(dbWrapper, addNodeToBatch(_, _, "DIFFERENT_SOURCE_LOCATIONS", "SECOND_AST")).Times(Exactly(1)); // funcA's call to funcB

    // relationships
    EXPECT_CALL(dbWrapper, addRelationshipToBatch(_, _)).Times(Exactly(0)); // no relationships, parents are not displayed

    TreeComparer comparer(firstAstTree, secondAstTree, dbWrapper);
    comparer.printDifferences();
}