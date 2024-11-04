#include "../headers/loggers/logger_creator.h"
#include "../headers/loggers/database_logger.h"

class ConsoleLoggerCreator : public LoggerCreator {
public:
    std::unique_ptr<TreeComparerLogger> createLogger() override {
        return std::make_unique<DatabaseLogger>();
    }
};