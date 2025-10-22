// SPDX-License-Identifier: MIT
// ObjectArrayWidget - Header for handling arrays of complex objects

#ifndef CONFIGGUI_UI_OBJECT_ARRAY_WIDGET_H
#define CONFIGGUI_UI_OBJECT_ARRAY_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace configgui {
namespace ui {

/**
 * @class ObjectArrayWidget
 * @brief Widget for handling arrays of complex objects
 *
 * Displays array items where each item is a complex object (not just simple types).
 * Each object in the array is shown as a collapsible section with its own form fields.
 */
class ObjectArrayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectArrayWidget(const json& schema, QWidget* parent = nullptr);
    ~ObjectArrayWidget() override = default;

    /**
     * @brief Get current array values
     * @return JSON array with current objects
     */
    json getValues() const;

    /**
     * @brief Set array values
     * @param values JSON array
     */
    void setValues(const json& values);

signals:
    void valuesChanged();

private slots:
    void onAddItem();
    void onRemoveItem();

private:
    void addItemWidget(const json& item_data = json::object());
    void updateAddRemoveButtons();
    QWidget* createObjectFormWidget(const json& item_schema, const json& item_data);

    json schema_;
    QVBoxLayout* items_layout_;
    QWidget* buttons_container_;
    QPushButton* add_button_;
    QPushButton* remove_button_;
};

} // namespace ui
} // namespace configgui

#endif // CONFIGGUI_UI_OBJECT_ARRAY_WIDGET_H
