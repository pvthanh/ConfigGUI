// SPDX-License-Identifier: MIT
// Logger - Logging Utility

#ifndef CONFIGGUI_UTILS_LOGGER_H
#define CONFIGGUI_UTILS_LOGGER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <iomanip>

namespace configgui {
namespace utils {

/**
 * @class Logger
 * @brief Simple logging utility
 *
 * Provides logging at different levels: DEBUG, INFO, WARN, ERROR.
 * Can log to console and/or file.
 */
class Logger
{
public:
    enum class Level {
        Debug = 0,
        Info = 1,
        Warn = 2,
        Error = 3
    };

    /**
     * @brief Initialize logger
     * @param file_path Optional log file path
     * @param console_output Log to console
     * @param min_level Minimum level to log
     */
    static void init(const std::string& file_path = "", bool console_output = true,
                     Level min_level = Level::Debug);

    /**
     * @brief Log debug message
     * @param message Message to log
     */
    static void debug(const std::string& message);

    /**
     * @brief Log info message
     * @param message Message to log
     */
    static void info(const std::string& message);

    /**
     * @brief Log warning message
     * @param message Message to log
     */
    static void warn(const std::string& message);

    /**
     * @brief Log error message
     * @param message Message to log
     */
    static void error(const std::string& message);

    /**
     * @brief Close logger
     */
    static void close();

private:
    static Level min_level_;
    static std::ofstream log_file_;
    static bool console_output_;

    /**
     * @brief Get level name
     * @param level Log level
     * @return Level name as string
     */
    static std::string getLevelName(Level level);

    /**
     * @brief Get current timestamp
     * @return Timestamp string
     */
    static std::string getTimestamp();

    /**
     * @brief Write log entry
     * @param level Log level
     * @param message Message to log
     */
    static void writeLog(Level level, const std::string& message);
};

} // namespace utils
} // namespace configgui

#endif // CONFIGGUI_UTILS_LOGGER_H
