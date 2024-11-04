#ifndef _LOGGER_CREATOR_H
#define _LOGGER_CREATOR_H

#include "../loggers/tree_comparer_logger.h"
#include <memory>

class LoggerCreator {
public:
    virtual ~LoggerCreator() = default;
    virtual std::unique_ptr<TreeComparerLogger> createLogger() = 0;
};

#endif
