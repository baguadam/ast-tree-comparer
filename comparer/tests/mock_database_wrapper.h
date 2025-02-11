#ifndef _MOCK_DATABASE_WRAPPER_H_

#define _MOCK_DATABASE_WRAPPER_H_

#include <gmock/gmock.h>
#include "../include/idatabase_wrapper.h"
#include "../include/node.h"

class MockDatabaseWrapper : public IDatabaseWrapper {
public:
    MOCK_METHOD(void, addNodeToBatch, (const Node&, bool, const std::string&, const std::string&), (override));
    MOCK_METHOD(void, addRelationshipToBatch, (const Node&, const Node&), (override));
    MOCK_METHOD(void, createIndices, (), (override));
    MOCK_METHOD(void, finalize, (), (override));
    MOCK_METHOD(void, clearDatabase, (), (override));
};

#endif