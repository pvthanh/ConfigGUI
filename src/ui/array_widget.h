// SPDX-License-Identifier: MIT
// ArrayWidget - Header for handling array/list fields with dynamic add/remove

#ifndef CONFIGGUI_UI_ARRAY_WIDGET_H
#define CONFIGGUI_UI_ARRAY_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace configgui {
namespace ui {

/**
 * @class ArrayWidget
 * @brief Widget for handling array fields with dynamic add/remove buttons
 *
 * Displays array items as a list of input fields (typically dropdowns for enum arrays)
 * with add (+) and remove (-) buttons to manage array size.
 */
class ArrayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ArrayWidget(const json& schema, QWidget* parent = nullptr);
    ~ArrayWidget() override = default;

    /**
     * @brief Get current array values
     * @return JSON array with current values
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
    void addItemWidget(const QString& value = "");
    void updateAddRemoveButtons();

    json schema_;
    QVBoxLayout* items_layout_;
    QWidget* buttons_container_;
    QPushButton* add_button_;
    QPushButton* remove_button_;
};

} // namespace ui
} // namespace configgui

#endif // CONFIGGUI_UI_ARRAY_WIDGET_H
