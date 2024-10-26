#ifndef _DATABASE_H_

#define _DATABASE_H_

#include <string>

class Database {
public:
   Database(const std::string);
   void insertNode(int, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);
   void insertEdge(int, int);

private:
   SQLite::Database db;
   SQLite::Statement queryInsertNode;
   SQLite::Statement queryInsertEdge;   
};

#endif