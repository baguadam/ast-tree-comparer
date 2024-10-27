#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include "node.h"

class Utils {
public:
    static std::string getKey(Node* node, bool isDeclaration);
};

#endif 