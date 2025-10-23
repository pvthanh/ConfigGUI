// SPDX-License-Identifier: MIT
// Crash Handler - Install handlers for unexpected errors and Qt messages

#ifndef CONFIGGUI_UTILS_CRASH_HANDLER_H
#define CONFIGGUI_UTILS_CRASH_HANDLER_H

#include <string>

namespace configgui {
namespace utils {

// Install POSIX signal handlers (SIGSEGV, SIGABRT, SIGFPE, SIGILL) that write a
// minimal backtrace to a crash log file (async-signal-safe). If log_path is empty,
// a default path like "/tmp/configgui_crash.log" is used.
void installCrashHandlers(const std::string& log_path = "");

// Redirect Qt messages (qDebug/qWarning/qCritical/qFatal) to our logger.
void installQtMessageHandler();

// Install std::terminate handler to log unhandled exceptions on threads.
void installTerminateHandler();

} // namespace utils
} // namespace configgui

#endif // CONFIGGUI_UTILS_CRASH_HANDLER_H
