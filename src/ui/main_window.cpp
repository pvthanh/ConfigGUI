// SPDX-License-Identifier: MIT
// MainWindow - Implementation

#include "main_window.h"
#include "form_generator.h"
#include "format_selection_dialog.h"
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
#include "core/io/ini_reader.h"

using namespace configgui::core::models;

namespace configgui {
namespace ui {

// Helper function to convert JSON to INI format
static QString convertJsonToIni(const nlohmann::json& json_data)
{
    QString ini_content;
    QTextStream stream(&ini_content);
    
    // Process each top-level key in the JSON
    for (auto& element : json_data.items())
    {
        const std::string& key = element.key();
        const auto& value = element.value();
        
        if (value.is_object())
        {
            // For nested objects, create a section [key]
            stream << "[" << QString::fromStdString(key) << "]\n";
            
            // Add each property in the section
            for (auto& sub_element : value.items())
            {
                const std::string& sub_key = sub_element.key();
                const auto& sub_value = sub_element.value();
                
                QString value_str;
                if (sub_value.is_string())
                {
                    value_str = QString::fromStdString(sub_value.get<std::string>());
                }
                else if (sub_value.is_number())
                {
                    if (sub_value.is_number_integer())
                    {
                        value_str = QString::number(sub_value.get<long long>());
                    }
                    else
                    {
                        value_str = QString::number(sub_value.get<double>());
                    }
                }
                else if (sub_value.is_boolean())
                {
                    value_str = sub_value.get<bool>() ? "true" : "false";
                }
                else if (sub_value.is_array())
                {
                    // For arrays, join with comma
                    std::string array_str;
                    for (const auto& item : sub_value)
                    {
                        if (!array_str.empty()) array_str += ",";
                        if (item.is_string()) array_str += item.get<std::string>();
                        else if (item.is_number()) array_str += std::to_string(item.get<double>());
                        else if (item.is_boolean()) array_str += (item.get<bool>() ? "true" : "false");
                    }
                    value_str = QString::fromStdString(array_str);
                }
                else if (sub_value.is_null())
                {
                    value_str = "";
                }
                
                stream << QString::fromStdString(sub_key) << "=" << value_str << "\n";
            }
            stream << "\n";
        }
        else
        {
            // For simple key-value pairs at top level
            QString value_str;
            if (value.is_string())
            {
                value_str = QString::fromStdString(value.get<std::string>());
            }
            else if (value.is_number())
            {
                if (value.is_number_integer())
                {
                    value_str = QString::number(value.get<long long>());
                }
                else
                {
                    value_str = QString::number(value.get<double>());
                }
            }
            else if (value.is_boolean())
            {
                value_str = value.get<bool>() ? "true" : "false";
            }
            else if (value.is_null())
            {
                value_str = "";
            }
            
            stream << QString::fromStdString(key) << "=" << value_str << "\n";
        }
    }
    
    return ini_content;
}

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
        dialog.setNameFilter(tr("Configuration Files (*.json *.yaml *.yml *.ini);;JSON Files (*.json);;YAML Files (*.yaml *.yml);;INI Files (*.ini);;All Files (*)"));
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

    // Parse JSON schema using ordered_json to preserve property order
    try
    {
        const auto schema = nlohmann::ordered_json::parse(content.toStdString());

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
        // Parse the configuration based on file format
        nlohmann::ordered_json config;
        QString file_extension = QFileInfo(file_path).suffix().toLower();

        if (file_extension == "ini")
        {
            // Parse INI file using INIReader
            auto result = configgui::io::INIReader::readString(content.toStdString());
            if (result.is_failure())
            {
                QMessageBox::critical(this, tr("INI Parse Error"),
                    tr("Failed to parse INI configuration:\n\n%1").arg(QString::fromStdString(result.error())));
                return;
            }
            config = result.value();
        }
        else
        {
            // Parse as JSON/YAML using ordered_json (YAML support via json parser)
            config = nlohmann::ordered_json::parse(content.toStdString());
        }

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
            tr("Configuration loaded successfully!\n\nFile: %1\n\nFormat: %2\n\nContent preview:\n%3")
            .arg(file_path)
            .arg(file_extension.toUpper())
            .arg(content.mid(0, 200) + (content.length() > 200 ? "..." : "")));
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, tr("Parse Error"),
            tr("Failed to parse configuration file:\n\n%1").arg(QString::fromStdString(e.what())));
    }
}

void MainWindow::onFileSave()
{
    // Check if we have a form generator
    if (!form_generator_)
    {
        QMessageBox::warning(this, tr("Cannot Save"),
            tr("No form available. Please load a schema first."));
        return;
    }

    // Let user select format (JSON or INI)
    FormatSelectionDialog format_dialog(this);
    if (format_dialog.exec() != QDialog::Accepted)
    {
        return;  // User cancelled format selection
    }
    
    FormatType selected_format = format_dialog.selected_format();
    QString format_extension = format_dialog.selected_format_extension();
    QString format_filter = QString("Configuration Files (*%1);;All Files (*)").arg(format_extension);

    QString save_path = current_config_file_;

    // If no configuration is currently loaded, ask user where to save
    if (current_config_file_.isEmpty())
    {
        save_path = QFileDialog::getSaveFileName(this,
            tr("Save New Configuration"), "",
            format_filter);
        
        if (save_path.isEmpty())
        {
            return;  // User cancelled
        }
    }
    else
    {
        // Check if the current file has a different extension than selected format
        QString current_extension = QFileInfo(current_config_file_).suffix();
        QString expected_extension = format_extension.right(format_extension.length() - 1); // Remove the dot
        
        if (current_extension.toLower() != expected_extension.toLower())
        {
            // Different format - ask user if they want to save as new file
            QMessageBox::StandardButton reply = QMessageBox::question(this,
                tr("Save in Different Format?"),
                tr("Current file is .%1 but you selected .%2 format.\n\nWould you like to save as a new file?")
                    .arg(current_extension, expected_extension),
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes)
            {
                // Open file chooser for new file
                save_path = QFileDialog::getSaveFileName(this,
                    tr("Save Configuration As"), "",
                    format_filter);
                
                if (save_path.isEmpty())
                {
                    return;  // User cancelled
                }
            }
            else
            {
                return;  // User chose not to save
            }
        }
        else
        {
            // Same format - ask if user wants to overwrite
            QMessageBox::StandardButton reply = QMessageBox::question(this,
                tr("Overwrite Configuration?"),
                tr("Do you want to overwrite the current configuration file?\n\nFile: %1\n\nClick 'No' to save to a different location.")
                    .arg(QFileInfo(current_config_file_).fileName()),
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::No)
            {
                // User chose not to overwrite - open file chooser
                save_path = QFileDialog::getSaveFileName(this,
                    tr("Save Configuration As"), current_config_file_,
                    format_filter);
                
                if (save_path.isEmpty())
                {
                    return;  // User cancelled
                }
            }
        }
    }

    // Now save to the determined path in the selected format
    try
    {
        // Get current form data as JSON
        const auto form_data = form_generator_->getFormData();

        QString content;
        
        if (selected_format == FormatType::JSON)
        {
            // Save as JSON
            std::string json_string = form_data.dump(2);
            content = QString::fromStdString(json_string);
        }
        else if (selected_format == FormatType::INI)
        {
            // Save as INI - convert JSON to INI format
            // INI format: key=value with sections [section.subsection]
            content = convertJsonToIni(form_data);
        }
        else
        {
            QMessageBox::critical(this, tr("Error"),
                tr("Unsupported format selected"));
            return;
        }

        // Write to file
        QFile file(save_path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, tr("Error"),
                tr("Cannot write to file: %1").arg(save_path));
            return;
        }

        file.write(content.toUtf8());
        file.close();

        // Update the current config file path if we saved to a new location
        if (save_path != current_config_file_)
        {
            current_config_file_ = save_path;
        }

        // Mark form as clean
        if (form_generator_)
        {
            form_generator_->markClean();
        }

        // Update status
        if (status_label_)
        {
            status_label_->setText(tr("Saved config: %1").arg(QFileInfo(save_path).fileName()));
        }

        QMessageBox::information(this, tr("Configuration Saved"),
            tr("Configuration saved successfully as %1!\n\nFile: %2")
                .arg(format_dialog.selected_format_name(), save_path));
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
