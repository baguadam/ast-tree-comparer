#include "./headers/neo4j_database_wrapper.h"
#include <iostream>
#include <sstream>
#include "./headers/utils.h"
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

    // create indices for enhancedKey for faster lookup
    createIndices();
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
void Neo4jDatabaseWrapper::addNodeToBatch(const Node& node, bool isHighLevel) {
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
               << "}";

    nodeBatch.push_back(nodeStream.str());

    // if the batch size exceeds 5000, execute the batch
    if (nodeBatch.size() >= 5000) {
        executeBatch();
    }
}

/*
Description:
    Adds a relationship to the batch for creation in the database, if the batch size exceeds 5000, the batch is executed, also \ characters are properly escaped
    for valid JSON syntax
*/
void Neo4jDatabaseWrapper::addRelationshipToBatch(const Node& parent, const Node& child) {
    // Properly escape backslashes in the parent and child keys
    std::string escapedParentKey = parent.enhancedKey;
    std::string escapedChildKey = child.enhancedKey;

    std::string::size_type pos = 0;
    while ((pos = escapedParentKey.find("\\", pos)) != std::string::npos) {
        escapedParentKey.replace(pos, 1, "\\\\");
        pos += 2;
    }
    pos = 0;
    while ((pos = escapedChildKey.find("\\", pos)) != std::string::npos) {
        escapedChildKey.replace(pos, 1, "\\\\");
        pos += 2;
    }

    std::ostringstream relStream;
    relStream << "{\"parentKey\": \"" << escapedParentKey
              << "\", \"parentOrder\": " << parent.topologicalOrder
              << ", \"childKey\": \"" << escapedChildKey
              << "\", \"childOrder\": " << child.topologicalOrder
              << "}";

    relationshipBatch.push_back(relStream.str());

    // if the batch size exceeds 5000, execute the batch
    if (relationshipBatch.size() >= 5000) {
        executeBatch();
    }
}

/*
Description:
    Executes the batch of nodes and relationships in the database
*/
void Neo4jDatabaseWrapper::executeBatch() {
    if (nodeBatch.empty() && relationshipBatch.empty()) {
        return;  // nothing to execute
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
                    << "topologicalOrder: node.topologicalOrder, isHighLevel: node.isHighLevel})\", "
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

    sendRequest(queryStream.str());

    // clear the buffers
    nodeBatch.clear();
    relationshipBatch.clear();
}

/*
Description:
    Sends a request to the Neo4j database with the given query JSON
*/
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

/*
Description:
    Clears the database by deleting all nodes and relationships
*/
void Neo4jDatabaseWrapper::clearDatabase() {
    std::string query = "{\"statements\": [{\"statement\": \"MATCH (n) DETACH DELETE n\"}]}";
    sendRequest(query);
}

/*
Description:
    Creates the necessary indices for the database, in our case so far it's enough to create an index for the enhancedKey property of the Node
*/
void Neo4jDatabaseWrapper::createIndices() {
    std::string query = "{\"statements\": [{\"statement\": \"CREATE INDEX IF NOT EXISTS FOR (n:Node) ON (n.enhancedKey)\"}]}";
    sendRequest(query);
}

/*
Description:
    Finalizes the batch by executing any remaining nodes and relationships
*/
void Neo4jDatabaseWrapper::finalize() {
    // execute any remaining bathec
    if (!nodeBatch.empty() || !relationshipBatch.empty()) {
        executeBatch();
    }
}