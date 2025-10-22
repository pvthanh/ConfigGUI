// SPDX-License-Identifier: MIT
// DictionaryWidget - Implementation

#include "dictionary_widget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>

namespace configgui {
namespace ui {

DictionaryWidget::DictionaryWidget(const json& schema, QWidget* parent)
    : QWidget(parent)
    , schema_(schema)
{
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(5);
    main_layout->setContentsMargins(0, 0, 0, 0);

    // Container for key-value items
    items_layout_ = new QVBoxLayout();
    items_layout_->setSpacing(5);
    
    auto* items_widget = new QWidget();
    items_widget->setLayout(items_layout_);
    
    // Add scroll area for many items
    auto* scroll = new QScrollArea();
    scroll->setWidget(items_widget);
    scroll->setWidgetResizable(true);
    scroll->setMaximumHeight(300);
    main_layout->addWidget(scroll);

    // Add buttons container
    buttons_container_ = new QWidget();
    auto* button_layout = new QHBoxLayout(buttons_container_);
    button_layout->setSpacing(5);
    button_layout->setContentsMargins(0, 0, 0, 0);

    add_button_ = new QPushButton("+");
    add_button_->setMaximumWidth(40);
    add_button_->setToolTip("Add new key-value pair");
    connect(add_button_, &QPushButton::clicked, this, &DictionaryWidget::onAddItem);

    remove_button_ = new QPushButton("-");
    remove_button_->setMaximumWidth(40);
    remove_button_->setToolTip("Remove last key-value pair");
    connect(remove_button_, &QPushButton::clicked, this, &DictionaryWidget::onRemoveItem);

    button_layout->addWidget(add_button_);
    button_layout->addWidget(remove_button_);
    button_layout->addStretch();

    main_layout->addWidget(buttons_container_);

    // Initialize with default values if available
    if (schema_.contains("default") && schema_["default"].is_object())
    {
        for (auto it = schema_["default"].begin(); it != schema_["default"].end(); ++it)
        {
            QString key = QString::fromStdString(it.key());
            QString value;
            if (it.value().is_string())
            {
                value = QString::fromStdString(it.value().get<std::string>());
            }
            addItemWidget(key, value);
        }
    }

    updateAddRemoveButtons();
}

void DictionaryWidget::addItemWidget(const QString& key, const QString& value)
{
    auto* item_container = new QWidget();
    auto* item_layout = new QHBoxLayout(item_container);
    item_layout->setSpacing(5);
    item_layout->setContentsMargins(0, 0, 0, 0);

    // Key input field
    auto* key_edit = new QLineEdit();
    key_edit->setText(key);
    key_edit->setPlaceholderText("Key...");
    key_edit->setMaximumWidth(150);

    // Value input field
    auto* value_edit = new QLineEdit();
    value_edit->setText(value);
    value_edit->setPlaceholderText("Value...");

    item_layout->addWidget(new QLabel("Key:"), 0);
    item_layout->addWidget(key_edit, 1);
    item_layout->addWidget(new QLabel("Value:"), 0);
    item_layout->addWidget(value_edit, 1);

    items_layout_->addWidget(item_container);

    // Connect for change signals
    connect(key_edit, &QLineEdit::textChanged, this, &DictionaryWidget::valuesChanged);
    connect(value_edit, &QLineEdit::textChanged, this, &DictionaryWidget::valuesChanged);

    updateAddRemoveButtons();
}

void DictionaryWidget::onAddItem()
{
    addItemWidget();
    emit valuesChanged();
}

void DictionaryWidget::onRemoveItem()
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

void DictionaryWidget::updateAddRemoveButtons()
{
    int count = items_layout_->count();
    remove_button_->setEnabled(count > 0);
}

json DictionaryWidget::getValues() const
{
    json result = json::object();

    // Collect all key-value pairs from the items
    for (int i = 0; i < items_layout_->count(); ++i)
    {
        QWidget* item_widget = items_layout_->itemAt(i)->widget();
        if (!item_widget) continue;

        QHBoxLayout* item_layout = qobject_cast<QHBoxLayout*>(item_widget->layout());
        if (!item_layout) continue;

        // Extract key and value from line edits
        QLineEdit* key_edit = nullptr;
        QLineEdit* value_edit = nullptr;
        int edit_count = 0;

        for (int j = 0; j < item_layout->count(); ++j)
        {
            QLineEdit* edit = qobject_cast<QLineEdit*>(item_layout->itemAt(j)->widget());
            if (edit)
            {
                if (edit_count == 0) key_edit = edit;
                else if (edit_count == 1) value_edit = edit;
                edit_count++;
            }
        }

        if (key_edit && value_edit)
        {
            QString key = key_edit->text();
            QString value = value_edit->text();
            if (!key.isEmpty())
            {
                result[key.toStdString()] = value.toStdString();
            }
        }
    }

    return result;
}

void DictionaryWidget::setValues(const json& values)
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

    // Add items from JSON object
    if (values.is_object())
    {
        for (auto it = values.begin(); it != values.end(); ++it)
        {
            QString key = QString::fromStdString(it.key());
            QString value;
            if (it.value().is_string())
            {
                value = QString::fromStdString(it.value().get<std::string>());
            }
            addItemWidget(key, value);
        }
    }

    updateAddRemoveButtons();
}

} // namespace ui
} // namespace configgui
