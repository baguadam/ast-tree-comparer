#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <vector>
#include "node.h"
#include "enums.h"

class Utils {
public:
    static std::string getStmtKey(const Node*, const std::string&);
    static std::string getEnhancedDeclKey(const Node*);
    static size_t getFingerPrint(const Node*);
    static const Node* findDeclarationParent(const Node*);

    static std::string astIdToString(const ASTId);
    static std::string nodeTypeToString(const NodeType);
    static NodeType stringToNodeType(const std::string&);
    static std::string differenceTypeToString(const DifferenceType);

    static void printSeparators();
    static void printNodeDetails(const Node*, std::string);
    static std::vector<std::string> splitString(const std::string&, const char delimiter = '\t');
    static void ltrim(std::string&);

    static std::string base64Encode(const std::string&);
    static std::string escapeString(const std::string&);
private:
    static size_t hashString(const std::string&);
};

#endif 