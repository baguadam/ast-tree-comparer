#ifndef NEO4J_DATABASE_WRAPPER_H
#define NEO4J_DATABASE_WRAPPER_H

#include <string>
#include <curl/curl.h>
#include <memory>
#include <vector>
#include "node.h"  // Assuming Node struct is declared here

class Neo4jDatabaseWrapper {
public:
    Neo4jDatabaseWrapper(const std::string& uri, const std::string& username, const std::string& password);
    ~Neo4jDatabaseWrapper();

    void addNodeToBatch(const Node& node, bool isHighLevel, const std::string&, const std::string&);
    void addRelationshipToBatch(const Node& parent, const Node& child);
    void clearDatabase();
    void finalize();

private:
    std::string dbUri;
    std::string authHeader;

    CURL* curl;

    std::vector<std::string> nodeBatch;
    std::vector<std::string> relationshipBatch;

    void createIndices();
    void executeBatch();
    void sendRequest(const std::string& queryJson);
};

#endif