// SPDX-License-Identifier: MIT
// WidgetFactory - Implementation

#include "widget_factory.h"
#include "array_widget.h"
#include "range_widget.h"
#include "dictionary_widget.h"
#include "object_array_widget.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QWidget>

namespace configgui {
namespace ui {

QWidget* WidgetFactory::createWidget(const json& schema, QWidget* parent)
{
    // Check if it's an enum FIRST - enums can be attached to strings, integers, etc.
    if (schema.contains("enum") && schema["enum"].is_array() && schema["enum"].size() > 0)
    {
        return createEnumWidget(schema, parent);
    }

    // Check if it's an object with additionalProperties (key-value map / dictionary)
    if (schema.contains("type") && schema["type"].get<std::string>() == "object")
    {
        if (schema.contains("additionalProperties"))
        {
            return new DictionaryWidget(schema, parent);
        }
    }

    // Check if it's an array
    if (schema.contains("type") && schema["type"].get<std::string>() == "array")
    {
        // Check if it's an array of objects (complex objects)
        if (schema.contains("items") && schema["items"].contains("properties"))
        {
            // Array of objects with properties - use ObjectArrayWidget
            return new ObjectArrayWidget(schema, parent);
        }

        // Check if it's a fixed-size array (minItems === maxItems)
        if (schema.contains("minItems") && schema.contains("maxItems"))
        {
            int min_items = schema["minItems"].get<int>();
            int max_items = schema["maxItems"].get<int>();
            
            // If minItems equals maxItems, it's a fixed-size array (like a range)
            if (min_items == max_items && min_items > 0)
            {
                return new RangeWidget(schema, parent);
            }
        }

        // Variable-size array of simple types (strings, enums, etc.)
        return new ArrayWidget(schema, parent);
    }

    const FieldType field_type = getFieldType(schema);

    switch (field_type)
    {
        case FieldType::String:
            return createStringWidget(schema, parent);
        case FieldType::Integer:
            return createIntegerWidget(schema, parent);
        case FieldType::Number:
            return createNumberWidget(schema, parent);
        case FieldType::Boolean:
            return createBooleanWidget(schema, parent);
        case FieldType::Array:
            // Use ArrayWidget for all variable-size arrays - it handles add/remove with proper UI
            return new ArrayWidget(schema, parent);
        default:
            // For Object and Unknown types, return nullptr to skip
            return nullptr;
    }
}

FieldType WidgetFactory::getFieldType(const json& schema) const
{
    // Note: Enum handling is done in createWidget() before calling getFieldType()
    // This allows enums to be treated as ComboBox regardless of their base type

    // If no type field, try to infer or return Unknown
    if (!schema.contains("type"))
    {
        // If it has additionalProperties, it's likely an object
        if (schema.contains("additionalProperties"))
        {
            return FieldType::Object;
        }
        // If it has properties, it's an object
        if (schema.contains("properties"))
        {
            return FieldType::Object;
        }
        return FieldType::Unknown;
    }

    const auto type_str = schema["type"].get<std::string>();

    if (type_str == "string")
    {
        return FieldType::String;
    }
    if (type_str == "integer")
    {
        return FieldType::Integer;
    }
    if (type_str == "number")
    {
        return FieldType::Number;
    }
    if (type_str == "boolean")
    {
        return FieldType::Boolean;
    }
    if (type_str == "array")
    {
        return FieldType::Array;
    }
    if (type_str == "object")
    {
        return FieldType::Object;
    }

    return FieldType::Unknown;
}

QWidget* WidgetFactory::createStringWidget(const json& schema, QWidget* parent)
{
    auto* widget = new QLineEdit(parent);
    
    // Use x-hint as placeholder if available, otherwise use examples or default placeholder
    if (schema.contains("x-hint"))
    {
        widget->setPlaceholderText(QString::fromStdString(schema["x-hint"].get<std::string>()));
    }
    else if (schema.contains("examples") && schema["examples"].is_array() && schema["examples"].size() > 0)
    {
        widget->setPlaceholderText(QString::fromStdString(schema["examples"][0].get<std::string>()));
    }
    else
    {
        widget->setPlaceholderText("Enter text...");
    }
    
    // Set default value if available
    if (schema.contains("default"))
    {
        widget->setText(QString::fromStdString(schema["default"].get<std::string>()));
    }

    // Apply constraints
    if (schema.contains("minLength"))
    {
        // Could set validator for min length
    }
    if (schema.contains("maxLength"))
    {
        widget->setMaxLength(schema["maxLength"].get<int>());
    }
    
    // Add tooltip with description or pattern
    if (schema.contains("description"))
    {
        widget->setToolTip(QString::fromStdString(schema["description"].get<std::string>()));
    }
    else if (schema.contains("pattern"))
    {
        widget->setToolTip("Pattern: " + QString::fromStdString(schema["pattern"].get<std::string>()));
    }

    return widget;
}

QWidget* WidgetFactory::createIntegerWidget(const json& schema, QWidget* parent)
{
    auto* widget = new QSpinBox(parent);
    
    // Set default value if available
    if (schema.contains("default"))
    {
        widget->setValue(schema["default"].get<int>());
    }
    else
    {
        widget->setValue(0);
    }

    // Apply minimum constraint (with exclusiveMinimum handling)
    if (schema.contains("exclusiveMinimum"))
    {
        widget->setMinimum(schema["exclusiveMinimum"].get<int>() + 1);
    }
    else if (schema.contains("minimum"))
    {
        widget->setMinimum(schema["minimum"].get<int>());
    }
    else
    {
        widget->setMinimum(INT_MIN);
    }
    
    // Apply maximum constraint
    if (schema.contains("maximum"))
    {
        widget->setMaximum(schema["maximum"].get<int>());
    }
    else
    {
        widget->setMaximum(INT_MAX);
    }
    
    // Add tooltip with description
    if (schema.contains("description"))
    {
        widget->setToolTip(QString::fromStdString(schema["description"].get<std::string>()));
    }
    else if (schema.contains("x-hint"))
    {
        widget->setToolTip(QString::fromStdString(schema["x-hint"].get<std::string>()));
    }

    return widget;
}

QWidget* WidgetFactory::createNumberWidget(const json& schema, QWidget* parent)
{
    auto* widget = new QDoubleSpinBox(parent);
    
    // Set decimal places for better precision
    widget->setDecimals(6);
    
    // Set default value if available
    if (schema.contains("default"))
    {
        widget->setValue(schema["default"].get<double>());
    }
    else
    {
        widget->setValue(0.0);
    }

    // Apply minimum constraint (with exclusiveMinimum handling)
    if (schema.contains("exclusiveMinimum"))
    {
        // For exclusive minimum, set to just above the value
        double min_val = schema["exclusiveMinimum"].get<double>();
        widget->setMinimum(min_val + 0.000001);
    }
    else if (schema.contains("minimum"))
    {
        widget->setMinimum(schema["minimum"].get<double>());
    }
    else
    {
        widget->setMinimum(-999999999.0);
    }
    
    // Apply maximum constraint
    if (schema.contains("maximum"))
    {
        widget->setMaximum(schema["maximum"].get<double>());
    }
    else
    {
        widget->setMaximum(999999999.0);
    }
    
    // Add tooltip with description
    if (schema.contains("description"))
    {
        widget->setToolTip(QString::fromStdString(schema["description"].get<std::string>()));
    }
    else if (schema.contains("x-hint"))
    {
        widget->setToolTip(QString::fromStdString(schema["x-hint"].get<std::string>()));
    }

    return widget;
}

QWidget* WidgetFactory::createBooleanWidget(const json& schema, QWidget* parent)
{
    auto* widget = new QCheckBox(parent);
    
    // Set default value if available
    if (schema.contains("default"))
    {
        widget->setChecked(schema["default"].get<bool>());
    }
    
    // Add tooltip
    if (schema.contains("description"))
    {
        widget->setToolTip(QString::fromStdString(schema["description"].get<std::string>()));
    }
    else if (schema.contains("x-hint"))
    {
        widget->setToolTip(QString::fromStdString(schema["x-hint"].get<std::string>()));
    }
    
    return widget;
}

QWidget* WidgetFactory::createEnumWidget(const json& schema, QWidget* parent)
{
    auto* widget = new QComboBox(parent);

    // Add empty option first
    widget->addItem("");
    
    if (schema.contains("enum"))
    {
        const auto& enums = schema["enum"];
        if (enums.is_array())
        {
            int default_index = 0;
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto& item = enums.at(i);
                if (item.is_string())
                {
                    QString enum_text = QString::fromStdString(item.get<std::string>());
                    widget->addItem(enum_text);
                    
                    // Check if this is the default value
                    if (schema.contains("default") && schema["default"].get<std::string>() == item.get<std::string>())
                    {
                        default_index = static_cast<int>(i) + 1;  // +1 because we added empty option first
                    }
                }
            }
            
            // Set default selection
            if (default_index > 0)
            {
                widget->setCurrentIndex(default_index);
            }
        }
    }
    
    // Add tooltip
    if (schema.contains("description"))
    {
        widget->setToolTip(QString::fromStdString(schema["description"].get<std::string>()));
    }
    else if (schema.contains("x-hint"))
    {
        widget->setToolTip(QString::fromStdString(schema["x-hint"].get<std::string>()));
    }

    return widget;
}

} // namespace ui
} // namespace configgui
