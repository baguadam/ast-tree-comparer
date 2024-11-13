#include "./headers/utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

/*
Description:
    Generates a unique key for a statement node based on its parent
*/
std::string Utils::getStmtKey(const Node* node, const std::string& declarationParentKey) {
    std::string statementKey = node->kind + "|" + node->usr + "|" + node->path + "|" + std::to_string(node->lineNumber) + ":" + std::to_string(node->columnNumber);
    return declarationParentKey + "|" + statementKey;  // concatenate parent and current node's key
}

/*
Description:
    Generates an enhanced key of the node from the path to the root
*/
std::string Utils::getEnhancedDeclKey(const Node* node) {
    std::string key = node->kind + "|" + node->usr + "|" + node->path + "|";
    return key;
}

/*
Description:
    Generates a unique fingerprint for a node based on its information and children, each node is hashed separately and the 
    result is combined using XOR operation
*/
size_t Utils::getFingerPrint(const Node* node) {
    size_t hash = hashString(node->kind) ^ hashString(node->usr) ^
                  hashString(node->path) ^ std::hash<int>{}(node->lineNumber) ^
                  std::hash<int>{}(node->columnNumber) ^ std::hash<int>{}(node->topologicalOrder);

    for (const Node* child : node->children) {
        hash ^= getFingerPrint(child);
    }

    return hash;
}

/*
Description:
    Finds the first declaration parent of a given node (most cases it is for Statement nodes)
*/
const Node* Utils::findDeclarationParent(const Node* node) {
    const Node* parent = node->parent;
    while (parent && parent->type != DECLARATION) {
        parent = parent->parent;
    }
    return parent;
}

/*
Description:
    Prints a separator line to the console
*/
void Utils::printSeparators() {
    std::cout << "----------------------------------------\n";
}

/*
Description:
    Prints the details of a node to the console
*/
void Utils::printNodeDetails(const Node* node, std::string indent) {
    std::cout << indent << "Node details:\n";
    std::cout << indent << node->kind << " " << node->type << " " << node->usr << " " << node->path << " " << node->lineNumber << ":" << node->columnNumber << "\n";
    std::cout << indent << "*** Parent enhanced key: " << (node->parent ? node->parent->enhancedKey : "None") << "\n";
    
    printSeparators();
}

/*
Description:
    Converts an AST ID to a string
*/
std::string Utils::astIdToString(const ASTId ast) {
    switch (ast) {
        case FIRST_AST: return "FIRST AST";
        case SECOND_AST: return "SECOND AST";
        default: return "UNKNOWN AST";
    }
}

/*
Description:
    Converts a node type to a string
*/
std::string Utils::nodeTypeToString(const NodeType type) {
    switch (type) {
        case DECLARATION: return "Declaration";
        case STATEMENT: return "Statement";
        default: return "Unknown";
    }
}

/*
Description:
    Converts a string to a node type
*/
NodeType Utils::stringToNodeType(const std::string& type) {
    if (type == "Declaration") {
        return DECLARATION;
    } else if (type == "Statement") {
        return STATEMENT;
    } else {
        std::cerr << "Warning: Unknown node type: " << type << '\n';
        return UNKNOWN;
    }
}

/*
Description:
    Converts a difference type to a string
*/
std::string Utils::differenceTypeToString(const DifferenceType type) {
    switch (type) {
        case ONLY_IN_FIRST_AST: return "NODE EXISTS ONLY IN FIRST AST";
        case ONLY_IN_SECOND_AST: return "NODE EXISTS ONLY IN SECOND AST";
        case DIFFERENT_PARENT: return "NODES HAVE DIFFERENT PARENTS";
        case DIFFERENT_SOURCE_LOCATIONS: return "NODES HAVE DIFFERENT SOURCE LOCATIONS";
        default: return "UNKNOWN DIFFERENCE";
    }
}

/*
Description:
    Splits a line for the necessary columns based on the given deliminator (our case it is most likely '|')
*/
std::vector<std::string> Utils::splitString(const std::string& str, const char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }

    return tokens;
}

/*
Description:
    Trims the leading whitespace of a string
*/
void Utils::ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

/*
Description:
    Trims the trailing whitespace of a string
*/
void Utils::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

/*
Description:
    Hashes a string using the std::hash function
*/
size_t Utils::hashString(const std::string& str) {
    std::hash<std::string> hasher;
    return hasher(str);
}

/*
Description:
    Base64 encodes a given string, used for encoding the credentials for the Neo4j database
*/
std::string Utils::base64Encode(const std::string& in) {
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

/*
Description:
    Escapes the backslashes in a string for valid JSON syntax
*/
std::string Utils::escapeString(const std::string& str) {
    std::string escaped = str;
    std::string::size_type pos = 0;
    while ((pos = escaped.find("\\", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\\\");
        pos += 2;
    }
    return escaped;
}