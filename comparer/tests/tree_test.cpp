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

        // check proper test writing
        ASSERT_TRUE(std::filesystem::exists("test_ast_1.txt"));
        ASSERT_TRUE(std::filesystem::exists("invalid_ast.txt"));
    }


    void TearDown() override {
        if (std::filesystem::exists("test_ast_1.txt")) {
            std::filesystem::remove("invalid_ast.txt");
        }
    }
};

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

// SUCCESSFUL tree construction from file
TEST_F(TreeTest, ConstructTreeFromFile) {
    ASSERT_NO_THROW({
        Tree testTree("test_ast_1.txt");
    });
}