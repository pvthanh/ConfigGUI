// SPDX-License-Identifier: MIT
// MainWindow - Main application window

#pragma once

#include <QMainWindow>
#include <memory>
#include <QString>

class QMenu;
class QToolBar;
class QStatusBar;
class QWidget;
class QLabel;
class QScrollArea;

namespace configgui {
namespace ui {
class FormGenerator;
}

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
    void onFileOpenSchema();
    void onFileOpenConfiguration();
    void onFileSave();
    void onFileExit();
    void onHelpAbout();

private:
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void connectSignals();
    void loadSchema(const QString& file_path);
    void loadConfiguration(const QString& file_path);

    // UI Components
    QMenu* file_menu_;
    QMenu* help_menu_;
    QToolBar* toolbar_;
    QWidget* central_widget_;
    QLabel* status_label_;
    FormGenerator* form_generator_;
    QScrollArea* scroll_area_;

    // State tracking
    QString current_schema_file_;
    QString current_config_file_;
};

} // namespace ui
} // namespace configgui
