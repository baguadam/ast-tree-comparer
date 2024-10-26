#include "./headers/tree_comparer.h"
#include "./headers/tree.h"
#include "./headers/database.h"
#include <iostream>

void testDatabaseOperations() {
    // Create a database object
    Database db("db.sqlite3");

    // Insert some nodes
    db.insertNode(1, "Declaration", "Function", "usr1", "path/to/file1.cpp", "None");
    db.insertNode(2, "Declaration", "Class", "usr2", "path/to/file2.cpp", "None");
    db.insertNode(3, "Statement", "ReturnStmt", "usr3", "path/to/file3.cpp", "None");

    // Insert some edges
    db.insertEdge(2, 1); // Node 2 is a child of Node 1
    db.insertEdge(3, 2); // Node 3 is a child of Node 2

    std::cout << "Database operations completed successfully.\n";
}

int main() {
    testDatabaseOperations();

    Tree firstStandardAST("../../asts/first_standard_ast.txt");
    Tree secondStandardAST("../../asts/second_standard_ast.txt");

    TreeComparer comparer(firstStandardAST.getRoot(), secondStandardAST.getRoot());
    comparer.printDifferences();
}