/* Updated neo4j_database_wrapper.cpp */
#include "../include/neo4j_database_wrapper.h"
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "../include/utils.h"
#include <curl/curl.h>
#include <iomanip>

using json = nlohmann::json;

Neo4jDatabaseWrapper::Neo4jDatabaseWrapper(const std::string& uri, const std::string& username, const std::string& password)
    : dbUri(uri + "/db/neo4j/tx/commit") {
    // initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    // create authentication header (Basic Auth)
    std::string credentials = username + ":" + password;
    std::string encoded_credentials = Utils::base64Encode(credentials);
    authHeader = "Authorization: Basic " + encoded_credentials;
}

Neo4jDatabaseWrapper::~Neo4jDatabaseWrapper() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void Neo4jDatabaseWrapper::addNodeToBatch(const Node& node, bool isHighLevel, const std::string& differenceType, const std::string& astOrigin) {
    // create a JSON object for the node
    json nodeJson = {
        {"enhancedKey", node.enhancedKey},
        {"topologicalOrder", node.topologicalOrder},
        {"type", node.type},
        {"kind", node.kind},
        {"usr", node.usr},
        {"path", node.path},
        {"lineNumber", node.lineNumber},
        {"columnNumber", node.columnNumber},
        {"isHighLevel", isHighLevel},
        {"differenceType", differenceType},
        {"astOrigin", astOrigin}
    };

    // add node JSON to the batch
    nodeBatch.push_back(nodeJson);

    // if the batch size exceeds 1000, execute the batch
    if (nodeBatch.size() >= 3000 && !executeBatch()) {
        std::cerr << "Execution failed for node batch." << std::endl;
    }
}

void Neo4jDatabaseWrapper::addRelationshipToBatch(const Node& parent, const Node& child) {
    // create a JSON object for the relationship
    json relationshipJson = {
        {"parentKey", parent.enhancedKey},
        {"parentOrder", parent.topologicalOrder},
        {"childKey", child.enhancedKey},
        {"childOrder", child.topologicalOrder}
    };

    // add relationship JSON to the batch
    relationshipBatch.push_back(relationshipJson);

    // if the batch size exceeds 1000, execute the batch
    if (relationshipBatch.size() >= 3000 && !executeBatch()) {
        std::cerr << "Execution failed for relationship batch." << std::endl;
    }
}

bool Neo4jDatabaseWrapper::executeBatch() {
    if (isCircuitBreakerActive) {
        return false;
    }

    if (nodeBatch.empty() && relationshipBatch.empty()) {
        return true; // nothing to execute
    }

    json requestBody = {
        {"statements", json::array()}
    };

    if (!nodeBatch.empty()) {
        json nodeStatement = {
            {"statement", "UNWIND $nodes AS node "
                          "CREATE (n:Node {enhancedKey: node.enhancedKey, type: node.type, kind: node.kind, usr: node.usr, "
                          "path: node.path, lineNumber: node.lineNumber, columnNumber: node.columnNumber, "
                          "topologicalOrder: node.topologicalOrder, isHighLevel: node.isHighLevel, diffType: node.differenceType, ast: node.astOrigin})"},
            {"parameters", {{"nodes", nodeBatch}}}
        };
        requestBody["statements"].push_back(nodeStatement);
    }

    if (!relationshipBatch.empty()) {
        json relationshipStatement = {
            {"statement", "UNWIND $relationships AS rel "
                          "MATCH (a:Node {enhancedKey: rel.parentKey, topologicalOrder: rel.parentOrder}) "
                          "WITH a, rel "
                          "MATCH (b:Node {enhancedKey: rel.childKey, topologicalOrder: rel.childOrder}) "
                          "CREATE (a)-[:HAS_CHILD]->(b)"},
            {"parameters", {{"relationships", relationshipBatch}}}
        };
        requestBody["statements"].push_back(relationshipStatement);
    }

    // serialize requestBody to a string
    std::string queryJson = requestBody.dump();

    // send request
    if (!sendRequest(queryJson)) {
        consecutiveFailures++;
        std::cerr << "Failed to execute batch. Consecutive failures: " << consecutiveFailures << std::endl;

        if (consecutiveFailures >= failureThreshold) {
            isCircuitBreakerActive = true;
            std::cerr << "Circuit breaker activated after " << failureThreshold << " failures." << std::endl;
        }

        nodeBatch.clear();
        relationshipBatch.clear();
        return false;
    }

    // reset state on success
    consecutiveFailures = 0;
    nodeBatch.clear();
    relationshipBatch.clear();
    return true;


    
}

bool Neo4jDatabaseWrapper::sendRequest(const std::string& queryJson) {
    if (isCircuitBreakerActive) {
        std::cerr << "Circuit breaker active. Skipping request." << std::endl;
        return false;
    }

    if (!curl) {
        std::cerr << "CURL initialization failed" << std::endl;
        return false;
    }

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, dbUri.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, queryJson.c_str());

    // retry mechanism
    int retryCount = 0;
    const int maxRetries = 3;
    CURLcode res;
    do {
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL request failed: " << curl_easy_strerror(res)
                      << " - Retrying (" << retryCount + 1 << "/" << maxRetries << ")" << std::endl;
            retryCount++;
        } else {
            break; // request succeeded
        }
    } while (retryCount < maxRetries);

    // cleanup
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "CURL request ultimately failed after retries: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    return true;
}

void Neo4jDatabaseWrapper::clearDatabase() {
    json query = {
        {"statements", {{
            {"statement", "MATCH (n) DETACH DELETE n"}
        }}}
    };
    if (sendRequest(query.dump())) {
        std::cout << "Database cleared successfully." << std::endl;
    } else {
        throw std::runtime_error("Failed to clear the Neo4j database.");
    }
}

void Neo4jDatabaseWrapper::createIndices() {
    std::vector<std::string> indexStatements = {
        "CREATE INDEX IF NOT EXISTS FOR (n:Node) ON (n.enhancedKey)",
        "CREATE INDEX IF NOT EXISTS FOR (n:Node) ON (n.differenceType)",
        "CREATE INDEX IF NOT EXISTS FOR (n:Node) ON (n.astOrigin)"
    };

    for (const auto& statement : indexStatements) {
        json query = {
            {"statements", {{
                {"statement", statement}
            }}}
        };
        if (!sendRequest(query.dump())) {
            throw std::runtime_error("Failed to create indices in the Neo4j database.");
        }
    }

    std::cout << "Indices created successfully." << std::endl;
}

void Neo4jDatabaseWrapper::finalize() {
    // execute any remaining batches
    if ((!nodeBatch.empty() || !relationshipBatch.empty()) && !executeBatch()) {
        std::cerr << "Failed to execute remaining batch." << std::endl;
        nodeBatch.clear();
        relationshipBatch.clear();
    }
}