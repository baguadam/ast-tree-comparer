#ifndef _ENUM_H_

#define _ENUM_H_

enum ASTId {
    FIRST_AST,
    SECOND_AST
};

enum NodeType {
    DECLARATION,
    STATEMENT,
    UNKNOWN
};

enum DifferenceType {
    ONLY_IN_FIRST_AST,
    ONLY_IN_SECOND_AST,
    DIFFERENT_PARENT,
    DIFFERENT_SOURCE_LOCATIONS
};

#endif