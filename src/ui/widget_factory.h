// SPDX-License-Identifier: MIT
// WidgetFactory - Creates UI widgets from schema types

#pragma once

#include <nlohmann/json.hpp>
#include <QWidget>
#include <memory>
#include <QString>

using json = nlohmann::ordered_json;

class QLabel;

namespace configgui {
namespace ui {

enum class FieldType
{
    String,
    Integer,
    Number,
    Boolean,
    Array,
    Object,
    Unknown
};

/// @brief Factory for creating widgets from JSON schema fields
class WidgetFactory
{
public:
    WidgetFactory() = default;
    ~WidgetFactory() = default;

    // Non-copyable
    WidgetFactory(const WidgetFactory&) = delete;
    WidgetFactory& operator=(const WidgetFactory&) = delete;

    /// @brief Create widget from schema property
    [[nodiscard]] QWidget* createWidget(const json& schema, QWidget* parent = nullptr);

    /// @brief Get field type from schema
    [[nodiscard]] FieldType getFieldType(const json& schema) const;

private:
    [[nodiscard]] QWidget* createStringWidget(const json& schema, QWidget* parent);
    [[nodiscard]] QWidget* createIntegerWidget(const json& schema, QWidget* parent);
    [[nodiscard]] QWidget* createNumberWidget(const json& schema, QWidget* parent);
    [[nodiscard]] QWidget* createBooleanWidget(const json& schema, QWidget* parent);
    [[nodiscard]] QWidget* createEnumWidget(const json& schema, QWidget* parent);
};

} // namespace ui
} // namespace configgui
