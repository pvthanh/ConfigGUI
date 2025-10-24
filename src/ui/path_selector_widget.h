// SPDX-License-Identifier: MIT
// PathSelectorWidget - Composite widget for selecting file or folder paths

#pragma once

#include <QWidget>
#include <QString>
#include <nlohmann/json.hpp>

class QLineEdit;
class QToolButton;

using json = nlohmann::ordered_json;

namespace configgui {
namespace ui {

class PathSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PathSelectorWidget(const json& schema, const QString& fieldName, QWidget* parent = nullptr);
    ~PathSelectorWidget() override = default;

    QString text() const;
    void setText(const QString& value);

signals:
    void pathChanged(const QString& value);

private slots:
    void onBrowseClicked();
    void chooseFile();
    void chooseFolder();

private:
    void inferBehaviorFromSchema(const json& schema, const QString& fieldName);

    QLineEdit* edit_;
    QToolButton* browse_;
    bool allow_file_;
    bool allow_dir_;
    QString file_filter_;
};

} // namespace ui
} // namespace configgui
