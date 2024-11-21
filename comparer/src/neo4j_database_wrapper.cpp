#include "../include/neo4j_database_wrapper.h"
#include <iostream>
#include <sstream>
#include "../include/utils.h"
#include <curl/curl.h>
#include <iomanip>

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

/*
Description:
    Adds a node to the batch for creation in the database, if the batch size exceeds 5000, the batch is executed, also \ characters are properly escaped
    for valid JSON syntax
*/
void Neo4jDatabaseWrapper::addNodeToBatch(const Node& node, bool isHighLevel, const std::string& differenceType, const std::string& astOrigin) {
    // properly escape backslashes in the path and enhancedKey
    std::string escapedPath = Utils::escapeString(node.path);
    std::string escapedEnhancedKey = Utils::escapeString(node.enhancedKey);

    std::ostringstream nodeStream;
    nodeStream << "{\"enhancedKey\": \"" << escapedEnhancedKey
               << "\", \"topologicalOrder\": " << node.topologicalOrder
               << ", \"type\": \"" << node.type
               << "\", \"kind\": \"" << node.kind
               << "\", \"usr\": \"" << node.usr
               << "\", \"path\": \"" << escapedPath
               << "\", \"lineNumber\": " << node.lineNumber
               << ", \"columnNumber\": " << node.columnNumber
               << ", \"isHighLevel\": " << (isHighLevel ? "true" : "false")
               << ", \"differenceType\": \"" << differenceType
               << "\", \"astOrigin\": \"" << astOrigin << "\""
               << "}";

    nodeBatch.push_back(nodeStream.str());

    // if the batch size exceeds 5000, execute the batch
    if (nodeBatch.size() >= 5000 && !executeBatch()) {
        std::cerr << "Execution failed for node batch." << std::endl;
    }
}

/*
Description:
    Adds a relationship to the batch for creation in the database, if the batch size exceeds 5000, the batch is executed, also \ characters are properly escaped
    for valid JSON syntax
*/
void Neo4jDatabaseWrapper::addRelationshipToBatch(const Node& parent, const Node& child) {
    // Properly escape backslashes in the parent and child keys
    std::string escapedParentKey = Utils::escapeString(parent.enhancedKey);
    std::string escapedChildKey = Utils::escapeString(child.enhancedKey);

    std::ostringstream relStream;
    relStream << "{\"parentKey\": \"" << escapedParentKey
              << "\", \"parentOrder\": " << parent.topologicalOrder
              << ", \"childKey\": \"" << escapedChildKey
              << "\", \"childOrder\": " << child.topologicalOrder
              << "}";

    relationshipBatch.push_back(relStream.str());

    // if the batch size exceeds 5000, execute the batch
    if (relationshipBatch.size() >= 5000 && !executeBatch()) {
        std::cerr << "Execution failed for node batch." << std::endl;
    }
}

/*
Description:
    Executes the batch of nodes and relationships in the database
*/
bool Neo4jDatabaseWrapper::executeBatch() {
    if (isCircuitBreakerActive) {
        return false;
    }

    if (nodeBatch.empty() && relationshipBatch.empty()) {
        return true; // nothing to execute
    }

    std::ostringstream queryStream;
    queryStream << "{\"statements\": [";

    bool hasNodes = !nodeBatch.empty();
    bool hasRelationships = !relationshipBatch.empty();

    // if there are nodes to be created, add the node creation query.
    if (hasNodes) {
        queryStream << "{\"statement\": \"UNWIND $nodes AS node "
                    << "CREATE (n:Node {enhancedKey: node.enhancedKey, type: node.type, kind: node.kind, usr: node.usr, "
                    << "path: node.path, lineNumber: node.lineNumber, columnNumber: node.columnNumber, "
                    << "topologicalOrder: node.topologicalOrder, isHighLevel: node.isHighLevel, diffType: node.differenceType, ast: node.astOrigin})\", "
                    << "\"parameters\": {\"nodes\": [";

        // JSON format for nodes
        for (size_t i = 0; i < nodeBatch.size(); ++i) {
            if (i > 0) queryStream << ",";
            queryStream << nodeBatch[i];
        }

        queryStream << "]}}";
    }

    // if there are relationships to be created, add the relationship creation query.
    if (hasRelationships) {
        if (hasNodes) {
            queryStream << ",";  // add a comma to separate the two queries
        }

        queryStream << "{\"statement\": \"UNWIND $relationships AS rel "
                    << "MATCH (a:Node {enhancedKey: rel.parentKey, topologicalOrder: rel.parentOrder}) "
                    << "WITH a, rel "
                    << "MATCH (b:Node {enhancedKey: rel.childKey, topologicalOrder: rel.childOrder}) "
                    << "CREATE (a)-[:HAS_CHILD]->(b)\", "
                    << "\"parameters\": {\"relationships\": [";

        // JSON format for relationships
        for (size_t i = 0; i < relationshipBatch.size(); ++i) {
            if (i > 0) queryStream << ",";
            queryStream << relationshipBatch[i];
        }

        queryStream << "]}}";
    }

    queryStream << "]}";

    // send request
    if (!sendRequest(queryStream.str())) {
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

/*
Description:
    Sends a request to the Neo4j database with the given query JSON
*/
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

/*
Description:
    Clears the database by deleting all nodes and relationships
*/
void Neo4jDatabaseWrapper::clearDatabase() {
    std::string query = "{\"statements\": [{\"statement\": \"MATCH (n) DETACH DELETE n\"}]}";
    if (sendRequest(query)) {
        std::cout << "Database cleared successfully." << std::endl;
    } else {
        throw std::runtime_error("Failed to clear the Neo4j database.");
    }
}

/*
Description:
    Creates the necessary indices for the database, in our case so far it's enough to create an index for the enhancedKey property of the Node
*/
void Neo4jDatabaseWrapper::createIndices() {
    std::string query = "{\"statements\": [{\"statement\": \"CREATE INDEX IF NOT EXISTS FOR (n:Node) ON (n.enhancedKey)\"}]}"; 
    std::string indexDifferenceType = "{\"statements\": [{\"statement\": \"CREATE INDEX IF NOT EXISTS FOR (n:Node) ON (n.differenceType)\"}]}";
    std::string indexAstOrigin = "{\"statements\": [{\"statement\": \"CREATE INDEX IF NOT EXISTS FOR (n:Node) ON (n.astOrigin)\"}]}";
    
    if (sendRequest(query) && sendRequest(indexDifferenceType) && sendRequest(indexAstOrigin)) {
        std::cout << "Indices created successfully." << std::endl;
    } else {
        throw std::runtime_error("Failed to create indices in the Neo4j database.");
    }
}

/*
Description:
    Finalizes the batch by executing any remaining nodes and relationships
*/
void Neo4jDatabaseWrapper::finalize() {
    // execute any remaining bathes
    if ((!nodeBatch.empty() || !relationshipBatch.empty()) && !executeBatch()) {
        std::cerr << "Failed to execute remaining batch." << std::endl;
        nodeBatch.clear();
        relationshipBatch.clear();
    }
}