// SPDX-License-Identifier: MIT
// Schema-Driven Configuration GUI - Main Entry Point
// Copyright (c) 2025

#include <QApplication>
#include "ui/main_window.h"
#include "utils/logger.h"
#include "utils/crash_handler.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    QApplication::setApplicationName("ConfigGUI");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setApplicationDisplayName("Schema-Driven Configuration GUI");

    // Global UI stylesheet: improve dropdown (QComboBox) item hover/selection visibility
    // Applies to ALL combo boxes across the application.
    // - Hover: light blue background, black text
    // - Selected: slightly darker blue, black text
    // This avoids cases where the hover state appears white and reduces readability.
    const QString kGlobalStyle = QString(
        "QComboBox QAbstractItemView {"
        "  background: #ffffff;"
        "  color: #000000;"
        "  selection-background-color: #cce4ff;"
        "  selection-color: #000000;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "  background: #e6f2ff;"
        "  color: #000000;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "  background: #cce4ff;"
        "  color: #000000;"
        "}"
    );
    app.setStyleSheet(kGlobalStyle);

    // Initialize logging and crash handlers early
    configgui::utils::Logger::init("configgui.log", true, configgui::utils::Logger::Level::Debug);
    configgui::utils::installCrashHandlers("/tmp/configgui_crash.log");
    configgui::utils::installQtMessageHandler();
    configgui::utils::installTerminateHandler();

    try
    {
        // Create and show main window
        configgui::ui::MainWindow window;
        window.show();
        return app.exec();
    }
    catch (const std::exception& e)
    {
        configgui::utils::Logger::error(std::string("Unhandled exception: ") + e.what());
    }
    catch (...)
    {
        configgui::utils::Logger::error("Unhandled non-standard exception in main");
    }

    return 1;
}
