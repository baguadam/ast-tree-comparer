#ifndef NEO4J_DATABASE_WRAPPER_H
#define NEO4J_DATABASE_WRAPPER_H

#include <string>
#include <curl/curl.h>
#include <memory>
#include <vector>
#include "node.h"
#include "idatabase_wrapper.h"

class Neo4jDatabaseWrapper : public IDatabaseWrapper {
public:
    Neo4jDatabaseWrapper(const std::string&, const std::string&, const std::string&);
    ~Neo4jDatabaseWrapper() override;

    void addNodeToBatch(const Node&, bool, const std::string&, const std::string&) override;
    void addRelationshipToBatch(const Node&, const Node&) override;
    void clearDatabase() override;
    void finalize() override;

private:
    std::string dbUri;
    std::string authHeader;

    CURL* curl;

    std::vector<std::string> nodeBatch;
    std::vector<std::string> relationshipBatch;

    void createIndices();
    void executeBatch();
    void sendRequest(const std::string&);
};

#endif