// SPDX-License-Identifier: MIT
// Crash Handler - Implementation

#include "crash_handler.h"
#include "logger.h"

#include <csignal>
#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <QMessageBox>
#include <QApplication>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

#if defined(__linux__)
#include <execinfo.h>
#endif

#include <QtGlobal>
#include <QString>
#include <exception>

namespace configgui {
namespace utils {

namespace {
int g_log_fd = -1;

// Windows-specific crash handling
#ifdef _WIN32
void writeWindowsCrashInfo(const char* error_msg)
{
    // Create crash report file
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream filename;
    
    // Use safe localtime alternative on Windows for filename too
#ifdef _WIN32
    struct tm timeinfo;
    if (localtime_s(&timeinfo, &time_t) == 0) {
        filename << "configgui_crash_" << std::put_time(&timeinfo, "%Y%m%d_%H%M%S") << ".log";
    } else {
        filename << "configgui_crash_unknown_time.log";
    }
#else
    filename << "configgui_crash_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << ".log";
#endif
    
    std::ofstream crash_file(filename.str());
    if (crash_file.is_open()) {
        crash_file << "===== ConfigGUI Crash Report =====\n";
        
        // Use safe localtime alternative on Windows
#ifdef _WIN32
        struct tm timeinfo2;
        if (localtime_s(&timeinfo2, &time_t) == 0) {
            crash_file << "Time: " << std::put_time(&timeinfo2, "%Y-%m-%d %H:%M:%S") << "\n";
        }
#else
        crash_file << "Time: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";
#endif
        
        crash_file << "Error: " << error_msg << "\n";
        crash_file << "====================================\n";
        crash_file.close();
    }
    
    // Show user-friendly message
    if (QApplication::instance()) {
        QString message = QString("ConfigGUI has encountered an error and needs to close.\n\n"
                                "Error: %1\n\n"
                                "A crash report has been saved to: %2\n\n"
                                "Please restart the application.")
                         .arg(error_msg)
                         .arg(filename.str().c_str());
        
        QMessageBox::critical(nullptr, "ConfigGUI Error", message);
    }
}

LONG WINAPI windowsExceptionHandler(EXCEPTION_POINTERS* exceptionInfo)
{
    const char* error_msg = "Unknown Windows exception";
    
    switch (exceptionInfo->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
            error_msg = "Memory access violation";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            error_msg = "Stack overflow";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            error_msg = "Division by zero";
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            error_msg = "Illegal instruction";
            break;
        default:
            error_msg = "Unhandled Windows exception";
            break;
    }
    
    writeWindowsCrashInfo(error_msg);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

// Write a string using async-signal-safe write()
void writeStr(int fd, const char* s)
{
#ifndef _WIN32
    if (fd < 0 || s == nullptr) { return; }
    const size_t len = std::strlen(s);
    if (len > 0) {
        // best-effort
        (void)::write(fd, s, len);
        (void)::write(fd, "\n", 1);
    }
#else
    (void)fd;
    (void)s;
#endif
}

void signalHandler(int sig)
{
    writeStr(g_log_fd, "===== ConfigGUI Crash Detected =====");
    switch (sig) {
        case SIGSEGV: writeStr(g_log_fd, "Signal: SIGSEGV (segmentation fault)"); break;
        case SIGABRT: writeStr(g_log_fd, "Signal: SIGABRT (abort)"); break;
        case SIGFPE:  writeStr(g_log_fd, "Signal: SIGFPE (floating-point exception)"); break;
        case SIGILL:  writeStr(g_log_fd, "Signal: SIGILL (illegal instruction)"); break;
        default:      writeStr(g_log_fd, "Signal: Unknown"); break;
    }

#if defined(__linux__)
    void* buffer[64];
    int nptrs = ::backtrace(buffer, 64);
    if (nptrs > 0) {
        writeStr(g_log_fd, "Backtrace:");
        ::backtrace_symbols_fd(buffer, nptrs, g_log_fd);
    }
#endif

    // Flush and terminate immediately
    ::_exit(128 + sig);
}

QString qtMsgTypeToString(QtMsgType type)
{
    switch (type) {
        case QtDebugMsg:    return QStringLiteral("DEBUG");
        case QtInfoMsg:     return QStringLiteral("INFO");
        case QtWarningMsg:  return QStringLiteral("WARN");
        case QtCriticalMsg: return QStringLiteral("ERROR");
        case QtFatalMsg:    return QStringLiteral("FATAL");
    }
    return QStringLiteral("UNKNOWN");
}

void qtMessageRedirect(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    Q_UNUSED(ctx);
    const std::string line = (qtMsgTypeToString(type) + QStringLiteral(": ") + msg).toStdString();
    switch (type) {
        case QtDebugMsg:    Logger::debug(line); break;
        case QtInfoMsg:     Logger::info(line); break;
        case QtWarningMsg:  Logger::warn(line); break;
        case QtCriticalMsg: Logger::error(line); break;
        case QtFatalMsg:    Logger::error(line); break;
    }
}

} // namespace

void installCrashHandlers(const std::string& log_path)
{
#ifdef _WIN32
    // Install Windows structured exception handler
    SetUnhandledExceptionFilter(windowsExceptionHandler);
    
    (void)log_path;
    g_log_fd = 2; // stderr on Windows
#else
    // Open crash log file
    std::string path = log_path.empty() ? std::string("/tmp/configgui_crash.log") : log_path;
    g_log_fd = ::open(path.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (g_log_fd < 0) {
        // Fallback to stderr
        g_log_fd = STDERR_FILENO;
    }
#endif

    // Install minimal signal handlers (cross-platform)
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGFPE,  signalHandler);
    std::signal(SIGILL,  signalHandler);
}

void installQtMessageHandler()
{
    qInstallMessageHandler(qtMessageRedirect);
}

void installTerminateHandler()
{
    std::set_terminate([](){
        try {
            // Try to rethrow and capture the what() if possible
            auto eptr = std::current_exception();
            std::string error_msg;
            
            if (eptr) {
                try { 
                    std::rethrow_exception(eptr); 
                }
                catch (const std::exception& e) {
                    error_msg = std::string("Unhandled exception: ") + e.what();
                    Logger::error(error_msg);
                }
                catch (...) {
                    error_msg = "Unknown unhandled exception";
                    Logger::error(error_msg);
                }
            } else {
                error_msg = "std::terminate called without active exception";
                Logger::error(error_msg);
            }

#ifdef _WIN32
            // Show Windows message box for unhandled exceptions
            writeWindowsCrashInfo(error_msg.c_str());
#endif
        } catch (...) {
            // Swallow any errors during logging
        }
        ::_exit(1);
    });
}

} // namespace utils
} // namespace configgui
