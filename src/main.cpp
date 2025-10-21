// SPDX-License-Identifier: MIT
// Schema-Driven Configuration GUI - Main Entry Point
// Copyright (c) 2025

#include <QApplication>
#include "ui/main_window.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    QApplication::setApplicationName("ConfigGUI");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setApplicationDisplayName("Schema-Driven Configuration GUI");

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
