// SPDX-License-Identifier: MIT
// Logger - Implementation

#include "logger.h"

namespace configgui {
namespace utils {

Logger::Level Logger::min_level_ = Logger::Level::Debug;
std::ofstream Logger::log_file_;
bool Logger::console_output_ = true;

void Logger::init(const std::string& file_path, bool console_output, Level min_level)
{
    min_level_ = min_level;
    console_output_ = console_output;

    if (!file_path.empty())
    {
        log_file_.open(file_path, std::ios::app);
    }
}

void Logger::debug(const std::string& message)
{
    if (min_level_ <= Level::Debug)
    {
        writeLog(Level::Debug, message);
    }
}

void Logger::info(const std::string& message)
{
    if (min_level_ <= Level::Info)
    {
        writeLog(Level::Info, message);
    }
}

void Logger::warn(const std::string& message)
{
    if (min_level_ <= Level::Warn)
    {
        writeLog(Level::Warn, message);
    }
}

void Logger::error(const std::string& message)
{
    if (min_level_ <= Level::Error)
    {
        writeLog(Level::Error, message);
    }
}

void Logger::close()
{
    if (log_file_.is_open())
    {
        log_file_.close();
    }
}

std::string Logger::getLevelName(Level level)
{
    switch (level)
    {
        case Level::Debug:
            return "DEBUG";
        case Level::Info:
            return "INFO";
        case Level::Warn:
            return "WARN";
        case Level::Error:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

std::string Logger::getTimestamp()
{
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::writeLog(Level level, const std::string& message)
{
    std::string log_line = "[" + getTimestamp() + "] [" + getLevelName(level) + "] " + message;

    if (console_output_)
    {
        std::cout << log_line << std::endl;
    }

    if (log_file_.is_open())
    {
        log_file_ << log_line << std::endl;
        log_file_.flush();
    }
}

} // namespace utils
} // namespace configgui
