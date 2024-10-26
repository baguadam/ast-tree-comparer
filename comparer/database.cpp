#include <iostream>
#include "./headers/database.h"

Database::Database(const std::string dbPath) : db(dbPath) {
    std::cout << "Initializing database with path: " << dbPath << std::endl;
    // createTables();
}

// void Database::createTables() {
//     try {
//         std::cout << "Creating Nodes table..." << std::endl;
//         db.exec("CREATE TABLE IF NOT EXISTS Nodes ("
//                 "id INTEGER PRIMARY KEY,"
//                 "type TEXT NOT NULL,"
//                 "kind TEXT NOT NULL,"
//                 "usr TEXT NOT NULL,"
//                 "path TEXT NOT NULL,"
//                 "differenceType TEXT NOT NULL,"
//                 "comment TEXT);");
//         std::cout << "Nodes table created successfully." << std::endl;

//         std::cout << "Creating Edges table..." << std::endl;
//         db.exec("CREATE TABLE IF NOT EXISTS Edges ("
//                 "childId INTEGER NOT NULL,"
//                 "parentId INTEGER NOT NULL,"
//                 "FOREIGN KEY(childId) REFERENCES Nodes(id),"
//                 "FOREIGN KEY(parentId) REFERENCES Nodes(id));");
//         std::cout << "Edges table created successfully." << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "Error creating tables: " << e.what() << std::endl;
//     }
// }

// void Database::insertNode(int id, const std::string& type, const std::string& kind, const std::string& usr, const std::string& path, const std::string& differenceType) {
//     try {
//         std::cout << "Inserting node with id: " << id << std::endl;
//         queryInsertNode.bind(1, id);
//         queryInsertNode.bind(2, type);
//         queryInsertNode.bind(3, kind);
//         queryInsertNode.bind(4, usr);
//         queryInsertNode.bind(5, path);
//         queryInsertNode.bind(6, differenceType);
//         queryInsertNode.bind(7, "");
//         queryInsertNode.exec();
//         queryInsertNode.reset();
//         std::cout << "Node inserted successfully." << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "Error inserting node: " << e.what() << std::endl; 
//     }
// }

// void Database::insertEdge(int childId, int parentId) {
//     try {
//         std::cout << "Inserting edge from childId: " << childId << " to parentId: " << parentId << std::endl;
//         queryInsertEdge.bind(1, childId);
//         queryInsertEdge.bind(2, parentId);
//         queryInsertEdge.exec();
//         queryInsertEdge.reset();
//         std::cout << "Edge inserted successfully." << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "Error inserting edge: " << e.what() << std::endl;
//     }
// }