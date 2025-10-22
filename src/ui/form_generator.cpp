// SPDX-License-Identifier: MIT
// FormGenerator - Implementation

#include "form_generator.h"
#include "widget_factory.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace configgui {
namespace ui {

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

    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        const QString field_name = QString::fromStdString(it.key());
        const json& field_schema = it.value();

        addFieldToForm(field_name, field_schema);
    }

    layout_->addStretch();
    return true;
}

void FormGenerator::addFieldToForm(const QString& field_name, const json& field_schema)
{
    // Check if this is a nested object with properties
    if (field_schema.is_object() && field_schema.contains("properties") && 
        field_schema["type"].is_string() && field_schema["type"].get<std::string>() == "object")
    {
        // Create a collapsible section for this object
        auto* section_container = new QWidget();
        auto* section_layout = new QVBoxLayout(section_container);
        section_layout->setSpacing(5);
        section_layout->setContentsMargins(0, 0, 0, 0);

        // Create section header
        auto* section_label = new QLabel(field_name + ":");
        section_label->setStyleSheet("font-weight: bold; margin-top: 10px; margin-bottom: 5px; color: #333333;");
        section_layout->addWidget(section_label);

        // Create inner container with indentation for nested fields
        auto* inner_container = new QWidget();
        auto* inner_layout = new QVBoxLayout(inner_container);
        inner_layout->setSpacing(5);
        inner_layout->setContentsMargins(20, 0, 0, 0);  // Add left margin for indentation
        
        // Add fields from nested object
        const auto& properties = field_schema["properties"];
        int field_count = 0;
        for (auto it = properties.begin(); it != properties.end(); ++it)
        {
            const QString nested_field_name = QString::fromStdString(it.key());
            const json& nested_field_schema = it.value();
            
            // Create nested field widget
            WidgetFactory factory;
            QWidget* widget = factory.createWidget(nested_field_schema);
            
            if (widget != nullptr)
            {
                // Check if it's a real widget type
                auto* empty_widget = qobject_cast<QWidget*>(widget);
                if (empty_widget && !qobject_cast<QLineEdit*>(widget) &&
                    !qobject_cast<QSpinBox*>(widget) &&
                    !qobject_cast<QDoubleSpinBox*>(widget) &&
                    !qobject_cast<QCheckBox*>(widget) &&
                    !qobject_cast<QComboBox*>(widget))
                {
                    // Unsupported type, skip
                    widget->deleteLater();
                    continue;
                }
                
                // Create horizontal layout for nested field
                auto* nested_h_layout = new QHBoxLayout();
                auto* nested_label = new QLabel(nested_field_name + ":");
                nested_label->setMinimumWidth(130);
                nested_h_layout->addWidget(nested_label);
                nested_h_layout->addWidget(widget);
                nested_h_layout->addStretch();
                
                inner_layout->addLayout(nested_h_layout);
                
                // Track the field widget
                FieldWidget fw{widget, nested_field_name, nested_field_schema};
                field_widgets_[nested_field_name] = fw;
                
                // Connect change signals
                if (auto* line_edit = qobject_cast<QLineEdit*>(widget))
                {
                    connect(line_edit, &QLineEdit::textChanged, this, &FormGenerator::onFieldChanged);
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
                
                field_count++;
            }
        }
        
        section_layout->addWidget(inner_container);
        layout_->addWidget(section_container);
        return;
    }

    // Otherwise, add as a simple field
    addSimpleFieldToForm(field_name, field_schema);
}

void FormGenerator::addSimpleFieldToForm(const QString& field_name, const json& field_schema)
{
    // Create input widget
    WidgetFactory factory;
    QWidget* widget = factory.createWidget(field_schema);

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

    // Handle nested objects (e.g., application, server, database sections)
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        const json& value = it.value();
        
        // If the value is a nested object, recursively update its fields
        if (value.is_object())
        {
            for (auto nested_it = value.begin(); nested_it != value.end(); ++nested_it)
            {
                updateFieldValue(QString::fromStdString(nested_it.key()), nested_it.value());
            }
        }
        else
        {
            // Otherwise update the field directly
            updateFieldValue(QString::fromStdString(it.key()), value);
        }
    }

    is_dirty_ = false;
}

void FormGenerator::updateFieldValue(const QString& field_name, const json& value)
{
    if (!field_widgets_.contains(field_name))
    {
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
