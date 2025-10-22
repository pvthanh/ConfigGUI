// SPDX-License-Identifier: MIT
// MainWindow - Implementation

#include "main_window.h"
#include "form_generator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QScrollArea>
#include <QScrollBar>
#include <nlohmann/json.hpp>

namespace configgui {
namespace ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , file_menu_(nullptr)
    , help_menu_(nullptr)
    , toolbar_(nullptr)
    , central_widget_(nullptr)
    , status_label_(nullptr)
    , form_generator_(nullptr)
    , scroll_area_(nullptr)
    , current_schema_file_("")
    , current_config_file_("")
{
    setWindowTitle("ConfigGUI - Schema-Driven Configuration Manager");
    setGeometry(100, 100, 800, 600);

    setupUI();
    createMenuBar();
    createToolBar();
    connectSignals();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    // Create central widget with layout
    central_widget_ = new QWidget(this);
    auto* layout = new QVBoxLayout(central_widget_);
    layout->setContentsMargins(0, 0, 0, 0);

    // Create welcome label
    auto* welcome_label = new QLabel("Welcome to ConfigGUI\n\nLoad a schema to begin", central_widget_);
    welcome_label->setAlignment(Qt::AlignCenter);
    welcome_label->setObjectName("welcomeLabel");
    layout->addWidget(welcome_label);

    // Create form generator (hidden initially)
    form_generator_ = new FormGenerator(central_widget_);
    form_generator_->setObjectName("formGenerator");
    form_generator_->hide();

    // Create scroll area for form
    scroll_area_ = new QScrollArea(central_widget_);
    scroll_area_->setWidget(form_generator_);
    scroll_area_->setWidgetResizable(true);
    scroll_area_->hide();
    layout->addWidget(scroll_area_);

    // Status label
    status_label_ = new QLabel("Ready", central_widget_);
    status_label_->setAlignment(Qt::AlignLeft);
    layout->addWidget(status_label_);

    setCentralWidget(central_widget_);
}

void MainWindow::createMenuBar()
{
    // File Menu
    file_menu_ = menuBar()->addMenu(tr("&File"));

    auto* open_schema_action = file_menu_->addAction(tr("&Load Schema"));
    connect(open_schema_action, &QAction::triggered, this, &MainWindow::onFileOpenSchema);

    auto* open_config_action = file_menu_->addAction(tr("&Load Configuration"));
    connect(open_config_action, &QAction::triggered, this, &MainWindow::onFileOpenConfiguration);

    auto* save_action = file_menu_->addAction(tr("&Save Configuration"));
    save_action->setShortcut(QKeySequence::Save);
    connect(save_action, &QAction::triggered, this, &MainWindow::onFileSave);

    file_menu_->addSeparator();

    auto* exit_action = file_menu_->addAction(tr("E&xit"));
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &MainWindow::onFileExit);

    // Help Menu
    help_menu_ = menuBar()->addMenu(tr("&Help"));

    auto* about_action = help_menu_->addAction(tr("&About"));
    connect(about_action, &QAction::triggered, this, &MainWindow::onHelpAbout);
}

void MainWindow::createToolBar()
{
    toolbar_ = addToolBar(tr("Main Toolbar"));
    toolbar_->setObjectName("MainToolbar");

    auto* open_schema_action = toolbar_->addAction(tr("Load Schema"));
    connect(open_schema_action, &QAction::triggered, this, &MainWindow::onFileOpenSchema);

    auto* open_config_action = toolbar_->addAction(tr("Load Configuration"));
    connect(open_config_action, &QAction::triggered, this, &MainWindow::onFileOpenConfiguration);

    auto* save_action = toolbar_->addAction(tr("Save"));
    connect(save_action, &QAction::triggered, this, &MainWindow::onFileSave);
}

void MainWindow::connectSignals()
{
    // Additional signal connections can be added here
}

void MainWindow::onFileOpen()
{
    // This function is deprecated - use onFileOpenSchema or onFileOpenConfiguration
}

void MainWindow::onFileOpenSchema()
{
    try
    {
        QFileDialog dialog(this);
        dialog.setWindowTitle(tr("Load Schema File"));
        dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        dialog.setNameFilter(tr("JSON Schema Files (*.json);;All Files (*)"));
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setOptions(QFileDialog::DontUseNativeDialog);  // Use Qt file dialog instead of native
        
        if (dialog.exec() == QDialog::Accepted)
        {
            QStringList selected_files = dialog.selectedFiles();
            if (!selected_files.isEmpty())
            {
                loadSchema(selected_files.at(0));
            }
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("File dialog error: %1").arg(QString::fromStdString(e.what())));
    }
}

void MainWindow::onFileOpenConfiguration()
{
    try
    {
        QFileDialog dialog(this);
        dialog.setWindowTitle(tr("Load Configuration File"));
        dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        dialog.setNameFilter(tr("JSON Files (*.json);;YAML Files (*.yaml *.yml);;All Files (*)"));
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setOptions(QFileDialog::DontUseNativeDialog);  // Use Qt file dialog instead of native
        
        if (dialog.exec() == QDialog::Accepted)
        {
            QStringList selected_files = dialog.selectedFiles();
            if (!selected_files.isEmpty())
            {
                loadConfiguration(selected_files.at(0));
            }
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("File dialog error: %1").arg(QString::fromStdString(e.what())));
    }
}

void MainWindow::loadSchema(const QString& file_path)
{
    // Read the schema file
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open file: %1").arg(file_path));
        return;
    }

    const QString content = QString::fromUtf8(file.readAll());
    file.close();

    // Parse JSON schema
    try
    {
        const auto schema = nlohmann::json::parse(content.toStdString());

        // Generate form from schema
        if (form_generator_ && form_generator_->generateFromSchema(schema))
        {
            // Track the schema file for save operations
            current_schema_file_ = file_path;

            // Hide welcome label and show form
            auto* welcome_label = central_widget_->findChild<QLabel*>("welcomeLabel");
            if (welcome_label)
            {
                welcome_label->hide();
            }
            if (scroll_area_)
            {
                // Reset scroll position to top
                scroll_area_->verticalScrollBar()->setValue(0);
                scroll_area_->show();
            }
            form_generator_->show();

            // Update status
            if (status_label_)
            {
                status_label_->setText(tr("Schema loaded: %1").arg(QFileInfo(file_path).fileName()));
            }

            QMessageBox::information(this, tr("Schema Loaded"),
                tr("Schema loaded successfully and form generated!\n\nFile: %1").arg(file_path));
        }
        else
        {
            QMessageBox::warning(this, tr("Error"),
                tr("Failed to generate form from schema.\n\nFile: %1").arg(file_path));
        }
    }
    catch (const nlohmann::json::exception& e)
    {
        QMessageBox::critical(this, tr("JSON Error"),
            tr("Failed to parse JSON schema:\n\n%1").arg(QString::fromStdString(e.what())));
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("An error occurred:\n\n%1").arg(QString::fromStdString(e.what())));
    }
}

void MainWindow::loadConfiguration(const QString& file_path)
{
    // Read and validate the configuration file
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open file: %1").arg(file_path));
        return;
    }

    const QString content = QString::fromUtf8(file.readAll());
    file.close();

    try
    {
        // Parse the configuration JSON
        const auto config = nlohmann::json::parse(content.toStdString());

        // Update form with configuration data
        if (form_generator_)
        {
            form_generator_->setFormData(config);
        }

        // Track the configuration file for save operations
        current_config_file_ = file_path;

        // Update status
        if (status_label_)
        {
            status_label_->setText(tr("Loaded config: %1").arg(QFileInfo(file_path).fileName()));
        }

        QMessageBox::information(this, tr("Configuration Loaded"),
            tr("Configuration loaded successfully!\n\nFile: %1\n\nContent preview:\n%2")
            .arg(file_path)
            .arg(content.mid(0, 200) + (content.length() > 200 ? "..." : "")));
    }
    catch (const nlohmann::json::exception& e)
    {
        QMessageBox::critical(this, tr("JSON Error"),
            tr("Failed to parse JSON configuration:\n\n%1").arg(QString::fromStdString(e.what())));
    }
}

void MainWindow::onFileSave()
{
    // Check if we have a form generator and configuration loaded
    if (!form_generator_ || current_config_file_.isEmpty())
    {
        QMessageBox::warning(this, tr("Cannot Save"),
            tr("No configuration loaded. Please load a configuration file first."));
        return;
    }

    try
    {
        // Get current form data
        const auto form_data = form_generator_->getFormData();

        // Convert to string
        std::string json_string = form_data.dump(2);
        QString content = QString::fromStdString(json_string);

        // Write to file
        QFile file(current_config_file_);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, tr("Error"),
                tr("Cannot write to file: %1").arg(current_config_file_));
            return;
        }

        file.write(content.toUtf8());
        file.close();

        // Mark form as clean
        if (form_generator_)
        {
            form_generator_->markClean();
        }

        // Update status
        if (status_label_)
        {
            status_label_->setText(tr("Saved config: %1").arg(QFileInfo(current_config_file_).fileName()));
        }

        QMessageBox::information(this, tr("Configuration Saved"),
            tr("Configuration saved successfully!\n\nFile: %1").arg(current_config_file_));
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Failed to save configuration:\n\n%1").arg(QString::fromStdString(e.what())));
    }
}

void MainWindow::onFileExit()
{
    QApplication::quit();
}

void MainWindow::onHelpAbout()
{
    QMessageBox::about(this, tr("About ConfigGUI"),
                       tr("ConfigGUI v1.0\n\n"
                          "Schema-Driven Configuration Manager\n"
                          "Built with Qt 6 and C++17\n"
                          "MISRA C++ Compliant"));
}

} // namespace ui
} // namespace configgui
