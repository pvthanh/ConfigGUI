// SPDX-License-Identifier: MIT
// PathSelectorWidget - Implementation

#include "path_selector_widget.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QMenu>
#include <QAction>

namespace configgui {
namespace ui {

PathSelectorWidget::PathSelectorWidget(const json& schema, const QString& fieldName, QWidget* parent)
    : QWidget(parent)
    , edit_(new QLineEdit(this))
    , browse_(new QToolButton(this))
    , allow_file_(true)
    , allow_dir_(true)
{
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(0, 0, 0, 0);

    // Default look
    browse_->setText("...");
    browse_->setToolTip("Browse for file or folder");

    inferBehaviorFromSchema(schema, fieldName);

    // Set default value if present
    if (schema.contains("default") && schema["default"].is_string())
    {
        edit_->setText(QString::fromStdString(schema["default"].get<std::string>()));
    }
    else if (schema.contains("examples") && schema["examples"].is_array() && schema["examples"].size() > 0 && schema["examples"][0].is_string())
    {
        edit_->setPlaceholderText(QString::fromStdString(schema["examples"][0].get<std::string>()));
    }

    layout->addWidget(edit_, 1);
    layout->addWidget(browse_);

    connect(browse_, &QToolButton::clicked, this, &PathSelectorWidget::onBrowseClicked);
    connect(edit_, &QLineEdit::textChanged, this, &PathSelectorWidget::pathChanged);
}

QString PathSelectorWidget::text() const
{
    return edit_->text();
}

void PathSelectorWidget::setText(const QString& value)
{
    edit_->setText(value);
}

void PathSelectorWidget::onBrowseClicked()
{
    if (allow_file_ && allow_dir_)
    {
        // Present a small menu to choose between file and folder
        auto* menu = new QMenu(this);
        QAction* actFile = menu->addAction("Select File…");
        QAction* actFolder = menu->addAction("Select Folder…");
        QAction* chosen = menu->exec(browse_->mapToGlobal(QPoint(0, browse_->height())));
        if (chosen == actFile) { chooseFile(); }
        else if (chosen == actFolder) { chooseFolder(); }
        return;
    }

    if (allow_file_) { chooseFile(); return; }
    if (allow_dir_) { chooseFolder(); return; }
}

void PathSelectorWidget::chooseFile()
{
    const QString start_dir = edit_->text().isEmpty() ? QString() : QFileInfo(edit_->text()).absolutePath();
    const QString filter = file_filter_.isEmpty() ? QStringLiteral("All Files (*.*)") : file_filter_;
    const QString file = QFileDialog::getOpenFileName(this, tr("Select file"), start_dir, filter);
    if (!file.isEmpty())
    {
        edit_->setText(file);
        emit pathChanged(file);
    }
}

void PathSelectorWidget::chooseFolder()
{
    const QString start_dir = edit_->text().isEmpty() ? QString() : QFileInfo(edit_->text()).absolutePath();
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Select folder"), start_dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
    {
        edit_->setText(dir);
        emit pathChanged(dir);
    }
}

void PathSelectorWidget::inferBehaviorFromSchema(const json& schema, const QString& fieldName)
{
    // Heuristics: if name contains "file" -> file only; if contains "path"/"dir"/"folder" -> both (prefer folder)
    const QString name = fieldName.toLower();
    if (name.contains("file"))
    {
        allow_file_ = true;
        allow_dir_ = false;
    }
    else if (name.contains("configpath") || name.contains("path") || name.contains("dir") || name.contains("folder"))
    {
        allow_file_ = true;
        allow_dir_ = true; // allow both
    }

    // If schema has a pattern indicating an extension, set a filter and bias to file selection
    if (schema.contains("pattern") && schema["pattern"].is_string())
    {
        const std::string pattern = schema["pattern"].get<std::string>();
        if (pattern.find(".json") != std::string::npos)
        {
            file_filter_ = QStringLiteral("JSON Files (*.json)");
            allow_file_ = true;
        }
        else if (pattern.find(".csv") != std::string::npos)
        {
            file_filter_ = QStringLiteral("CSV Files (*.csv)");
            allow_file_ = true;
        }
    }

    // Placeholder/tooltip from schema
    if (schema.contains("x-hint"))
    {
        edit_->setToolTip(QString::fromStdString(schema["x-hint"].get<std::string>()));
    }
}

} // namespace ui
} // namespace configgui
