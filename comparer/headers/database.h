#ifndef _DATABASE_H_

#define _DATABASE_H_

#include <string>
#include <memory>
#include "enums.h"
#include "node.h"

class Database {
public:
   Database(const std::string);

   void insertNode(const Node*, const ASTId, const DifferenceType, bool isHighestLevelNode = false);
   void insertEdge(const std::string&, const std::string&);

private:
   void createTables();
   void clearDatabase();
   void initializeStatements(); 
};

#endif