// SPDX-License-Identifier: MIT
// WidgetFactory - Implementation

#include "widget_factory.h"
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
    const FieldType field_type = getFieldType(schema);

    switch (field_type)
    {
        case FieldType::String:
            // Check if it's an enum string
            if (schema.contains("enum"))
            {
                return createEnumWidget(schema, parent);
            }
            return createStringWidget(schema, parent);
        case FieldType::Integer:
            return createIntegerWidget(schema, parent);
        case FieldType::Number:
            return createNumberWidget(schema, parent);
        case FieldType::Boolean:
            return createBooleanWidget(schema, parent);
        case FieldType::Array:
            // Array could be an enum (strings in array) or actual array of items
            if (schema.contains("enum"))
            {
                return createEnumWidget(schema, parent);
            }
            // For non-enum arrays, check if it's an array of simple types
            if (schema.contains("items"))
            {
                const auto& items = schema["items"];
                if (items.contains("enum"))
                {
                    // Array of enums - create combo box for the single selection or text area for multi
                    return createStringWidget(schema, parent);
                }
                if (items.is_object() && items.contains("type"))
                {
                    const auto item_type = items["type"].get<std::string>();
                    if (item_type == "string")
                    {
                        // Array of strings - treat as multi-line text
                        auto* widget = new QLineEdit(parent);
                        widget->setPlaceholderText("Comma-separated values");
                        return widget;
                    }
                }
            }
            // For other arrays, treat as text input with placeholder
            {
                auto* widget = new QLineEdit(parent);
                widget->setPlaceholderText("Array input");
                return widget;
            }
        default:
            // For Object and Unknown types, return nullptr to skip
            return nullptr;
    }
}

FieldType WidgetFactory::getFieldType(const json& schema) const
{
    // First check if it has enum - enums should use ComboBox regardless of type
    if (schema.contains("enum") && schema["enum"].is_array() && schema["enum"].size() > 0)
    {
        return FieldType::Array; // Array type indicates enum/combo box
    }

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
    widget->setPlaceholderText("Enter text...");

    if (schema.contains("minLength"))
    {
        // Could set validator
    }
    if (schema.contains("maxLength"))
    {
        widget->setMaxLength(schema["maxLength"].get<int>());
    }

    return widget;
}

QWidget* WidgetFactory::createIntegerWidget(const json& schema, QWidget* parent)
{
    auto* widget = new QSpinBox(parent);

    if (schema.contains("minimum"))
    {
        widget->setMinimum(schema["minimum"].get<int>());
    }
    if (schema.contains("maximum"))
    {
        widget->setMaximum(schema["maximum"].get<int>());
    }

    return widget;
}

QWidget* WidgetFactory::createNumberWidget(const json& schema, QWidget* parent)
{
    auto* widget = new QDoubleSpinBox(parent);

    if (schema.contains("minimum"))
    {
        widget->setMinimum(schema["minimum"].get<double>());
    }
    if (schema.contains("maximum"))
    {
        widget->setMaximum(schema["maximum"].get<double>());
    }

    return widget;
}

QWidget* WidgetFactory::createBooleanWidget(const json& /*schema*/, QWidget* parent)
{
    return new QCheckBox(parent);
}

QWidget* WidgetFactory::createEnumWidget(const json& schema, QWidget* parent)
{
    auto* widget = new QComboBox(parent);

    if (schema.contains("enum"))
    {
        const auto& enums = schema["enum"];
        if (enums.is_array())
        {
            for (const auto& item : enums)
            {
                if (item.is_string())
                {
                    widget->addItem(QString::fromStdString(item.get<std::string>()));
                }
            }
        }
    }

    return widget;
}

} // namespace ui
} // namespace configgui
