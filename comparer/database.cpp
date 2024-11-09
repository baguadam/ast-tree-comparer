#include <iostream>
#include "./headers/database.h"

Database::Database(const std::string dbPath) : db(dbPath, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE) {
    std::cout << "Initializing database with path: " << dbPath << std::endl;
    clearDatabase();
    createTables();
    initializeStatements();
}

void Database::clearDatabase() {
    try {
        std::cout << "Dropping existing tables if they exist..." << std::endl;
        db.exec("DROP TABLE IF EXISTS Nodes;");
        db.exec("DROP TABLE IF EXISTS Edges;");
        std::cout << "Existing tables dropped successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error clearing database: " << e.what() << std::endl;
    }
}

void Database::initializeStatements() {
    try {
        queryInsertNode = std::make_unique<SQLite::Statement>(db, "INSERT INTO Nodes(key, type, kind, usr, path, differenceType, AST, isHighestLevelNode, comment) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
        queryInsertEdge = std::make_unique<SQLite::Statement>(db, "INSERT INTO Edges(childId, parentId) VALUES (?, ?)");
    } catch (const std::exception& e) {
        std::cerr << "Error initializing statements: " << e.what() << std::endl;
    }
}

void Database::createTables() {
    try {
        std::cout << "Creating Nodes table..." << std::endl;
        db.exec("CREATE TABLE IF NOT EXISTS Nodes ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "key TEXT NOT NULL,"
                "type INTEGER NOT NULL,"
                "kind TEXT NOT NULL,"
                "usr TEXT NOT NULL,"
                "path TEXT NOT NULL,"
                "differenceType INTEGER NOT NULL,"
                "AST INTEGER NOT NULL,"
                "isHighestLevelNode BOOLEAN DEFAULT 0,"
                "comment TEXT);");
        std::cout << "Nodes table created successfully." << std::endl;

        std::cout << "Creating Edges table..." << std::endl;
        db.exec("CREATE TABLE IF NOT EXISTS Edges ("
                "childId INTEGER NOT NULL,"
                "parentId INTEGER NOT NULL,"
                "FOREIGN KEY(childId) REFERENCES Nodes(id),"
                "FOREIGN KEY(parentId) REFERENCES Nodes(id));");
        std::cout << "Edges table created successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error creating tables: " << e.what() << std::endl;
    }
}

void Database::insertNode(const Node* node, const ASTId astId, const DifferenceType differenceType, bool isHighestLevelNode) {
    try {
        queryInsertNode->bind(1, node->enhancedKey);  
        queryInsertNode->bind(2, node->type);
        queryInsertNode->bind(3, node->kind);
        queryInsertNode->bind(4, node->usr);
        queryInsertNode->bind(5, node->path);
        queryInsertNode->bind(6, differenceType);
        queryInsertNode->bind(7, astId);
        queryInsertNode->bind(8, isHighestLevelNode);
        queryInsertNode->bind(9, "");
        queryInsertNode->exec();
        queryInsertNode->reset();
    } catch (const std::exception& e) {
        std::cerr << "Error inserting node: " << e.what() << std::endl; 
    }
}

void Database::insertEdge(const std::string& childId, const std::string& parentId) {
    try {
        queryInsertEdge->bind(1, childId);
        queryInsertEdge->bind(2, parentId);
        queryInsertEdge->exec();
        queryInsertEdge->reset();
    } catch (const std::exception& e) {
        std::cerr << "Error inserting edge: " << e.what() << std::endl;
    }
}