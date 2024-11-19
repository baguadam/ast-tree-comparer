#ifndef _IDATABASE_WRAPPER_H_

#define _IDATABASE_WRAPPER_H_
#include "node.h"

class IDatabaseWrapper {
public:
    virtual ~IDatabaseWrapper() = default;
    virtual void addNodeToBatch(const Node&, bool, const std::string&, const std::string&) = 0;
    virtual void addRelationshipToBatch(const Node&, const Node&) = 0;
    virtual void finalize() = 0;
    virtual void createIndices() = 0;
    virtual void clearDatabase() = 0;
};

#endif