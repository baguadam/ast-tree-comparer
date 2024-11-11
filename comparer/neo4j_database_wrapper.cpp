#include "./headers/neo4j_database_wrapper.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <iomanip>

Neo4jDatabaseWrapper::Neo4jDatabaseWrapper(const std::string& uri, const std::string& username, const std::string& password)
    : dbUri(uri + "/db/neo4j/tx/commit") {
    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    // Create authentication header (Basic Auth)
    std::ostringstream authStream;
    authStream << username << ":" << password;
    std::string authString = authStream.str();
    authHeader = "Authorization: Basic " + std::string(curl_easy_escape(curl, authString.c_str(), authString.length()));
}

Neo4jDatabaseWrapper::~Neo4jDatabaseWrapper() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void Neo4jDatabaseWrapper::addNodeToBatch(const Node& node) {
    std::ostringstream nodeStream;
    nodeStream << "CREATE (n:Node {type: '" << node.type
               << "', kind: '" << node.kind << "', usr: '" << node.usr
               << "', path: '" << node.path << "', lineNumber: " << node.lineNumber
               << ", columnNumber: " << node.columnNumber
               << ", topologicalOrder: " << node.topologicalOrder
               << ", enhancedKey: '" << node.enhancedKey << "'})";

    nodeBatch.push_back(nodeStream.str());
}

void Neo4jDatabaseWrapper::addRelationshipToBatch(const Node& parent, const Node& child) {
    std::ostringstream relStream;
    relStream << "MATCH (a:Node {enhancedKey: '" << parent.enhancedKey
              << "'}), (b:Node {enhancedKey: '" << child.enhancedKey
              << "'}) CREATE (a)-[:HAS_CHILD]->(b)";

    relationshipBatch.push_back(relStream.str());
}

void Neo4jDatabaseWrapper::executeBatch() {
    if (nodeBatch.empty() && relationshipBatch.empty()) {
        return;  // Nothing to execute
    }

    std::ostringstream queryStream;
    queryStream << "{\"statements\": [{\"statement\": \"";

    // Combine all CREATE statements into a single Cypher query
    for (const auto& nodeQuery : nodeBatch) {
        queryStream << nodeQuery << " ";
    }
    for (const auto& relQuery : relationshipBatch) {
        queryStream << relQuery << " ";
    }

    queryStream << "\"}]}";

    // Send the batch request
    sendRequest(queryStream.str());

    // Clear the batches after executing
    nodeBatch.clear();
    relationshipBatch.clear();
}

void Neo4jDatabaseWrapper::sendRequest(const std::string& queryJson) {
    if (!curl) {
        std::cerr << "CURL initialization failed" << std::endl;
        return;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, dbUri.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, queryJson.c_str());

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
}