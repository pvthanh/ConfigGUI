// SPDX-License-Identifier: MIT
// RangeWidget - Implementation for fixed-size arrays (min/max ranges)

#include "range_widget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

namespace configgui {
namespace ui {

RangeWidget::RangeWidget(const json& schema, QWidget* parent)
    : QWidget(parent)
    , schema_(schema)
{
    main_layout_ = new QVBoxLayout(this);
    main_layout_->setSpacing(5);
    main_layout_->setContentsMargins(0, 0, 0, 0);

    createFieldsForFixedSize();

    // Initialize with default values if available
    if (schema_.contains("default") && schema_["default"].is_array())
    {
        setValues(schema_["default"]);
    }
}

void RangeWidget::createFieldsForFixedSize()
{
    // Determine the fixed size from minItems or maxItems
    int fixed_size = 0;
    
    if (schema_.contains("minItems"))
    {
        fixed_size = schema_["minItems"].get<int>();
    }
    else if (schema_.contains("maxItems"))
    {
        fixed_size = schema_["maxItems"].get<int>();
    }

    // Determine the item type from the items schema
    std::string item_type = "string";
    if (schema_.contains("items") && schema_["items"].contains("type"))
    {
        item_type = schema_["items"]["type"].get<std::string>();
    }

    double min_value = -999999999.0;
    double max_value = 999999999.0;

    // Get min/max constraints from items schema
    if (schema_.contains("items"))
    {
        const auto& items_schema = schema_["items"];
        if (items_schema.contains("minimum"))
        {
            min_value = items_schema["minimum"].get<double>();
        }
        if (items_schema.contains("maximum"))
        {
            max_value = items_schema["maximum"].get<double>();
        }
    }

    // Create exactly 'fixed_size' input fields
    for (int i = 0; i < fixed_size; ++i)
    {
        auto* item_container = new QWidget();
        auto* item_layout = new QHBoxLayout(item_container);
        item_layout->setSpacing(5);
        item_layout->setContentsMargins(0, 0, 0, 0);

        QWidget* item_widget = nullptr;

        // Determine label based on position
        QString label_text;
        if (fixed_size == 2)
        {
            label_text = (i == 0) ? "Min:" : "Max:";
        }
        else
        {
            label_text = QString("[%1]:").arg(i);
        }

        auto* label = new QLabel(label_text);
        label->setMaximumWidth(40);
        item_layout->addWidget(label);

        // Create appropriate input widget
        if (item_type == "integer")
        {
            auto* spin = new QSpinBox();
            spin->setMinimum(static_cast<int>(min_value));
            spin->setMaximum(static_cast<int>(max_value));
            item_widget = spin;
            connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
                    this, &RangeWidget::valuesChanged);
        }
        else if (item_type == "number")
        {
            auto* spin = new QDoubleSpinBox();
            spin->setMinimum(min_value);
            spin->setMaximum(max_value);
            spin->setDecimals(6);
            item_widget = spin;
            connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &RangeWidget::valuesChanged);
        }
        else
        {
            // Default to line edit for strings
            auto* line_edit = new QLineEdit();
            line_edit->setPlaceholderText(QString("Item %1").arg(i));
            item_widget = line_edit;
            connect(line_edit, &QLineEdit::textChanged,
                    this, &RangeWidget::valuesChanged);
        }

        item_layout->addWidget(item_widget, 1);
        item_layout->addStretch();

        main_layout_->addWidget(item_container);
        input_fields_.push_back(item_widget);
    }

    main_layout_->addStretch();
}

json RangeWidget::getValues() const
{
    json values = json::array();

    for (const auto* widget : input_fields_)
    {
        // Try different widget types
        if (auto* spin = qobject_cast<const QSpinBox*>(widget))
        {
            values.push_back(spin->value());
        }
        else if (auto* double_spin = qobject_cast<const QDoubleSpinBox*>(widget))
        {
            values.push_back(double_spin->value());
        }
        else if (auto* line_edit = qobject_cast<const QLineEdit*>(widget))
        {
            values.push_back(line_edit->text().toStdString());
        }
    }

    return values;
}

void RangeWidget::setValues(const json& values)
{
    if (!values.is_array())
    {
        return;
    }

    for (size_t i = 0; i < input_fields_.size() && i < values.size(); ++i)
    {
        auto* widget = input_fields_[i];
        const auto& value = values[i];

        if (auto* spin = qobject_cast<QSpinBox*>(widget))
        {
            if (value.is_number())
            {
                spin->setValue(value.get<int>());
            }
        }
        else if (auto* double_spin = qobject_cast<QDoubleSpinBox*>(widget))
        {
            if (value.is_number())
            {
                double_spin->setValue(value.get<double>());
            }
        }
        else if (auto* line_edit = qobject_cast<QLineEdit*>(widget))
        {
            if (value.is_string())
            {
                line_edit->setText(QString::fromStdString(value.get<std::string>()));
            }
        }
    }
}

} // namespace ui
} // namespace configgui
