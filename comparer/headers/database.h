#ifndef _DATABASE_H_

#define _DATABASE_H_

#include <string>
#include <memory>
#include <SQLiteCpp/SQLiteCpp.h>
#include "enums.h"
#include "node.h"

class Database {
public:
   Database(const std::string);

   void insertNode(const Node*, const ASTId, const DifferenceType);
   void insertEdge(int, int);

private:
   SQLite::Database db;
   std::unique_ptr<SQLite::Statement> queryInsertNode;
   std::unique_ptr<SQLite::Statement> queryInsertEdge;  

   void createTables();
   void clearDatabase();
   void initializeStatements(); 
};

#endif