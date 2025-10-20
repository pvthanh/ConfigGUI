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
            return createStringWidget(schema, parent);
        case FieldType::Integer:
            return createIntegerWidget(schema, parent);
        case FieldType::Number:
            return createNumberWidget(schema, parent);
        case FieldType::Boolean:
            return createBooleanWidget(schema, parent);
        default:
            return new QWidget(parent);
    }
}

FieldType WidgetFactory::getFieldType(const json& schema) const
{
    if (!schema.contains("type"))
    {
        return FieldType::Unknown;
    }

    const auto type_str = schema["type"].get<std::string>();

    if (type_str == "string")
    {
        if (schema.contains("enum"))
        {
            return FieldType::Array; // Treated as enum
        }
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

QWidget* WidgetFactory::createBooleanWidget(const json& schema, QWidget* parent)
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
