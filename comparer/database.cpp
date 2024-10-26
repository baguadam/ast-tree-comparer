#include <iostream>
#include "./headers/database.h"

Database::Database(const std::string dbPath) : db(dbPath),
                                               queryInsertNode(db, "INSERT INTO Nodes(id, type, kind, usr, path, differenceType, comment) VALUES (?, ?, ?, ?, ?, ?, ?)"),
                                               queryInsertEdge(db, "INSERT INTO Edges(childId, parentId) VALUES (?, ?)") {
 
    createTables();
}

void Database::createTables() {
    try {
        // create Nodes table
        db.exec("CREATE TABLE IF NOT EXISTS Nodes ("
                "id INTEGER PRIMARY KEY,"
                "type TEXT NOT NULL,"
                "kind TEXT NOT NULL,"
                "usr TEXT NOT NULL,"
                "path TEXT NOT NULL,"
                "differenceType TEXT NOT NULL,"
                "comment TEXT);");
        
        // create Edges table
        db.exec("CREATE TABLE IF NOT EXISTS Edges ("
                "childId INTEGER NOT NULL,"
                "parentId INTEGER NOT NULL,"
                "FOREIGN KEY(childId) REFERENCES Nodes(id),"
                "FOREIGN KEY(parentId) REFERENCES Nodes(id));");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void Database::insertNode(int id, const std::string& type, const std::string& kind, const std::string& usr, const std::string& differenceType, const std::string& path) {
    try {
        queryInsertNode.bind(1, id);
        queryInsertNode.bind(2, type);
        queryInsertNode.bind(3, kind);
        queryInsertNode.bind(4, usr);
        queryInsertNode.bind(5, path);
        queryInsertNode.bind(6, differenceType);
        queryInsertNode.bind(7, "");
        queryInsertNode.exec();
        queryInsertNode.reset();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl; 
    }
}

void Database::insertEdge(int childId, int parentId) {
    try {
        queryInsertEdge.bind(1, childId);
        queryInsertEdge.bind(2, parentId);
        queryInsertEdge.exec();
        queryInsertEdge.reset();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}