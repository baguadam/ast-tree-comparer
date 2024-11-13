#include "./headers/neo4j_database_wrapper.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <iomanip>

static std::string base64_encode(const std::string &in) {
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                            "abcdefghijklmnopqrstuvwxyz"
                                            "0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

Neo4jDatabaseWrapper::Neo4jDatabaseWrapper(const std::string& uri, const std::string& username, const std::string& password)
    : dbUri(uri + "/db/neo4j/tx/commit") {
    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    // Create authentication header (Basic Auth)
    std::string credentials = username + ":" + password;
    std::string encoded_credentials = base64_encode(credentials);
    authHeader = "Authorization: Basic " + encoded_credentials;

    // Create indices for enhancedKey for faster lookup
    createIndices();
}

Neo4jDatabaseWrapper::~Neo4jDatabaseWrapper() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void Neo4jDatabaseWrapper::addNodeToBatch(const Node& node, bool isHighLevel) {
    // Properly escape backslashes in the path and enhancedKey
    std::string escapedPath = node.path;
    std::string escapedEnhancedKey = node.enhancedKey;

    // Replace backslash with double backslash
    std::string::size_type pos = 0;
    while ((pos = escapedPath.find("\\", pos)) != std::string::npos) {
        escapedPath.replace(pos, 1, "\\\\");
        pos += 2;
    }
    pos = 0;
    while ((pos = escapedEnhancedKey.find("\\", pos)) != std::string::npos) {
        escapedEnhancedKey.replace(pos, 1, "\\\\");
        pos += 2;
    }

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

    // If the batch size exceeds 5000, execute the batch
    if (nodeBatch.size() >= 5000) {
        executeBatch();
    }
}

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

    // If the batch size exceeds 5000, execute the batch
    if (relationshipBatch.size() >= 5000) {
        executeBatch();
    }
}

void Neo4jDatabaseWrapper::executeBatch() {
    if (nodeBatch.empty() && relationshipBatch.empty()) {
        return;  // Nothing to execute
    }

    std::ostringstream queryStream;
    queryStream << "{\"statements\": [";

    bool hasNodes = !nodeBatch.empty();
    bool hasRelationships = !relationshipBatch.empty();

    // If there are nodes to be created, add the node creation query.
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

    // If there are relationships to be created, add the relationship creation query.
    if (hasRelationships) {
        if (hasNodes) {
            queryStream << ",";  // Add a comma to separate the two queries
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

    // Clear the buffers
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

void Neo4jDatabaseWrapper::clearDatabase() {
    std::string query = "{\"statements\": [{\"statement\": \"MATCH (n) DETACH DELETE n\"}]}";
    sendRequest(query);
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