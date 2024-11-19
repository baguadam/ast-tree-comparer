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
    void createIndices();
    void finalize() override;

private:
    bool isCircuitBreakerActive = false; // prevents execution if set
    int consecutiveFailures = 0;       // counts consecutive failures
    const int failureThreshold = 3;    // maximum allowed failures

    std::string dbUri;
    std::string authHeader;
    CURL* curl;

    std::vector<std::string> nodeBatch;
    std::vector<std::string> relationshipBatch;

    bool executeBatch();
    bool sendRequest(const std::string&);
};

#endif