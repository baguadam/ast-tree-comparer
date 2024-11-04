#ifndef _DATABASE_LOGGER_CREATOR_H_

#define _DATABASE_LOGGER_CREATOR_H_

#include "logger_creator.h"
#include "../loggers/database_logger.h"

class DatabaseLoggerCreator : public LoggerCreator { 
public:
    std::unique_ptr<TreeComparerLogger> createLogger() override {
        return std::make_unique<DatabaseLogger>();
    }
};

#endif