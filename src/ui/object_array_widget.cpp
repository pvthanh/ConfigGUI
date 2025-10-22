// SPDX-License-Identifier: MIT
// ObjectArrayWidget - Implementation

#include "object_array_widget.h"
#include "widget_factory.h"
#include "rule_editor_widget.h"
#include "rule_parser.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QToolButton>
#include <QScrollArea>
#include <QFormLayout>

namespace configgui {
namespace ui {

ObjectArrayWidget::ObjectArrayWidget(const json& schema, QWidget* parent)
    : QWidget(parent)
    , schema_(schema)
{
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(5);
    main_layout->setContentsMargins(0, 0, 0, 0);

    // Container for array items
    items_layout_ = new QVBoxLayout();
    items_layout_->setSpacing(8);
    
    auto* items_widget = new QWidget();
    items_widget->setLayout(items_layout_);
    
    // Add scroll area for many items
    auto* scroll = new QScrollArea();
    scroll->setWidget(items_widget);
    scroll->setWidgetResizable(true);
    scroll->setMaximumHeight(400);
    main_layout->addWidget(scroll);

    // Add buttons container
    buttons_container_ = new QWidget();
    auto* button_layout = new QHBoxLayout(buttons_container_);
    button_layout->setSpacing(5);
    button_layout->setContentsMargins(0, 0, 0, 0);

    add_button_ = new QPushButton("+");
    add_button_->setMaximumWidth(40);
    add_button_->setToolTip("Add new object to array");
    connect(add_button_, &QPushButton::clicked, this, &ObjectArrayWidget::onAddItem);

    remove_button_ = new QPushButton("-");
    remove_button_->setMaximumWidth(40);
    remove_button_->setToolTip("Remove last object from array");
    connect(remove_button_, &QPushButton::clicked, this, &ObjectArrayWidget::onRemoveItem);

    button_layout->addWidget(add_button_);
    button_layout->addWidget(remove_button_);
    button_layout->addStretch();

    main_layout->addWidget(buttons_container_);

    // Initialize with default or example values if available
    // First try "default", then fall back to "examples"
    if (schema_.contains("default") && schema_["default"].is_array())
    {
        for (const auto& item : schema_["default"])
        {
            addItemWidget(item);
        }
    }
    else if (schema_.contains("examples") && schema_["examples"].is_array())
    {
        // Use examples as initial data if no default
        for (const auto& item : schema_["examples"])
        {
            if (item.is_array() && item.size() > 0)
            {
                // Examples is an array of arrays, take first example array
                for (const auto& example_item : item)
                {
                    if (example_item.is_object())
                    {
                        addItemWidget(example_item);
                    }
                }
                break;  // Only use first example array
            }
        }
    }

    updateAddRemoveButtons();
}

QWidget* ObjectArrayWidget::createObjectFormWidget(const json& /* item_schema */, const json& item_data)
{
    auto* form_widget = new QWidget();
    auto* form_layout = new QFormLayout(form_widget);
    form_layout->setSpacing(8);
    form_layout->setContentsMargins(10, 5, 10, 5);

    // Get the item schema (items definition)
    if (!schema_.contains("items") || !schema_["items"].contains("properties"))
    {
        return form_widget;
    }

    const auto& properties = schema_["items"]["properties"];
    std::vector<std::string> field_order;

    // Determine field order from "required" array if available
    if (schema_["items"].contains("required") && schema_["items"]["required"].is_array())
    {
        for (const auto& field : schema_["items"]["required"])
        {
            if (field.is_string())
            {
                field_order.push_back(field.get<std::string>());
            }
        }
    }
    else
    {
        for (auto it = properties.begin(); it != properties.end(); ++it)
        {
            field_order.push_back(it.key());
        }
    }

    // Check if this is a rule object (has "name" and "type" fields)
    bool is_rule_object = properties.contains("name") && 
                          properties.contains("type") && 
                          properties["type"].contains("enum");
    
    if (is_rule_object && item_data.contains("name") && item_data.contains("type"))
    {
        // This is a rule definition object - use RuleEditorWidget
        std::string rule_name = item_data["name"].is_string() ? 
                                item_data["name"].get<std::string>() : "new_rule";
        
        // Create a shorthand representation of the rule
        // Old format: {"name":"type","type":"string", "allowEmpty":false, "pattern":"...", etc}
        RuleDefinition rule_def;
        rule_def.name = rule_name;
        rule_def.type = item_data.contains("type") && item_data["type"].is_string() ?
                        item_data["type"].get<std::string>() : "string";
        rule_def.required = !( item_data.contains("allowEmpty") && item_data["allowEmpty"].is_boolean() &&
                              item_data["allowEmpty"].get<bool>());
        rule_def.allow_empty = item_data.contains("allowEmpty") && item_data["allowEmpty"].is_boolean() ?
                               item_data["allowEmpty"].get<bool>() : false;
        
        if (item_data.contains("pattern") && item_data["pattern"].is_string())
        {
            rule_def.pattern = item_data["pattern"].get<std::string>();
        }
        
        if (item_data.contains("minimum") && item_data["minimum"].is_number())
        {
            rule_def.minimum = item_data["minimum"].get<double>();
        }
        
        if (item_data.contains("maximum") && item_data["maximum"].is_number())
        {
            rule_def.maximum = item_data["maximum"].get<double>();
        }
        
        if (item_data.contains("enum") && item_data["enum"].is_array())
        {
            for (const auto& enum_val : item_data["enum"])
            {
                if (enum_val.is_string())
                {
                    rule_def.enum_values.push_back(enum_val.get<std::string>());
                }
            }
        }
        
        // Convert to shorthand
        std::string shorthand = RuleParser::toShorthand(rule_def);
        
        auto* rule_editor = new RuleEditorWidget(rule_name, shorthand, form_widget);
        form_layout->addRow(rule_editor);
        
        // The RuleEditorWidget will emit ruleChanged() when modified
        connect(rule_editor, &RuleEditorWidget::ruleChanged, this, &ObjectArrayWidget::valuesChanged);
        
        return form_widget;
    }

    // Standard object handling - create generic form fields
    WidgetFactory factory;
    for (const auto& field_name : field_order)
    {
        if (properties.contains(field_name))
        {
            const auto& field_schema = properties[field_name];
            
            // Create label
            QString label_text = QString::fromStdString(field_name);
            label_text = label_text.at(0).toUpper() + label_text.mid(1);  // Capitalize
            auto* label = new QLabel(label_text + ":");

            // Create widget
            QWidget* widget = factory.createWidget(field_schema);
            if (!widget)
            {
                // If widget factory returns null, use a line edit
                widget = new QLineEdit();
            }

            // Set value from item_data if available
            if (item_data.contains(field_name))
            {
                const auto& value = item_data[field_name];
                if (auto* line_edit = qobject_cast<QLineEdit*>(widget))
                {
                    if (value.is_string())
                    {
                        line_edit->setText(QString::fromStdString(value.get<std::string>()));
                    }
                    else
                    {
                        line_edit->setText(QString::fromStdString(value.dump()));
                    }
                }
                // Could add more type handling here for other widgets
            }

            // Add to form layout
            form_layout->addRow(label, widget);

            // Connect for change signals - use proper signal for each widget type
            if (auto* line_edit = qobject_cast<QLineEdit*>(widget))
            {
                connect(line_edit, &QLineEdit::textChanged, this, &ObjectArrayWidget::valuesChanged);
            }
            else if (auto* combo = qobject_cast<QComboBox*>(widget))
            {
                connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ObjectArrayWidget::valuesChanged);
            }
            else if (auto* check_box = qobject_cast<QCheckBox*>(widget))
            {
                connect(check_box, &QCheckBox::stateChanged, this, &ObjectArrayWidget::valuesChanged);
            }
            else if (auto* spin_box = qobject_cast<QSpinBox*>(widget))
            {
                connect(spin_box, QOverload<int>::of(&QSpinBox::valueChanged), this, &ObjectArrayWidget::valuesChanged);
            }
            else if (auto* double_spin = qobject_cast<QDoubleSpinBox*>(widget))
            {
                connect(double_spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ObjectArrayWidget::valuesChanged);
            }
        }
    }

    return form_widget;
}

void ObjectArrayWidget::addItemWidget(const json& item_data)
{
    auto* item_container = new QGroupBox();
    auto* item_layout = new QVBoxLayout(item_container);
    item_layout->setSpacing(5);

    // Create header with collapse button
    auto* header_container = new QWidget();
    auto* header_layout = new QHBoxLayout(header_container);
    header_layout->setSpacing(5);
    header_layout->setContentsMargins(0, 0, 0, 0);

    auto* toggle_button = new QToolButton();
    toggle_button->setText("▼");  // Expanded initially
    toggle_button->setStyleSheet("QToolButton { border: none; background: none; font-size: 11px; padding: 0px; margin: 0px; }");
    toggle_button->setMaximumWidth(20);

    // Create label showing object index/summary
    QString summary;
    if (item_data.contains("name"))
    {
        summary = QString::fromStdString(item_data["name"].get<std::string>());
    }
    else if (item_data.contains("type"))
    {
        summary = QString::fromStdString(item_data["type"].get<std::string>());
    }
    else
    {
        summary = QString::number(items_layout_->count() + 1);
    }

    auto* header_label = new QLabel("Item: " + summary);
    header_label->setStyleSheet("font-weight: bold;");

    header_layout->addWidget(toggle_button);
    header_layout->addWidget(header_label, 1);
    header_container->setLayout(header_layout);

    // Create form widget for the object
    auto* form_widget = createObjectFormWidget(schema_["items"], item_data);
    form_widget->setStyleSheet("background-color: #f5f5f5; border: 1px solid #ddd; border-radius: 3px; padding: 5px;");

    item_layout->addWidget(header_container);
    item_layout->addWidget(form_widget);

    // Initially collapse the form (show only header)
    form_widget->setVisible(false);
    toggle_button->setText("▶");  // Show collapsed arrow

    // Connect toggle
    QObject::connect(toggle_button, &QToolButton::clicked, [toggle_button, form_widget]()
    {
        bool is_visible = form_widget->isVisible();
        form_widget->setVisible(!is_visible);
        toggle_button->setText(is_visible ? "▶" : "▼");
    });

    items_layout_->addWidget(item_container);
    updateAddRemoveButtons();
}

void ObjectArrayWidget::onAddItem()
{
    addItemWidget();
    emit valuesChanged();
}

void ObjectArrayWidget::onRemoveItem()
{
    // Remove last item
    int count = items_layout_->count();
    if (count > 0)
    {
        QLayoutItem* item = items_layout_->takeAt(count - 1);
        if (item && item->widget())
        {
            item->widget()->deleteLater();
        }
        delete item;
        updateAddRemoveButtons();
        emit valuesChanged();
    }
}

void ObjectArrayWidget::updateAddRemoveButtons()
{
    int count = items_layout_->count();
    remove_button_->setEnabled(count > 0);
}

json ObjectArrayWidget::getValues() const
{
    json result = json::array();

    // Collect all objects from the items
    for (int i = 0; i < items_layout_->count(); ++i)
    {
        auto* item_widget = items_layout_->itemAt(i)->widget();
        if (!item_widget) continue;

        // Find all QLineEdit widgets in this item and collect their values
        json obj = json::object();
        
        // Get field names from schema
        if (schema_.contains("items") && schema_["items"].contains("properties"))
        {
            const auto& properties = schema_["items"]["properties"];
            for (auto it = properties.begin(); it != properties.end(); ++it)
            {
                // For now, extract from form - simplified version
                // In a real implementation, would need to find and extract from specific widgets
                obj[it.key()] = "";
            }
        }

        result.push_back(obj);
    }

    return result;
}

void ObjectArrayWidget::setValues(const json& values)
{
    // Clear existing items
    while (items_layout_->count() > 0)
    {
        QLayoutItem* item = items_layout_->takeAt(0);
        if (item && item->widget())
        {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Add items from JSON array
    if (values.is_array())
    {
        for (const auto& item : values)
        {
            addItemWidget(item);
        }
    }

    updateAddRemoveButtons();
}

} // namespace ui
} // namespace configgui
