// SPDX-License-Identifier: MIT
// FormGenerator - Implementation

#include "form_generator.h"
#include "widget_factory.h"
#include "path_selector_widget.h"
#include "array_widget.h"
#include "range_widget.h"
#include "dictionary_widget.h"
#include "object_array_widget.h"
#include "rule_parser.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QPropertyAnimation>
#include <iostream>

namespace configgui {
namespace ui {

// Helper function to capitalize first letter of string
static QString capitalizeFirstLetter(const QString& str)
{
    if (str.isEmpty())
        return str;
    return str.at(0).toUpper() + str.mid(1);
}

FormGenerator::FormGenerator(QWidget* parent)
    : QWidget(parent)
    , layout_(new QVBoxLayout(this))
    , is_dirty_(false)
{
    layout_->setSpacing(10);
    layout_->setContentsMargins(10, 10, 10, 10);
}

bool FormGenerator::generateFromSchema(const json& schema)
{
    // Clear existing widgets
    clearForm();

    if (!schema.is_object() || !schema.contains("properties"))
    {
        return false;
    }

    // Store the schema for later use in getFormData()
    schema_ = schema;

    const auto& properties = schema["properties"];
    if (!properties.is_object())
    {
        return false;
    }

    // Use "required" array to maintain order if available, otherwise iterate normally
    std::vector<std::string> field_order;
    
    if (schema.contains("required") && schema["required"].is_array())
    {
        // Use the required array order
        for (const auto& required_field : schema["required"])
        {
            if (required_field.is_string())
            {
                field_order.push_back(required_field.get<std::string>());
            }
        }
    }
    else
    {
        // Fallback: iterate through properties (may not preserve order)
        for (auto it = properties.begin(); it != properties.end(); ++it)
        {
            field_order.push_back(it.key());
        }
    }

    // Iterate in determined order
    for (const auto& field_name_str : field_order)
    {
        if (properties.contains(field_name_str))
        {
            const QString field_name = QString::fromStdString(field_name_str);
            const json& field_schema = properties[field_name_str];
            addFieldToForm(field_name, field_schema);
        }
    }

    layout_->addStretch();
    return true;
}

void FormGenerator::addFieldToForm(const QString& field_name, const json& field_schema)
{
    // Step 1: Check if it's a nested object with properties (type=object)
    if (field_schema.is_object() && field_schema.contains("properties"))
    {
        const auto field_type = field_schema.contains("type") ? 
            field_schema["type"].get<std::string>() : "object";
        
        if (field_type == "object")
        {
            // Create a collapsible section for this object
            auto* section_container = new QWidget();
            auto* section_layout = new QVBoxLayout(section_container);
            section_layout->setSpacing(5);
            section_layout->setContentsMargins(0, 0, 0, 0);

            // Create header bar with collapse/expand button
            auto* header_container = new QWidget();
            auto* header_layout = new QHBoxLayout(header_container);
            header_layout->setSpacing(5);
            header_layout->setContentsMargins(0, 10, 0, 5);

            // Collapse/expand button (triangle/arrow)
            auto* toggle_button = new QToolButton();
            toggle_button->setText("▶");  // Right arrow (collapsed) - start collapsed
            toggle_button->setStyleSheet("QToolButton { border: none; background: none; font-size: 12px; padding: 0px; }");
            toggle_button->setMaximumWidth(25);
            
            // Section label with capitalized name
            QString capitalized_name = capitalizeFirstLetter(field_name);
            QString header_text = capitalized_name + ":";
            if (field_schema.contains("description"))
            {
                header_text += " - " + QString::fromStdString(field_schema["description"].get<std::string>());
            }
            
            auto* section_label = new QLabel(header_text);
            section_label->setStyleSheet("font-weight: bold; color: #1a1a1a;");
            section_label->setWordWrap(true);
            
            header_layout->addWidget(toggle_button);
            header_layout->addWidget(section_label, 1);
            section_layout->addWidget(header_container);

            // Create inner container with indentation for nested fields
            auto* inner_container = new QWidget();
            auto* inner_layout = new QVBoxLayout(inner_container);
            inner_layout->setSpacing(8);
            inner_layout->setContentsMargins(20, 0, 0, 0);  // Add left margin for indentation
            
            // Add fields from nested object recursively using the "required" order
            const auto& properties = field_schema["properties"];
            std::vector<std::string> nested_field_order;
            
            if (field_schema.contains("required") && field_schema["required"].is_array())
            {
                // Use the required array order
                for (const auto& required_field : field_schema["required"])
                {
                    if (required_field.is_string())
                    {
                        nested_field_order.push_back(required_field.get<std::string>());
                    }
                }
            }
            else
            {
                // Fallback: iterate through properties
                for (auto it = properties.begin(); it != properties.end(); ++it)
                {
                    nested_field_order.push_back(it.key());
                }
            }
            
            // Iterate in determined order
            for (const auto& nested_field_name_str : nested_field_order)
            {
                if (properties.contains(nested_field_name_str))
                {
                    const QString nested_field_name = QString::fromStdString(nested_field_name_str);
                    const json& nested_field_schema = properties[nested_field_name_str];
                    addFieldToFormWithMetadata(inner_layout, nested_field_name, nested_field_schema);
                }
            }
            
            section_layout->addWidget(inner_container);
            
            // Set initial state to collapsed (hidden)
            inner_container->setVisible(false);
            
            // Connect toggle button to show/hide inner_container
            QObject::connect(toggle_button, &QToolButton::clicked, [toggle_button, inner_container]()
            {
                bool is_visible = inner_container->isVisible();
                inner_container->setVisible(!is_visible);
                toggle_button->setText(is_visible ? "▶" : "▼");  // Toggle arrow direction
            });
            
            layout_->addWidget(section_container);
            return;
        }
    }

    // Step 2: Otherwise, add as a simple field with metadata
    addFieldToFormWithMetadata(layout_, field_name, field_schema);
}

void FormGenerator::addFieldToFormWithMetadata(QVBoxLayout* parent_layout, const QString& field_name, const json& field_schema)
{
    // Step 1: Check if it's a nested object with properties
    if (field_schema.is_object() && field_schema.contains("properties"))
    {
        const auto field_type = field_schema.contains("type") ? 
            field_schema["type"].get<std::string>() : "";
        
        if (field_type == "object" || (field_type.empty() && field_schema.contains("additionalProperties")))
        {
            // Create a section for this nested object with toggle button
            auto* section_container = new QWidget();
            auto* section_layout = new QVBoxLayout(section_container);
            section_layout->setSpacing(5);
            section_layout->setContentsMargins(0, 0, 0, 0);

            // Create header bar with collapse/expand button
            auto* header_container = new QWidget();
            auto* header_layout = new QHBoxLayout(header_container);
            header_layout->setSpacing(5);
            header_layout->setContentsMargins(0, 5, 0, 2);

            // Collapse/expand button (triangle/arrow)
            auto* toggle_button = new QToolButton();
            toggle_button->setText("▶");  // Right arrow (collapsed) - start collapsed
            toggle_button->setStyleSheet("QToolButton { border: none; background: none; font-size: 11px; padding: 0px; margin: 0px; }");
            toggle_button->setMaximumWidth(20);
            
            // Section label
            QString header_text = field_name + ":";
            if (field_schema.contains("description"))
            {
                header_text += " - " + QString::fromStdString(field_schema["description"].get<std::string>());
            }
            
            auto* section_label = new QLabel(header_text);
            section_label->setStyleSheet("font-weight: bold; color: #1a1a1a;");
            section_label->setWordWrap(true);
            
            header_layout->addWidget(toggle_button);
            header_layout->addWidget(section_label, 1);
            section_layout->addWidget(header_container);

            // Create inner container with indentation for nested fields
            auto* inner_container = new QWidget();
            auto* inner_layout = new QVBoxLayout(inner_container);
            inner_layout->setSpacing(8);
            inner_layout->setContentsMargins(20, 0, 0, 0);  // Add left margin for indentation
            
            // Add fields from nested object recursively using the "required" order
            const auto& properties = field_schema["properties"];
            std::vector<std::string> nested_field_order;
            
            if (field_schema.contains("required") && field_schema["required"].is_array())
            {
                // Use the required array order
                for (const auto& required_field : field_schema["required"])
                {
                    if (required_field.is_string())
                    {
                        nested_field_order.push_back(required_field.get<std::string>());
                    }
                }
            }
            else
            {
                // Fallback: iterate through properties
                for (auto it = properties.begin(); it != properties.end(); ++it)
                {
                    nested_field_order.push_back(it.key());
                }
            }
            
            // Iterate in determined order
            for (const auto& nested_field_name_str : nested_field_order)
            {
                if (properties.contains(nested_field_name_str))
                {
                    const QString nested_field_name = QString::fromStdString(nested_field_name_str);
                    const json& nested_field_schema = properties[nested_field_name_str];
                    
                    // Recursively handle nested fields (this will call addFieldToFormWithMetadata again)
                    addFieldToFormWithMetadata(inner_layout, nested_field_name, nested_field_schema);
                }
            }
            
            section_layout->addWidget(inner_container);
            
            // Set initial state to collapsed (hidden)
            inner_container->setVisible(false);
            
            // Connect toggle button to show/hide inner_container
            QObject::connect(toggle_button, &QToolButton::clicked, [toggle_button, inner_container]()
            {
                bool is_visible = inner_container->isVisible();
                inner_container->setVisible(!is_visible);
                toggle_button->setText(is_visible ? "▶" : "▼");  // Toggle arrow direction
            });
            
            parent_layout->addWidget(section_container);
            return;
        }
    }

    // Step 2: Otherwise, add as a simple field with metadata
    // Prefer a specialized PathSelectorWidget for path-like string fields
    QWidget* widget = nullptr;
    const bool is_string_type = field_schema.contains("type") && field_schema["type"].is_string() && field_schema["type"].get<std::string>() == "string";
    auto looks_path_like = [&](const QString& name, const json& schema) -> bool {
        const QString n = name.toLower();
        if (n.contains("file") || n.contains("configpath") || n.contains("path") || n.contains("dir") || n.contains("folder"))
        {
            return true;
        }
        if (schema.contains("pattern") && schema["pattern"].is_string())
        {
            const std::string p = schema["pattern"].get<std::string>();
            if (p.find(".json") != std::string::npos || p.find(".csv") != std::string::npos)
            {
                return true;
            }
        }
        return false;
    };

    if (is_string_type && looks_path_like(field_name, field_schema))
    {
        // Use a dedicated path selector composite widget
        widget = new PathSelectorWidget(field_schema, field_name);
    }
    else
    {
        // Create input widget using WidgetFactory
        WidgetFactory factory;
        widget = factory.createWidget(field_schema);
    }

    // Skip if widget creation returned null (unsupported type)
    if (widget == nullptr)
    {
        return;
    }

    // Check if it's a real widget type (including ArrayWidget, RangeWidget, DictionaryWidget, and ObjectArrayWidget)
    if (!qobject_cast<QLineEdit*>(widget) &&
        !qobject_cast<QSpinBox*>(widget) &&
        !qobject_cast<QDoubleSpinBox*>(widget) &&
        !qobject_cast<QCheckBox*>(widget) &&
        !qobject_cast<QComboBox*>(widget) &&
        !qobject_cast<ArrayWidget*>(widget) &&
        !qobject_cast<RangeWidget*>(widget) &&
        !qobject_cast<DictionaryWidget*>(widget) &&
        !qobject_cast<ObjectArrayWidget*>(widget) &&
        !qobject_cast<PathSelectorWidget*>(widget))
    {
        widget->deleteLater();
        return;
    }

    // Create container for the field with label and widget
    auto* field_container = new QWidget();
    auto* field_layout = new QVBoxLayout(field_container);
    field_layout->setSpacing(3);
    field_layout->setContentsMargins(0, 0, 0, 0);

    // Create label with required indicator
    QString label_text = field_name;
    auto* label = new QLabel(label_text + ":");
    label->setStyleSheet("font-weight: 500; color: #222222;");
    
    auto* h_layout = new QHBoxLayout();
    h_layout->addWidget(label, 0, Qt::AlignTop);
    h_layout->addWidget(widget, 1);
    h_layout->addStretch();
    
    field_layout->addLayout(h_layout);

    // Add hint text if available
    if (field_schema.contains("x-hint"))
    {
        auto* hint_label = new QLabel(QString::fromStdString(field_schema["x-hint"].get<std::string>()));
        hint_label->setStyleSheet("font-size: 11px; color: #666666; font-style: italic;");
        hint_label->setWordWrap(true);
        field_layout->addWidget(hint_label);
    }

    // Add description if available (in addition to x-hint)
    if (field_schema.contains("description") && !field_schema.contains("x-hint"))
    {
        auto* desc_label = new QLabel(QString::fromStdString(field_schema["description"].get<std::string>()));
        desc_label->setStyleSheet("font-size: 10px; color: #777777;");
        desc_label->setWordWrap(true);
        field_layout->addWidget(desc_label);
    }

    parent_layout->addWidget(field_container);

    // Track the field widget
    FieldWidget fw{widget, field_name, field_schema};
    field_widgets_[field_name] = fw;

    // Connect change signals for all widget types
    if (auto* line_edit = qobject_cast<QLineEdit*>(widget))
    {
        connect(line_edit, &QLineEdit::textChanged, this, &FormGenerator::onFieldChanged);
    }
    else if (auto* path_widget = qobject_cast<PathSelectorWidget*>(widget))
    {
        connect(path_widget, &PathSelectorWidget::pathChanged, this, &FormGenerator::onFieldChanged);
    }
    else if (auto* spin_box = qobject_cast<QSpinBox*>(widget))
    {
        connect(spin_box, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &FormGenerator::onFieldChanged);
    }
    else if (auto* double_spin = qobject_cast<QDoubleSpinBox*>(widget))
    {
        connect(double_spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &FormGenerator::onFieldChanged);
    }
    else if (auto* check_box = qobject_cast<QCheckBox*>(widget))
    {
        connect(check_box, &QCheckBox::stateChanged, this, &FormGenerator::onFieldChanged);
    }
    else if (auto* combo_box = qobject_cast<QComboBox*>(widget))
    {
        connect(combo_box, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &FormGenerator::onFieldChanged);
    }
    else if (auto* array_widget = qobject_cast<ArrayWidget*>(widget))
    {
        connect(array_widget, &ArrayWidget::valuesChanged, this, &FormGenerator::onFieldChanged);
    }
    else if (auto* range_widget = qobject_cast<RangeWidget*>(widget))
    {
        connect(range_widget, &RangeWidget::valuesChanged, this, &FormGenerator::onFieldChanged);
    }
    else if (auto* dict_widget = qobject_cast<DictionaryWidget*>(widget))
    {
        connect(dict_widget, &DictionaryWidget::valuesChanged, this, &FormGenerator::onFieldChanged);
    }
    else if (auto* obj_array_widget = qobject_cast<ObjectArrayWidget*>(widget))
    {
        connect(obj_array_widget, &ObjectArrayWidget::valuesChanged, this, &FormGenerator::onFieldChanged);
    }
}

void FormGenerator::addSimpleFieldToForm(const QString& field_name, const json& field_schema)
{
    // Create input widget
    QWidget* widget = nullptr;
    const bool is_string_type = field_schema.contains("type") && field_schema["type"].is_string() && field_schema["type"].get<std::string>() == "string";
    auto looks_path_like = [&](const QString& name, const json& schema) -> bool {
        const QString n = name.toLower();
        if (n.contains("file") || n.contains("configpath") || n.contains("path") || n.contains("dir") || n.contains("folder"))
        {
            return true;
        }
        if (schema.contains("pattern") && schema["pattern"].is_string())
        {
            const std::string p = schema["pattern"].get<std::string>();
            if (p.find(".json") != std::string::npos || p.find(".csv") != std::string::npos)
            {
                return true;
            }
        }
        return false;
    };

    if (is_string_type && looks_path_like(field_name, field_schema))
    {
        widget = new PathSelectorWidget(field_schema, field_name);
    }
    else
    {
        WidgetFactory factory;
        widget = factory.createWidget(field_schema);
    }

    // Skip if widget creation returned empty widget or nullptr
    if (widget == nullptr)
    {
        return;
    }

    // Check if it's just an empty QWidget (used for Object/Unknown types we can't handle)
    auto* empty_widget = qobject_cast<QWidget*>(widget);
    if (empty_widget && !qobject_cast<QLineEdit*>(widget) &&
        !qobject_cast<QSpinBox*>(widget) &&
        !qobject_cast<QDoubleSpinBox*>(widget) &&
        !qobject_cast<QCheckBox*>(widget) &&
        !qobject_cast<QComboBox*>(widget))
    {
        // It's a generic empty QWidget for unsupported types
        widget->deleteLater();
        return;
    }

    // Create horizontal layout for label and widget
    auto* h_layout = new QHBoxLayout();

    // Add label
    auto* label = new QLabel(field_name + ":");
    label->setMinimumWidth(150);
    h_layout->addWidget(label);

    h_layout->addWidget(widget);
    h_layout->addStretch();

    layout_->addLayout(h_layout);

    // Track field widget
    FieldWidget fw{widget, field_name, field_schema};
    field_widgets_[field_name] = fw;

    // Connect change signals
    if (auto* line_edit = qobject_cast<QLineEdit*>(widget))
    {
        connect(line_edit, &QLineEdit::textChanged, this, &FormGenerator::onFieldChanged);
    }
    else if (auto* path_widget = qobject_cast<PathSelectorWidget*>(widget))
    {
        connect(path_widget, &PathSelectorWidget::pathChanged, this, &FormGenerator::onFieldChanged);
    }
    else if (auto* spin_box = qobject_cast<QSpinBox*>(widget))
    {
        connect(spin_box, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &FormGenerator::onFieldChanged);
    }
    else if (auto* double_spin = qobject_cast<QDoubleSpinBox*>(widget))
    {
        connect(double_spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &FormGenerator::onFieldChanged);
    }
    else if (auto* check_box = qobject_cast<QCheckBox*>(widget))
    {
        connect(check_box, &QCheckBox::stateChanged, this, &FormGenerator::onFieldChanged);
    }
    else if (auto* combo_box = qobject_cast<QComboBox*>(widget))
    {
        connect(combo_box, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &FormGenerator::onFieldChanged);
    }
}

json FormGenerator::getFormData() const
{
    json data = json::object();

    // If we have a schema with nested objects, reconstruct that structure
    if (schema_.contains("properties"))
    {
        const auto& properties = schema_["properties"];
        
        for (auto it = properties.begin(); it != properties.end(); ++it)
        {
            const QString section_name = QString::fromStdString(it.key());
            const json& section_schema = it.value();

            // Check if this is a nested object
            if (section_schema.is_object() && section_schema.contains("properties") &&
                section_schema["type"].is_string() && section_schema["type"].get<std::string>() == "object")
            {
                // Create a nested object
                json nested_obj = json::object();
                const auto& nested_properties = section_schema["properties"];

                for (auto nested_it = nested_properties.begin(); nested_it != nested_properties.end(); ++nested_it)
                {
                    const QString nested_field_name = QString::fromStdString(nested_it.key());
                    
                    // Get value from field widget
                    if (field_widgets_.contains(nested_field_name))
                    {
                        const FieldWidget& fw = field_widgets_[nested_field_name];

                        if (auto* line_edit = qobject_cast<QLineEdit*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = line_edit->text().toStdString();
                        }
                        else if (auto* path_widget = qobject_cast<PathSelectorWidget*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = path_widget->text().toStdString();
                        }
                        else if (auto* combo_box = qobject_cast<QComboBox*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = combo_box->currentText().toStdString();
                        }
                        else if (auto* spin_box = qobject_cast<QSpinBox*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = spin_box->value();
                        }
                        else if (auto* double_spin = qobject_cast<QDoubleSpinBox*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = double_spin->value();
                        }
                        else if (auto* check_box = qobject_cast<QCheckBox*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = check_box->isChecked();
                        }
                        else if (auto* array_widget = qobject_cast<ArrayWidget*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = array_widget->getValues();
                        }
                        else if (auto* range_widget = qobject_cast<RangeWidget*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = range_widget->getValues();
                        }
                        else if (auto* dict_widget = qobject_cast<DictionaryWidget*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = dict_widget->getValues();
                        }
                        else if (auto* obj_array_widget = qobject_cast<ObjectArrayWidget*>(fw.widget))
                        {
                            nested_obj[nested_field_name.toStdString()] = obj_array_widget->getValues();
                        }
                    }
                }

                data[section_name.toStdString()] = nested_obj;
            }
            else
            {
                // Simple field at top level
                if (field_widgets_.contains(section_name))
                {
                    const FieldWidget& fw = field_widgets_[section_name];

                    if (auto* line_edit = qobject_cast<QLineEdit*>(fw.widget))
                    {
                        data[section_name.toStdString()] = line_edit->text().toStdString();
                    }
                    else if (auto* path_widget = qobject_cast<PathSelectorWidget*>(fw.widget))
                    {
                        data[section_name.toStdString()] = path_widget->text().toStdString();
                    }
                    else if (auto* combo_box = qobject_cast<QComboBox*>(fw.widget))
                    {
                        data[section_name.toStdString()] = combo_box->currentText().toStdString();
                    }
                    else if (auto* spin_box = qobject_cast<QSpinBox*>(fw.widget))
                    {
                        data[section_name.toStdString()] = spin_box->value();
                    }
                    else if (auto* double_spin = qobject_cast<QDoubleSpinBox*>(fw.widget))
                    {
                        data[section_name.toStdString()] = double_spin->value();
                    }
                    else if (auto* check_box = qobject_cast<QCheckBox*>(fw.widget))
                    {
                        data[section_name.toStdString()] = check_box->isChecked();
                    }
                    else if (auto* array_widget = qobject_cast<ArrayWidget*>(fw.widget))
                    {
                        data[section_name.toStdString()] = array_widget->getValues();
                    }
                    else if (auto* range_widget = qobject_cast<RangeWidget*>(fw.widget))
                    {
                        data[section_name.toStdString()] = range_widget->getValues();
                    }
                    else if (auto* dict_widget = qobject_cast<DictionaryWidget*>(fw.widget))
                    {
                        data[section_name.toStdString()] = dict_widget->getValues();
                    }
                    else if (auto* obj_array_widget = qobject_cast<ObjectArrayWidget*>(fw.widget))
                    {
                        data[section_name.toStdString()] = obj_array_widget->getValues();
                    }
                }
            }
        }
    }
    else
    {
        // Fallback to flat structure
        for (auto it = field_widgets_.begin(); it != field_widgets_.end(); ++it)
        {
            const QString& field_name = it.key();
            const FieldWidget& fw = it.value();

            if (auto* line_edit = qobject_cast<QLineEdit*>(fw.widget))
            {
                data[field_name.toStdString()] = line_edit->text().toStdString();
            }
            else if (auto* path_widget = qobject_cast<PathSelectorWidget*>(fw.widget))
            {
                data[field_name.toStdString()] = path_widget->text().toStdString();
            }
            else if (auto* combo_box = qobject_cast<QComboBox*>(fw.widget))
            {
                data[field_name.toStdString()] = combo_box->currentText().toStdString();
            }
            else if (auto* spin_box = qobject_cast<QSpinBox*>(fw.widget))
            {
                data[field_name.toStdString()] = spin_box->value();
            }
            else if (auto* double_spin = qobject_cast<QDoubleSpinBox*>(fw.widget))
            {
                data[field_name.toStdString()] = double_spin->value();
            }
            else if (auto* check_box = qobject_cast<QCheckBox*>(fw.widget))
            {
                data[field_name.toStdString()] = check_box->isChecked();
            }
            else if (auto* array_widget = qobject_cast<ArrayWidget*>(fw.widget))
            {
                data[field_name.toStdString()] = array_widget->getValues();
            }
            else if (auto* range_widget = qobject_cast<RangeWidget*>(fw.widget))
            {
                data[field_name.toStdString()] = range_widget->getValues();
            }
            else if (auto* dict_widget = qobject_cast<DictionaryWidget*>(fw.widget))
            {
                data[field_name.toStdString()] = dict_widget->getValues();
            }
            else if (auto* obj_array_widget = qobject_cast<ObjectArrayWidget*>(fw.widget))
            {
                data[field_name.toStdString()] = obj_array_widget->getValues();
            }
        }
    }

    return data;
}

void FormGenerator::setFormData(const json& data)
{
    if (!data.is_object())
    {
        return;
    }

    // Recursively apply values at any nesting level
    applyDataRecursive(data);

    is_dirty_ = false;
}

void FormGenerator::applyDataRecursive(const json& obj)
{
    for (auto it = obj.begin(); it != obj.end(); ++it)
    {
        const std::string key = it.key();
        json value = it.value();  // Make a copy so we can modify if needed

        // DEBUG: Log what we're processing
        std::cerr << "[FormGenerator::applyDataRecursive] Processing key: " << key 
                  << ", is_object: " << value.is_object() 
                  << ", is_array: " << value.is_array() << std::endl;

        // Special handling for "rules" field: convert shorthand dictionary format to array format
        if (key == "rules" && value.is_object() && !value.is_array())
        {
            // Check if this looks like the shorthand dictionary format (keys are field names)
            // vs. a real nested object
            bool is_shorthand_format = false;
            for (auto rule_it = value.begin(); rule_it != value.end(); ++rule_it)
            {
                // If the value is a string (shorthand) or object (inline rule), it's shorthand format
                if (rule_it.value().is_string() || 
                    (rule_it.value().is_object() && (rule_it.value().contains("type") || rule_it.value().contains("allowEmpty"))))
                {
                    is_shorthand_format = true;
                    break;
                }
            }
            
            if (is_shorthand_format)
            {
                std::cerr << "[FormGenerator::applyDataRecursive] Converting rules from shorthand dictionary format to array" << std::endl;
                // Convert shorthand dictionary format to array format
                value = RuleParser::convertNewFormatToOld(value);
                std::cerr << "[FormGenerator::applyDataRecursive] Converted rules, new size: " << (value.is_array() ? value.size() : 0) << std::endl;
            }
        }

        if (value.is_object())
        {
            // Keep descending until we reach leaf values (non-object)
            std::cerr << "[FormGenerator::applyDataRecursive] Recursing into object: " << key << std::endl;
            applyDataRecursive(value);
        }
        else
        {
            std::cerr << "[FormGenerator::applyDataRecursive] Calling updateFieldValue for key: " << key << std::endl;
            updateFieldValue(QString::fromStdString(key), value);
        }
    }
}

void FormGenerator::updateFieldValue(const QString& field_name, const json& value)
{
    std::cerr << "[FormGenerator::updateFieldValue] Attempting to update field: " << field_name.toStdString() 
              << ", field exists in field_widgets_: " << (field_widgets_.contains(field_name) ? "YES" : "NO") << std::endl;
    
    if (!field_widgets_.contains(field_name))
    {
        std::cerr << "[FormGenerator::updateFieldValue] Field not found, available fields: ";
        for (auto it = field_widgets_.begin(); it != field_widgets_.end(); ++it)
        {
            std::cerr << it.key().toStdString() << " ";
        }
        std::cerr << std::endl;
        return;
    }

    const FieldWidget& fw = field_widgets_[field_name];

    if (auto* line_edit = qobject_cast<QLineEdit*>(fw.widget))
    {
        if (value.is_string())
        {
            line_edit->setText(QString::fromStdString(value.get<std::string>()));
        }
    }
    else if (auto* path_widget = qobject_cast<PathSelectorWidget*>(fw.widget))
    {
        if (value.is_string())
        {
            path_widget->setText(QString::fromStdString(value.get<std::string>()));
        }
    }
    else if (auto* combo_box = qobject_cast<QComboBox*>(fw.widget))
    {
        if (value.is_string())
        {
            const QString text = QString::fromStdString(value.get<std::string>());
            int idx = combo_box->findText(text);
            if (idx >= 0)
            {
                combo_box->setCurrentIndex(idx);
            }
        }
    }
    else if (auto* spin_box = qobject_cast<QSpinBox*>(fw.widget))
    {
        if (value.is_number_integer())
        {
            spin_box->setValue(value.get<int>());
        }
    }
    else if (auto* double_spin = qobject_cast<QDoubleSpinBox*>(fw.widget))
    {
        if (value.is_number())
        {
            double_spin->setValue(value.get<double>());
        }
    }
    else if (auto* check_box = qobject_cast<QCheckBox*>(fw.widget))
    {
        if (value.is_boolean())
        {
            check_box->setChecked(value.get<bool>());
        }
    }
    else if (auto* array_widget = qobject_cast<ArrayWidget*>(fw.widget))
    {
        if (value.is_array())
        {
            array_widget->setValues(value);
        }
    }
    else if (auto* range_widget = qobject_cast<RangeWidget*>(fw.widget))
    {
        if (value.is_array())
        {
            range_widget->setValues(value);
        }
    }
    else if (auto* dict_widget = qobject_cast<DictionaryWidget*>(fw.widget))
    {
        if (value.is_object())
        {
            dict_widget->setValues(value);
        }
    }
    else if (auto* obj_array_widget = qobject_cast<ObjectArrayWidget*>(fw.widget))
    {
        if (value.is_array())
        {
            std::cerr << "[FormGenerator::updateFieldValue] Setting ObjectArrayWidget values for field: " 
                      << field_name.toStdString() << ", array size: " << value.size() << std::endl;
            obj_array_widget->setValues(value);
        }
    }
}

void FormGenerator::clearForm()
{
    // Delete all widgets
    QLayoutItem* item;
    while ((item = layout_->takeAt(0)) != nullptr)
    {
        if (QWidget* widget = item->widget())
        {
            delete widget;
        }
        delete item;
    }

    field_widgets_.clear();
    is_dirty_ = false;
    
    // Force layout update
    update();
    updateGeometry();
}

void FormGenerator::onFieldChanged()
{
    is_dirty_ = true;
    emit formModified();
}

} // namespace ui
} // namespace configgui
