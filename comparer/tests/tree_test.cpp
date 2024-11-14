#include <gtest/gtest.h>
#include "../headers/tree.h"
#include <fstream>
#include <filesystem>

class TreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // small valid AST file
        std::ofstream testFile("test_ast_1.txt");
        ASSERT_TRUE(testFile.is_open());
        testFile << "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n";
        testFile << " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n";
        testFile << "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n";     
        testFile.close();

        // large valid AST file with statements and multiple nodes
        std::ofstream testFile2("test_ast_2.txt");
        ASSERT_TRUE(testFile2.is_open());
        testFile2 << "Declaration\tTranslationUnit\tc:\tN/A\t0\t0\n";
        testFile2 << " Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\t308\t1\n";
        testFile2 << "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n";
        testFile2 << "  Declaration\tTypedef\tc:@N@std@T@size_t\tC:\\include\\bits\\c++config.h\t310\t3\n";
        testFile2 << "  Declaration\tFunction\tc:@F@doSomething\tC:\\include\\bits\\c++config.h\t350\t5\n";
        testFile2 << "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t351\t6\n";
        testFile2 << "    Declaration\tVar\tc:@F@doSomething@x\tC:\\include\\bits\\c++config.h\t352\t7\n";
        testFile2 << "    Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t353\t7\n";
        testFile2 << "    Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t354\t7\n";
        testFile2 << "  Declaration\tFunction\tc:@F@doSomethingElse\tC:\\include\\bits\\c++config.h\t400\t5\n";
        testFile2 << "   Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t401\t6\n";
        testFile2 << "    Declaration\tVar\tc:@F@doSomething@x\tC:\\include\\bits\\c++config.h\t402\t7\n";
        testFile2 << "    Statement\tExprStmt\tN/A\tC:\\include\\bits\\c++config.h\t403\t7\n";
        testFile2 << "    Statement\tReturnStmt\tN/A\tC:\\include\\bits\\c++config.h\t404\t7\n";
        testFile2.close();

        // small invalid AST file
        std::ofstream invalidFile("invalid_ast.txt");
        ASSERT_TRUE(invalidFile.is_open());
        invalidFile << "InvalidLineWithoutEnoughTokens\n";
        invalidFile.close();

        // empty AST file
        std::ofstream emptyFile("empty_ast.txt");
        ASSERT_TRUE(emptyFile.is_open());
        emptyFile.close();

        // check proper test writing
        ASSERT_TRUE(std::filesystem::exists("test_ast_1.txt"));
        ASSERT_TRUE(std::filesystem::exists("invalid_ast.txt"));
        ASSERT_TRUE(std::filesystem::exists("empty_ast.txt"));
    }

    void TearDown() override {
        if (std::filesystem::exists("test_ast_1.txt")) {
            std::filesystem::remove("test_ast_1.txt");
        }
        if (std::filesystem::exists("invalid_ast.txt")) {
            std::filesystem::remove("invalid_ast.txt");
        }
        if (std::filesystem::exists("empty_ast.txt")) {
            std::filesystem::remove("empty_ast.txt");
        }
        if (std::filesystem::exists("test_ast_2.txt")) {
            std::filesystem::remove("test_ast_2.txt");
        }
    }
};

// **********************************************
// Constructor tests
// **********************************************
// Test if constructing the tree with a nonexistent file throws an exception
TEST_F(TreeTest, TreeThrowsExceptionOnFileNotFound) {
    EXPECT_THROW({
        Tree testTree("nonexistent_file.txt");
    }, std::runtime_error);
}

// Test if constructing the tree with an invalid file format throws an exception
TEST_F(TreeTest, TreeThrowsExceptionOnInvalidFormat) {
    EXPECT_THROW({
        Tree testTree("invalid_ast.txt");
    }, std::runtime_error);
}

// Test if constructing the tree with an empty file throws an exception
TEST_F(TreeTest, TreeThrowsExceptionOnEmptyFile) {
    EXPECT_THROW({
        Tree testTree("empty_ast.txt");
    }, std::runtime_error);
}

// SUCCESSFUL tree construction from file
TEST_F(TreeTest, ConstructTreeFromFile) {
    ASSERT_NO_THROW({
        Tree testTree("test_ast_1.txt");
    });
}

// **********************************************
// buildTree tests for std::cerr outputs
// **********************************************
// Test to capture `std::cerr` output when reading an invalid file format
// Test to capture `std::cerr` output when reading an invalid file format
TEST_F(TreeTest, CaptureStderrForInvalidFormat) {
    std::ifstream file("invalid_ast.txt");
    std::stringstream buffer;
    std::streambuf* oldCerrBuffer = std::cerr.rdbuf(buffer.rdbuf());

    try {
        Tree testTree("invalid_ast.txt");
    } catch (const std::runtime_error&) {
        // suppress the runtime error to continue checking the `std::cerr` output
    }

    std::cerr.rdbuf(oldCerrBuffer);
    std::string capturedOutput = buffer.str();

    // check that the captured output contains the expected warning
    EXPECT_NE(capturedOutput.find("Warning: Invalid line in the file"), std::string::npos);
}

// Test to capture `std::cerr` output when declaration parent is not found
TEST_F(TreeTest, CaptureStderrForMissingDeclarationParent) {
    std::ofstream file("missing_decl_parent.txt");
    ASSERT_TRUE(file.is_open());
    file << "Statement\tCompoundStmt\tN/A\tC:\\include\\bits\\c++config.h\t320\t16\n"; // no declaration before this statement
    file.close();

    std::stringstream buffer;
    std::streambuf* oldCerrBuffer = std::cerr.rdbuf(buffer.rdbuf());

    try {
        Tree testTree("missing_decl_parent.txt");
    } catch (const std::runtime_error&) {
        // suppress runtime error to check captured output
    }

    std::cerr.rdbuf(oldCerrBuffer);
    std::string capturedOutput = buffer.str();

    // check the expected output to ensure it matches the missing parent warning
    EXPECT_NE(capturedOutput.find("Warning: Could not find declaration parent for statement node"), std::string::npos);

    // cleanup
    std::filesystem::remove("missing_decl_parent.txt");
}

// Test to capture `std::cerr` output when line or column number cannot be parsed
TEST_F(TreeTest, CaptureStderrForInvalidLineOrColumn) {
    std::ofstream invalidColumnFile("invalid_column.txt");
    ASSERT_TRUE(invalidColumnFile.is_open());
    invalidColumnFile << "Declaration\tNamespace\tc:@N@std\tC:\\include\\bits\\c++config.h\tABC\t123\n"; // invalid line number
    invalidColumnFile.close();

    std::stringstream buffer;
    std::streambuf* oldCerrBuffer = std::cerr.rdbuf(buffer.rdbuf());

    // expect an exception
    EXPECT_THROW({
        Tree testTree("invalid_column.txt");
    }, std::runtime_error);

    std::cerr.rdbuf(oldCerrBuffer);
    std::string capturedOutput = buffer.str();

    // expect std::cerr log
    EXPECT_NE(capturedOutput.find("ERROR: Failed to parse line or column number"), std::string::npos);

    // Cleanup
    std::filesystem::remove("invalid_column.txt");
}

// **********************************************
// Node creation, storing, retrieving from maps tests - WITH SMALL DATASET
// **********************************************
// Test root node existence and properties after successful construction
TEST_F(TreeTest, CheckRootNodeAfterConstruction) {
    Tree testTree("test_ast_1.txt");
    Node* root = testTree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->kind, "TranslationUnit");
    EXPECT_EQ(root->path, "N/A");
    EXPECT_EQ(root->lineNumber, 0);
    EXPECT_EQ(root->columnNumber, 0);
}

// Test if declaration node return is empty when no declaration nodes are present
TEST_F(TreeTest, AccessDeclarationNodeByNotExistingEnhancedKey) {
    Tree testTree("test_ast_1.txt");

    auto declRange = testTree.getDeclNodes("c:@N@std@T@size_t");
    ASSERT_EQ(declRange.first, declRange.second); // no declaration node with the enhanced key
}

// Test accessing a declaration node by its enhanced key
TEST_F(TreeTest, AccessDeclarationNodeByEnhancedKey) {
    Tree testTree("test_ast_1.txt");

    // generate the enhanced key for the declaration node we want to access
    std::string kind = "Namespace";
    std::string usr = "c:@N@std";
    std::string path = "C:\\include\\bits\\c++config.h";
    std::string enhancedKey = kind + "|" + usr + "|" + path + "|";

    // retrieve declaration nodes using the generated enhanced key
    auto declNodes = testTree.getDeclNodes(enhancedKey);

    // ensure the iterator range is valid and points to the correct node
    ASSERT_NE(declNodes.first, declNodes.second); // there should be at least one node with this key
    const Node* node = declNodes.first->second;
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->kind, kind);
    EXPECT_EQ(node->usr, usr);
    EXPECT_EQ(node->path, path);
}

// Test if we can iterate over all declaration nodes and find specific keys
TEST_F(TreeTest, IterateOverDeclarationNodes) {
    Tree testTree("test_ast_1.txt");

    // retrieve the entire declaration map
    const auto& declMap = testTree.getDeclNodeMultiMap();

    // ensure the map is not empty
    ASSERT_FALSE(declMap.empty());
    ASSERT_TRUE(declMap.size() == 3);

    // check if we can find a specific node by iterating over the map
    bool found = false;
    std::string targetKey = "Typedef|c:@N@std@T@size_t|C:\\include\\bits\\c++config.h|";
    for (const auto& pair : declMap) {
        if (pair.first == targetKey) {
            found = true;
            EXPECT_EQ(pair.second->kind, "Typedef");
            EXPECT_EQ(pair.second->usr, "c:@N@std@T@size_t");
            EXPECT_EQ(pair.second->path, "C:\\include\\bits\\c++config.h");
            break;
        }
    }

    EXPECT_TRUE(found);
}

// Test if in case if no statement nodes are present, the map is empty
TEST_F(TreeTest, NoStatementNodes) {
    Tree testTree("test_ast_1.txt");

    const auto& stmtMultiMap = testTree.getStmtNodeMultiMap();
    EXPECT_TRUE(stmtMultiMap.empty());
}

// Test the `getStmtNodes` method when there are no statement nodes
TEST_F(TreeTest, NoStatementNodesForGivenKey) {
    Tree testTree("test_ast_1.txt");

    std::string kind = "Typedef";
    std::string usr = "c:@N@std@T@size_t";
    std::string path = "C:\\include\\bits\\c++config.h";
    std::string enhancedKey = kind + "|" + usr + "|" + path + "|";

    auto declNode = testTree.getDeclNodes(enhancedKey);
    ASSERT_NE(declNode.first, declNode.second);

    std::string stmtKey = enhancedKey + "|" + std::to_string(declNode.first->second->topologicalOrder);
    auto stmtNodes = testTree.getStmtNodes(stmtKey);

    EXPECT_EQ(stmtNodes.first, stmtNodes.second);
}

// **********************************************
// Node storing and retrieving from maps tests - WITH LARGE DATASET
// **********************************************
// Test if multiple declaration nodes with the same key are stored correctly
TEST_F(TreeTest, CheckDeclarationNodesInAST) {
    Tree testTree("test_ast_2.txt");
    // ensure the number of unique declaration keys is correct., in `test_ast_2.txt`, the declarations are:
    // - TranslationUnit
    // - Namespace: std
    // - Typedef: size_t (repeated twice, so only one unique key)
    // - Function: doSomething
    // - Function: doSomethingElse
    // - Variable: x (inside both functions, treated as two separate declarations)
    auto declMap = testTree.getDeclNodeMultiMap();
    EXPECT_EQ(declMap.size(), 8); // all 8 nodes get added

    std::string typedefKey = "Typedef|c:@N@std@T@size_t|C:\\include\\bits\\c++config.h|";
    auto typedefRange = testTree.getDeclNodes(typedefKey);
    size_t typedefCount = std::distance(typedefRange.first, typedefRange.second);
    EXPECT_EQ(typedefCount, 2);  // expect two entries for typedef `size_t` with identical key

    std::string functionKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    auto functionRange = declMap.equal_range(functionKey);
    size_t functionCount = std::distance(functionRange.first, functionRange.second);
    EXPECT_EQ(functionCount, 1);  // expect one entry for `doSomething`

    std::string functionElseKey = "Function|c:@F@doSomethingElse|C:\\include\\bits\\c++config.h|";
    auto functionElseRange = declMap.equal_range(functionElseKey);
    size_t functionElseCount = std::distance(functionElseRange.first, functionElseRange.second);
    EXPECT_EQ(functionElseCount, 1);  // expect one entry for `doSomethingElse`

    std::string varKey = "Var|c:@F@doSomething@x|C:\\include\\bits\\c++config.h|";
    auto varRange = declMap.equal_range(varKey);
    size_t varCount = std::distance(varRange.first, varRange.second);
    EXPECT_EQ(varCount, 2);  // expect two entries for `x` due to repeated declaration

    std::string namespaceKey = "Namespace|c:@N@std|C:\\include\\bits\\c++config.h|";
    auto namespaceRange = declMap.equal_range(namespaceKey);
    size_t namespaceCount = std::distance(namespaceRange.first, namespaceRange.second);
    EXPECT_EQ(namespaceCount, 1);  // expect one entry for namespace `std`

    std::string translationUnitKey = "TranslationUnit|c:|N/A|";
    auto translationUnitRange = declMap.equal_range(translationUnitKey);
    size_t translationUnitCount = std::distance(translationUnitRange.first, translationUnitRange.second);
    EXPECT_EQ(translationUnitCount, 1);  // expect one entry for `TranslationUnit`
}

// Test if the statement nodes are stored correctly 
TEST_F(TreeTest, CheckStatementMultiMapSize) {
    Tree testTree("test_ast_2.txt");

    auto stmtMultiMap = testTree.getStmtNodeMultiMap();
    EXPECT_EQ(stmtMultiMap.size(), 2); // only two functions have statements
}

// helper fundtion
void CheckStatementsForFunction(Tree& testTree, const std::string& functionKey, 
                                const std::vector<std::string>& expectedStmtKinds, 
                                const std::vector<std::pair<int, int>>& expectedLineCols) {
    auto declNodeRange = testTree.getDeclNodes(functionKey);
    ASSERT_NE(declNodeRange.first, declNodeRange.second) << "Function with key " << functionKey << " not found in AST";

    Node* declNode = declNodeRange.first->second;

    // key for stmt statements
    std::string stmtKey = declNode->enhancedKey + "|" + std::to_string(declNode->topologicalOrder);
    auto stmtNodes = testTree.getStmtNodes(stmtKey);

    size_t stmtCount = std::distance(stmtNodes.first, stmtNodes.second);
    EXPECT_EQ(stmtCount, expectedStmtKinds.size()) << "Statement count mismatch for function: " << functionKey;

    auto stmtIt = stmtNodes.first;
    for (size_t i = 0; i < expectedStmtKinds.size() && stmtIt != stmtNodes.second; ++i, ++stmtIt) {
        Node* stmtNode = *stmtIt;

        EXPECT_EQ(stmtNode->kind, expectedStmtKinds[i]) << "Statement kind mismatch at index " << i << " for function " << functionKey;

        EXPECT_EQ(stmtNode->lineNumber, expectedLineCols[i].first) << "Line number mismatch for statement kind " << expectedStmtKinds[i] << " at index " << i;
        EXPECT_EQ(stmtNode->columnNumber, expectedLineCols[i].second) << "Column number mismatch for statement kind " << expectedStmtKinds[i] << " at index " << i;
    }
}

// Test if the statement nodes are stored correctly for each function
TEST_F(TreeTest, CheckStatementsForFunctions) {
    Tree testTree("test_ast_2.txt");

    // test for `doSomething` function
    std::string funcDoSomethingKey = "Function|c:@F@doSomething|C:\\include\\bits\\c++config.h|";
    std::vector<std::string> expectedStmtKindsDoSomething = {"CompoundStmt", "ExprStmt", "ReturnStmt"};
    std::vector<std::pair<int, int>> expectedLineColsDoSomething = {{351, 6}, {353, 7}, {354, 7}};
    CheckStatementsForFunction(testTree, funcDoSomethingKey, expectedStmtKindsDoSomething, expectedLineColsDoSomething);

    // test for `doSomethingElse` function
    std::string funcDoSomethingElseKey = "Function|c:@F@doSomethingElse|C:\\include\\bits\\c++config.h|";
    std::vector<std::string> expectedStmtKindsDoSomethingElse = {"CompoundStmt", "ExprStmt", "ReturnStmt"};
    std::vector<std::pair<int, int>> expectedLineColsDoSomethingElse = {{401, 6}, {403, 7}, {404, 7}};
    CheckStatementsForFunction(testTree, funcDoSomethingElseKey, expectedStmtKindsDoSomethingElse, expectedLineColsDoSomethingElse);
}