// SPDX-License-Identifier: MIT
// FormGenerator - Implementation

#include "form_generator.h"
#include "widget_factory.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
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

    const auto& properties = schema["properties"];
    if (!properties.is_object())
    {
        return false;
    }

    WidgetFactory factory;

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
    // Create horizontal layout for label and widget
    auto* h_layout = new QHBoxLayout();

    // Add label
    auto* label = new QLabel(field_name + ":");
    label->setMinimumWidth(150);
    h_layout->addWidget(label);

    // Create input widget
    WidgetFactory factory;
    QWidget* widget = factory.createWidget(field_schema);

    if (widget == nullptr)
    {
        widget = new QWidget();
    }

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
}

json FormGenerator::getFormData() const
{
    json data = json::object();

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

    return data;
}

void FormGenerator::setFormData(const json& data)
{
    if (!data.is_object())
    {
        return;
    }

    for (auto it = data.begin(); it != data.end(); ++it)
    {
        updateFieldValue(QString::fromStdString(it.key()), it.value());
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
}

void FormGenerator::onFieldChanged()
{
    is_dirty_ = true;
    emit formModified();
}

} // namespace ui
} // namespace configgui
