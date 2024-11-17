#ifndef _NODE_UTILITIES_H_
#define _NODE_UTILITIES_H_

#include "node.h"

inline bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.type == rhs.type &&
           lhs.kind == rhs.kind &&
           lhs.usr == rhs.usr &&
           lhs.path == rhs.path &&
           lhs.lineNumber == rhs.lineNumber &&
           lhs.columnNumber == rhs.columnNumber &&
           lhs.enhancedKey == rhs.enhancedKey;
}

#endif
