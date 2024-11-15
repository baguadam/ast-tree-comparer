#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../headers/tree.h"
#include "../headers/tree_comparer.h"
#include "mock_database_wrapper.h"
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
};

// Test for database operation calls
TEST_F(TreeComparerTest, TestDatabaseOperationCalls) {
    // Construct the trees using test AST files.
    Tree firstTree("test_ast_1.txt");
    Tree secondTree("test_ast_2.txt");

    // Ensure that both trees are properly loaded and have valid roots.
    ASSERT_NE(firstTree.getRoot(), nullptr) << "First tree root is null.";
    ASSERT_NE(secondTree.getRoot(), nullptr) << "Second tree root is null.";

    // Create the mock database wrapper directly as a unique_ptr.
    MockDatabaseWrapper dbWrapper;

    // Construct the TreeComparer instance with the mock database wrapper.
    TreeComparer comparer(firstTree, secondTree, dbWrapper);

    // Run the comparison.
    comparer.printDifferences();
}

