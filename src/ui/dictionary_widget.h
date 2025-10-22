// SPDX-License-Identifier: MIT
// DictionaryWidget - Header for handling key-value map objects

#ifndef CONFIGGUI_UI_DICTIONARY_WIDGET_H
#define CONFIGGUI_UI_DICTIONARY_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace configgui {
namespace ui {

/**
 * @class DictionaryWidget
 * @brief Widget for handling key-value map objects (additionalProperties)
 *
 * Displays key-value pairs as editable rows with add/remove buttons.
 * Typically used for synonym mappings or configuration dictionaries.
 */
class DictionaryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DictionaryWidget(const json& schema, QWidget* parent = nullptr);
    ~DictionaryWidget() override = default;

    /**
     * @brief Get current dictionary values as JSON object
     * @return JSON object with key-value pairs
     */
    json getValues() const;

    /**
     * @brief Set dictionary values
     * @param values JSON object
     */
    void setValues(const json& values);

signals:
    void valuesChanged();

private slots:
    void onAddItem();
    void onRemoveItem();

private:
    void addItemWidget(const QString& key = "", const QString& value = "");
    void updateAddRemoveButtons();

    json schema_;
    QVBoxLayout* items_layout_;
    QWidget* buttons_container_;
    QPushButton* add_button_;
    QPushButton* remove_button_;
};

} // namespace ui
} // namespace configgui

#endif // CONFIGGUI_UI_DICTIONARY_WIDGET_H
