#include "./headers/neo4j_database_wrapper.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <iomanip>

Neo4jDatabaseWrapper::Neo4jDatabaseWrapper(const std::string& uri, const std::string& username, const std::string& password)
    : dbUri(uri + "/db/neo4j/tx/commit") {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    // create authentication header (Basic Auth)
    std::ostringstream authStream;
    authStream << username << ":" << password;
    std::string authString = authStream.str();
    authHeader = "Authorization: Basic " + std::string(curl_easy_escape(curl, authString.c_str(), authString.length()));

    // creates the indices for enhancedKey for faster lookup
    createIndices();
}

Neo4jDatabaseWrapper::~Neo4jDatabaseWrapper() {
    finalize(); // for now it is called from the destructor, later will be called in the TreeComparer

    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void Neo4jDatabaseWrapper::addNodeToBatch(const Node& node, bool isHighLevel) {
    std::ostringstream nodeStream;
    nodeStream << "{\"enhancedKey\": \"" << node.enhancedKey
               << "\", \"topologicalOrder\": " << node.topologicalOrder
               << ", \"type\": \"" << node.type
               << "\", \"kind\": \"" << node.kind
               << "\", \"usr\": \"" << node.usr
               << "\", \"path\": \"" << node.path
               << "\", \"lineNumber\": " << node.lineNumber
               << ", \"columnNumber\": " << node.columnNumber
               << ", \"isHighLevel\": " << (isHighLevel ? "true" : "false")
               << "}";

    nodeBatch.push_back(nodeStream.str());

    // if the batch size exceeds 5000, execute the batch
    if (nodeBatch.size() >= 5000) {
        executeBatch();
    }
}

void Neo4jDatabaseWrapper::addRelationshipToBatch(const Node& parent, const Node& child) {
    std::ostringstream relStream;
    relStream << "{\"parentKey\": \"" << parent.enhancedKey
              << "\", \"parentOrder\": " << parent.topologicalOrder
              << ", \"childKey\": \"" << child.enhancedKey
              << "\", \"childOrder\": " << child.topologicalOrder
              << "}";

    relationshipBatch.push_back(relStream.str());

    // same with the batch size
    if (relationshipBatch.size() >= 5000) {
        executeBatch();
    }
}

void Neo4jDatabaseWrapper::executeBatch() {
    if (nodeBatch.empty() && relationshipBatch.empty()) {
        return;  // Nothing to execute
    }

    std::ostringstream queryStream;
    queryStream << "{\"statements\": [{\"statement\": \"";

    // UNWIND for bulk node creation
    if (!nodeBatch.empty()) {
        queryStream << "UNWIND $nodes AS node "
                    << "CREATE (n:Node {enhancedKey: node.enhancedKey, type: node.type, kind: node.kind, usr: node.usr, path: node.path, lineNumber: node.lineNumber, columnNumber: node.columnNumber, topologicalOrder: node.topologicalOrder, isHighLevel: node.isHighLevel}) ";
    }

    // UNWIND for bulk relationship creation
    if (!relationshipBatch.empty()) {
        queryStream << "UNWIND $relationships AS rel "
                    << "MATCH (a:Node {enhancedKey: rel.parentKey}), (b:Node {enhancedKey: rel.childKey}) "
                    << "CREATE (a)-[:HAS_CHILD]->(b) ";
    }

    queryStream << "\", \"parameters\": {\"nodes\": [";

    // JSON format for nodes
    for (size_t i = 0; i < nodeBatch.size(); ++i) {
        if (i > 0) queryStream << ",";
        queryStream << nodeBatch[i];
    }

    queryStream << "], \"relationships\": [";

    // JSON format for relationships
    for (size_t i = 0; i < relationshipBatch.size(); ++i) {
        if (i > 0) queryStream << ",";
        queryStream << relationshipBatch[i];
    }

    queryStream << "]}}]}";

    // send the request
    sendRequest(queryStream.str());

    // clear the buffers
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

    // RETRY MECHANISM for temporary network failures
    int retryCount = 0;
    const int maxRetries = 3;
    CURLcode res;
    do {
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL request failed: " << curl_easy_strerror(res) << " - Retrying (" << retryCount + 1 << "/" << maxRetries << ")" << std::endl;
            retryCount++;
        } else {
            break; // request succeeded
        }
    } while (retryCount < maxRetries);

    if (res != CURLE_OK) {
        std::cerr << "CURL request ultimately failed after retries: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
}

void Neo4jDatabaseWrapper::createIndices() {
    std::string query = "{\"statements\": [{\"statement\": \"CREATE INDEX IF NOT EXISTS FOR (n:Node) ON (n.enhancedKey)\"}]}";
    sendRequest(query);
}

void Neo4jDatabaseWrapper::finalize() {
    // execute any remaining bathec
    if (!nodeBatch.empty() || !relationshipBatch.empty()) {
        executeBatch();
    }
}