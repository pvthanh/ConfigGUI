// SPDX-License-Identifier: MIT
// ArrayWidget - Implementation

#include "array_widget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

namespace configgui {
namespace ui {

ArrayWidget::ArrayWidget(const json& schema, QWidget* parent)
    : QWidget(parent)
    , schema_(schema)
{
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(5);
    main_layout->setContentsMargins(0, 0, 0, 0);

    // Container for array items with scroll capability
    items_layout_ = new QVBoxLayout();
    items_layout_->setSpacing(3);
    
    auto* items_widget = new QWidget();
    items_widget->setLayout(items_layout_);
    main_layout->addWidget(items_widget);

    // Add buttons container
    buttons_container_ = new QWidget();
    auto* button_layout = new QHBoxLayout(buttons_container_);
    button_layout->setSpacing(5);
    button_layout->setContentsMargins(0, 0, 0, 0);

    add_button_ = new QPushButton("+");
    add_button_->setMaximumWidth(40);
    add_button_->setToolTip("Add new item to array");
    connect(add_button_, &QPushButton::clicked, this, &ArrayWidget::onAddItem);

    remove_button_ = new QPushButton("-");
    remove_button_->setMaximumWidth(40);
    remove_button_->setToolTip("Remove last item from array");
    connect(remove_button_, &QPushButton::clicked, this, &ArrayWidget::onRemoveItem);

    button_layout->addWidget(add_button_);
    button_layout->addWidget(remove_button_);
    button_layout->addStretch();

    main_layout->addWidget(buttons_container_);

    // Initialize with default values if available
    if (schema_.contains("default") && schema_["default"].is_array())
    {
        for (const auto& item : schema_["default"])
        {
            if (item.is_string())
            {
                addItemWidget(QString::fromStdString(item.get<std::string>()));
            }
        }
    }

    updateAddRemoveButtons();
}

void ArrayWidget::addItemWidget(const QString& value)
{
    auto* item_container = new QWidget();
    auto* item_layout = new QHBoxLayout(item_container);
    item_layout->setSpacing(5);
    item_layout->setContentsMargins(0, 0, 0, 0);

    // Create appropriate widget based on items schema
    QWidget* item_widget = nullptr;

    if (schema_.contains("items") && schema_["items"].contains("enum"))
    {
        // Enum dropdown
        auto* combo = new QComboBox();
        combo->addItem("");  // Empty option
        
        const auto& enums = schema_["items"]["enum"];
        if (enums.is_array())
        {
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto& enum_item = enums.at(i);
                if (enum_item.is_string())
                {
                    QString text = QString::fromStdString(enum_item.get<std::string>());
                    combo->addItem(text);
                    
                    // Set current value if it matches
                    if (!value.isEmpty() && text == value)
                    {
                        combo->setCurrentIndex(combo->count() - 1);
                    }
                }
            }
        }
        
        item_widget = combo;
    }
    else
    {
        // Plain text edit
        auto* line_edit = new QLineEdit();
        line_edit->setText(value);
        line_edit->setPlaceholderText("Enter value...");
        item_widget = line_edit;
    }

    item_layout->addWidget(item_widget, 1);
    items_layout_->addWidget(item_container);

    connect(item_widget, SIGNAL(currentTextChanged(const QString&)), this, SIGNAL(valuesChanged()));
    connect(item_widget, SIGNAL(textChanged(const QString&)), this, SIGNAL(valuesChanged()));

    updateAddRemoveButtons();
}

void ArrayWidget::onAddItem()
{
    addItemWidget();
    emit valuesChanged();
}

void ArrayWidget::onRemoveItem()
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

void ArrayWidget::updateAddRemoveButtons()
{
    int count = items_layout_->count();
    remove_button_->setEnabled(count > 0);
}

json ArrayWidget::getValues() const
{
    json result = json::array();

    for (int i = 0; i < items_layout_->count(); ++i)
    {
        QWidget* item_widget = items_layout_->itemAt(i)->widget();
        if (!item_widget) continue;

        QString value;

        if (auto* combo = qobject_cast<QComboBox*>(item_widget))
        {
            value = combo->currentText();
        }
        else if (auto* line_edit = qobject_cast<QLineEdit*>(item_widget))
        {
            value = line_edit->text();
        }

        if (!value.isEmpty())
        {
            result.push_back(value.toStdString());
        }
    }

    return result;
}

void ArrayWidget::setValues(const json& values)
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

    // Add new items
    if (values.is_array())
    {
        for (const auto& val : values)
        {
            if (val.is_string())
            {
                addItemWidget(QString::fromStdString(val.get<std::string>()));
            }
        }
    }

    updateAddRemoveButtons();
}

} // namespace ui
} // namespace configgui
