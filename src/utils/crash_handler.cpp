// SPDX-License-Identifier: MIT
// Crash Handler - Implementation

#include "crash_handler.h"
#include "logger.h"

#include <csignal>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
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

// Write a string using async-signal-safe write()
void writeStr(int fd, const char* s)
{
    if (fd < 0 || s == nullptr) { return; }
    const size_t len = std::strlen(s);
    if (len > 0) {
        // best-effort
        (void)::write(fd, s, len);
        (void)::write(fd, "\n", 1);
    }
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
    // Open crash log file
    std::string path = log_path.empty() ? std::string("/tmp/configgui_crash.log") : log_path;
    g_log_fd = ::open(path.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (g_log_fd < 0) {
        // Fallback to stderr
        g_log_fd = STDERR_FILENO;
    }

    // Install minimal handlers
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
            if (eptr) {
                try { std::rethrow_exception(eptr); }
                catch (const std::exception& e) {
                    Logger::error(std::string("std::terminate due to unhandled exception: ") + e.what());
                }
                catch (...) {
                    Logger::error("std::terminate due to unknown unhandled exception");
                }
            } else {
                Logger::error("std::terminate called without active exception");
            }
        } catch (...) {
            // Swallow any errors during logging
        }
        ::_exit(1);
    });
}

} // namespace utils
} // namespace configgui
