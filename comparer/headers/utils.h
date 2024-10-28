#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include "node.h"

class Utils {
public:
    static std::string getKey(const Node* node, bool isDeclaration);

private:
    static const Node* findDeclarationParent(const Node* node);
};

#endif 