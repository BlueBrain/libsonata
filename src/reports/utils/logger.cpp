#include <memory>
#include "logger.hpp"

/**
 * \brief Logger implementation based on spdlog
 */
struct Logger {
    logger_t logger;
    Logger(const std::string& name, std::string pattern) {
        logger = spdlog::stdout_color_mt(name);
        logger->set_pattern(std::move(pattern));
    }
};

Logger custom_logger("REPORTS", "[%n] [%^%l%$] :: %v");
logger_t logger = custom_logger.logger;
