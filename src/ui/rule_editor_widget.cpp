// SPDX-License-Identifier: MIT
// RuleEditorWidget - Implementation

#include "rule_editor_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <sstream>
#include <limits>
#include <cmath>

namespace configgui {
namespace ui {

// MISRA C++ compliant float comparison
static bool isFloatEqual(double a, double b)
{
    return std::fabs(a - b) < 1e-9;
}

RuleEditorWidget::RuleEditorWidget(const std::string& field_name, const std::string& shorthand, QWidget* parent)
    : QWidget(parent)
    , field_name_(field_name)
    , string_allow_empty_(nullptr)
    , string_pattern_(nullptr)
    , string_enum_(nullptr)
    , numeric_minimum_(nullptr)
    , numeric_maximum_(nullptr)
{
    // Parse the shorthand to get current rule definition
    current_rule_ = RuleParser::parseShorthand(field_name, shorthand);

    createUI();
}

RuleEditorWidget::RuleEditorWidget(const std::string& field_name, const RuleDefinition& rule, QWidget* parent)
    : QWidget(parent)
    , field_name_(field_name)
    , current_rule_(rule)
    , string_allow_empty_(nullptr)
    , string_pattern_(nullptr)
    , string_enum_(nullptr)
    , numeric_minimum_(nullptr)
    , numeric_maximum_(nullptr)
{
    // Use the rule definition directly - no shorthand parsing needed
    createUI();
}

void RuleEditorWidget::createUI()
{
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(8);
    main_layout->setContentsMargins(5, 5, 5, 5);

    // Header: Field Name and Type
    auto* header_layout = new QHBoxLayout();
    header_layout->setSpacing(5);

    auto* name_label = new QLabel("Field Name:");
    field_name_edit_ = new QLineEdit();
    field_name_edit_->setText(QString::fromStdString(field_name_));
    field_name_edit_->setReadOnly(true);  // Name is fixed once created
    field_name_edit_->setMaximumWidth(150);

    auto* type_label = new QLabel("Type:");
    type_combo_ = new QComboBox();
    type_combo_->addItems({"string", "integer", "float", "boolean"});
    // Set current type
    int type_index = type_combo_->findText(QString::fromStdString(current_rule_.type));
    if (type_index >= 0) type_combo_->setCurrentIndex(type_index);
    type_combo_->setMaximumWidth(120);

    auto* required_label = new QLabel("Required:");
    required_check_ = new QCheckBox();
    required_check_->setChecked(current_rule_.required);
    required_check_->setMaximumWidth(30);

    header_layout->addWidget(name_label);
    header_layout->addWidget(field_name_edit_, 1);
    header_layout->addWidget(type_label);
    header_layout->addWidget(type_combo_);
    header_layout->addWidget(required_label);
    header_layout->addWidget(required_check_);
    header_layout->addStretch();

    main_layout->addLayout(header_layout);

    // Dynamic widgets based on type
    dynamic_widget_ = new QWidget();
    dynamic_layout_ = new QVBoxLayout(dynamic_widget_);
    dynamic_layout_->setSpacing(5);
    dynamic_layout_->setContentsMargins(0, 0, 0, 0);

    main_layout->addWidget(dynamic_widget_);

    // Preview label
    preview_label_ = new QLabel();
    preview_label_->setStyleSheet("color: #666; font-style: italic; font-size: 10pt;");
    main_layout->addWidget(preview_label_);

    // Separator
    main_layout->addSpacing(5);

    // Signals
    connect(type_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RuleEditorWidget::onTypeChanged);
    connect(required_check_, &QCheckBox::stateChanged,
            this, &RuleEditorWidget::onRequiredChanged);

    // Initial UI setup
    createDynamicWidgets();
    updatePreview();
}

void RuleEditorWidget::createDynamicWidgets()
{
    clearDynamicWidgets();

    std::string type = type_combo_->currentText().toStdString();

    if (type == "string")
    {
        auto* form = new QFormLayout();
        form->setSpacing(5);
        form->setContentsMargins(0, 0, 0, 0);

        string_allow_empty_ = new QCheckBox("Allow Empty");
        string_allow_empty_->setChecked(current_rule_.allow_empty);
        form->addRow(string_allow_empty_);
        connect(string_allow_empty_, &QCheckBox::stateChanged, this, &RuleEditorWidget::updatePreview);

        auto* pattern_label = new QLabel("Pattern (Optional):");
        string_pattern_ = new QLineEdit();
        string_pattern_->setPlaceholderText("Regex pattern...");
        string_pattern_->setText(QString::fromStdString(current_rule_.pattern));
        form->addRow(pattern_label, string_pattern_);
        connect(string_pattern_, &QLineEdit::textChanged, this, &RuleEditorWidget::updatePreview);

        auto* enum_label = new QLabel("Enum Values (Optional):");
        enum_label->setToolTip("Separate values with | (e.g., Even|Odd|Mixed)");
        string_enum_ = new QLineEdit();
        string_enum_->setPlaceholderText("value1|value2|value3");
        if (!current_rule_.enum_values.empty())
        {
            std::string enum_str;
            for (size_t i = 0; i < current_rule_.enum_values.size(); ++i)
            {
                if (i > 0) enum_str += "|";
                enum_str += current_rule_.enum_values[i];
            }
            string_enum_->setText(QString::fromStdString(enum_str));
        }
        form->addRow(enum_label, string_enum_);
        connect(string_enum_, &QLineEdit::textChanged, this, &RuleEditorWidget::updatePreview);

        dynamic_layout_->addLayout(form);
    }
    else if (type == "integer" || type == "float")
    {
        auto* form = new QFormLayout();
        form->setSpacing(5);
        form->setContentsMargins(0, 0, 0, 0);

        numeric_minimum_ = new QDoubleSpinBox();
        numeric_minimum_->setMinimum(-999999);
        numeric_minimum_->setMaximum(999999);
        if (!isFloatEqual(current_rule_.minimum, std::numeric_limits<double>::lowest()))
        {
            numeric_minimum_->setValue(current_rule_.minimum);
        }
        form->addRow("Minimum:", numeric_minimum_);
        connect(numeric_minimum_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &RuleEditorWidget::updatePreview);

        numeric_maximum_ = new QDoubleSpinBox();
        numeric_maximum_->setMinimum(-999999);
        numeric_maximum_->setMaximum(999999);
        if (!isFloatEqual(current_rule_.maximum, std::numeric_limits<double>::max()))
        {
            numeric_maximum_->setValue(current_rule_.maximum);
        }
        form->addRow("Maximum:", numeric_maximum_);
        connect(numeric_maximum_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &RuleEditorWidget::updatePreview);

        dynamic_layout_->addLayout(form);
    }
    else if (type == "boolean")
    {
        auto* label = new QLabel("Boolean has no additional constraints");
        label->setStyleSheet("color: #999;");
        dynamic_layout_->addWidget(label);
    }

    dynamic_layout_->addStretch();
}

void RuleEditorWidget::clearDynamicWidgets()
{
    // Delete existing dynamic widgets
    string_allow_empty_ = nullptr;
    string_pattern_ = nullptr;
    string_enum_ = nullptr;
    numeric_minimum_ = nullptr;
    numeric_maximum_ = nullptr;

    // Clear layout completely - remove all items and delete their widgets
    while (dynamic_layout_->count() > 0)
    {
        QLayoutItem* item = dynamic_layout_->takeAt(0);
        if (item)
        {
            // If item contains a layout, clear it first
            if (item->layout())
            {
                QLayout* layout = item->layout();
                while (layout->count() > 0)
                {
                    QLayoutItem* child_item = layout->takeAt(0);
                    if (child_item->widget())
                    {
                        delete child_item->widget();
                    }
                    delete child_item;
                }
                delete layout;
            }
            // If item contains a widget, delete it
            else if (item->widget())
            {
                delete item->widget();
            }
            delete item;
        }
    }
}

void RuleEditorWidget::onTypeChanged(int index)
{
    Q_UNUSED(index);
    createDynamicWidgets();
    updatePreview();
    dynamic_layout_->update();
    dynamic_widget_->update();
    update();
    emit ruleChanged();
}

void RuleEditorWidget::onRequiredChanged(int state)
{
    Q_UNUSED(state);
    updatePreview();
    emit ruleChanged();
}

void RuleEditorWidget::updatePreview()
{
    RuleDefinition rule = getRule();
    std::string shorthand = RuleParser::toShorthand(rule);
    preview_label_->setText(QString("Preview: ") + QString::fromStdString(getFieldName()) + " : " +
                           QString::fromStdString(shorthand));
}

std::string RuleEditorWidget::getShorthand() const
{
    return RuleParser::toShorthand(getRule());
}

RuleDefinition RuleEditorWidget::getRule() const
{
    RuleDefinition rule;
    rule.name = field_name_;
    rule.type = type_combo_->currentText().toStdString();
    rule.required = required_check_->isChecked();
    rule.allow_empty = !rule.required;

    if (rule.type == "string")
    {
        if (string_allow_empty_)
        {
            rule.allow_empty = string_allow_empty_->isChecked();
            rule.required = !rule.allow_empty;
        }

        if (string_pattern_)
        {
            rule.pattern = string_pattern_->text().toStdString();
        }

        if (string_enum_)
        {
            std::string enum_str = string_enum_->text().toStdString();
            if (!enum_str.empty())
            {
                std::istringstream iss(enum_str);
                std::string val;
                while (std::getline(iss, val, '|'))
                {
                    val.erase(0, val.find_first_not_of(" \t"));
                    val.erase(val.find_last_not_of(" \t") + 1);
                    if (!val.empty())
                    {
                        rule.enum_values.push_back(val);
                    }
                }
            }
        }
    }
    else if (rule.type == "integer" || rule.type == "float")
    {
        if (numeric_minimum_)
        {
            rule.minimum = numeric_minimum_->value();
        }
        if (numeric_maximum_)
        {
            rule.maximum = numeric_maximum_->value();
        }
    }

    return rule;
}

} // namespace ui
} // namespace configgui
