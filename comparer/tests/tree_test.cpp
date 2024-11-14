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
    }
};

// **********************************************
// CONSTRUCTOR TESTS
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
// node creation and storing tests
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