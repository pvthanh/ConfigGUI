// SPDX-License-Identifier: MIT
// RangeWidget - Header for handling fixed-size array fields (like min/max ranges)

#ifndef CONFIGGUI_UI_RANGE_WIDGET_H
#define CONFIGGUI_UI_RANGE_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

namespace configgui {
namespace ui {

/**
 * @class RangeWidget
 * @brief Widget for handling fixed-size array fields (arrays with minItems === maxItems)
 *
 * For arrays that require a fixed number of items (e.g., latitude_range with exactly 2 values),
 * this widget creates that exact number of input fields without add/remove buttons.
 * Useful for ranges, coordinates, RGB values, etc.
 */
class RangeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RangeWidget(const json& schema, QWidget* parent = nullptr);
    ~RangeWidget() override = default;

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

private:
    void createFieldsForFixedSize();
    
    json schema_;
    std::vector<QWidget*> input_fields_;
    QVBoxLayout* main_layout_;
};

} // namespace ui
} // namespace configgui

#endif // CONFIGGUI_UI_RANGE_WIDGET_H
