// SPDX-License-Identifier: MIT
// RuleEditorWidget - Header for editing rules with dynamic UI

#ifndef CONFIGGUI_UI_RULE_EDITOR_WIDGET_H
#define CONFIGGUI_UI_RULE_EDITOR_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <nlohmann/json.hpp>
#include "rule_parser.h"

namespace configgui {
namespace ui {

/**
 * @class RuleEditorWidget
 * @brief Widget for editing a single rule with dynamic UI based on type
 */
class RuleEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RuleEditorWidget(const std::string& field_name, const std::string& shorthand, QWidget* parent = nullptr);
    ~RuleEditorWidget() override = default;

    /**
     * @brief Get the edited rule in shorthand format
     * @return Shorthand string (e.g., "string?", "integer[0,5]")
     */
    std::string getShorthand() const;

    /**
     * @brief Get the field name
     */
    std::string getFieldName() const { return field_name_; }

    /**
     * @brief Get the rule definition
     */
    RuleDefinition getRule() const;

signals:
    void ruleChanged();

private slots:
    void onTypeChanged(int index);
    void onRequiredChanged(int state);
    void updatePreview();

private:
    void createUI();
    void createDynamicWidgets();
    void clearDynamicWidgets();
    void loadRuleIntoUI(const RuleDefinition& rule);

    // Static helper to get default minimum/maximum for a type
    static double getDefaultMinimum(const std::string& type);
    static double getDefaultMaximum(const std::string& type);

    std::string field_name_;
    RuleDefinition current_rule_;

    // UI Components - Static
    QLineEdit* field_name_edit_;
    QComboBox* type_combo_;
    QCheckBox* required_check_;
    QLabel* preview_label_;

    // UI Components - Dynamic (shown based on type)
    QWidget* dynamic_widget_;
    QVBoxLayout* dynamic_layout_;

    // Dynamic widgets for each type
    // String
    QCheckBox* string_allow_empty_;
    QLineEdit* string_pattern_;
    QLineEdit* string_enum_;

    // Integer/Float
    QDoubleSpinBox* numeric_minimum_;
    QDoubleSpinBox* numeric_maximum_;
};

} // namespace ui
} // namespace configgui

#endif // CONFIGGUI_UI_RULE_EDITOR_WIDGET_H
