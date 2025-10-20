// SPDX-License-Identifier: MIT
// MainWindow - Main application window

#pragma once

#include <QMainWindow>
#include <memory>

class QMenu;
class QToolBar;
class QStatusBar;
class QWidget;

namespace configgui {
namespace core {
class ConfigurationData;
class JSONSchema;
}

namespace ui {

/// @brief Main application window
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // Non-copyable
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

private slots:
    void onFileOpen();
    void onFileSave();
    void onFileExit();
    void onHelpAbout();

private:
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void connectSignals();

    // UI Components
    QMenu* file_menu_;
    QMenu* help_menu_;
    QToolBar* toolbar_;
    QWidget* central_widget_;
};

} // namespace ui
} // namespace configgui
