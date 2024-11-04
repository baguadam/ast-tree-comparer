#ifndef _CONSOLE_LOGGER_CREATOR_H_

#define _CONSOLE_LOGGER_CREATOR_H_

#include "logger_creator.h"
#include "../loggers/console_logger.h"

class ConsoleLoggerCreator : public LoggerCreator { 
public:
    std::unique_ptr<TreeComparerLogger> createLogger() override {
        return std::make_unique<ConsoleLogger>();
    }
};

#endif