#pragma once
/**
 * \file
 * \brief Implement logger based on spdlog library
 */

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using logger_t = std::shared_ptr<spdlog::logger>;
extern logger_t logger;
