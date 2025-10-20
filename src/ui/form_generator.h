// SPDX-License-Identifier: MIT
// FormGenerator - Header

#ifndef CONFIGGUI_UI_FORM_GENERATOR_H
#define CONFIGGUI_UI_FORM_GENERATOR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace configgui {
namespace ui {

/**
 * @class FormGenerator
 * @brief Generates dynamic forms from JSON schema
 *
 * Creates form widgets with validation rules from schema definitions.
 * Manages form state and change tracking.
 */
class FormGenerator : public QWidget
{
    Q_OBJECT

public:
    explicit FormGenerator(QWidget* parent = nullptr);
    ~FormGenerator() override = default;

    /**
     * @brief Generate form from schema
     * @param schema JSON schema object
     * @return Success status
     */
    bool generateFromSchema(const json& schema);

    /**
     * @brief Get current form data
     * @return JSON object with current values
     */
    json getFormData() const;

    /**
     * @brief Set form data
     * @param data JSON object with values
     */
    void setFormData(const json& data);

    /**
     * @brief Clear all form fields
     */
    void clearForm();

    /**
     * @brief Check if form has unsaved changes
     * @return True if modified
     */
    bool isDirty() const { return is_dirty_; }

    /**
     * @brief Mark form as clean
     */
    void markClean() { is_dirty_ = false; }

signals:
    void formModified();
    void validationChanged(const QString& field, bool is_valid);

private slots:
    void onFieldChanged();

private:
    struct FieldWidget {
        QWidget* widget;
        QString field_name;
        json schema;
    };

    QVBoxLayout* layout_;
    QMap<QString, FieldWidget> field_widgets_;
    bool is_dirty_;

    void addFieldToForm(const QString& field_name, const json& field_schema);
    void updateFieldValue(const QString& field_name, const json& value);
};

} // namespace ui
} // namespace configgui

#endif // CONFIGGUI_UI_FORM_GENERATOR_H
