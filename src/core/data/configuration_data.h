// SPDX-License-Identifier: MIT
// ConfigurationData - User configuration values with validation state

#pragma once

#include "../schema/validation_error.h"
#include <nlohmann/json.hpp>
#include <map>
#include <string>

using json = nlohmann::ordered_json;

namespace configgui {
namespace core {

/// @brief Tracks validation state of a single field
struct FieldState
{
    bool is_dirty = false;                    ///< User has edited this field
    bool is_focused = false;                  ///< Field currently has focus
    std::vector<ValidationError> errors;      ///< Validation errors for this field
    json current_value;                       ///< Current user input value
};

/// @brief Represents configuration data with validation tracking
class ConfigurationData
{
public:
    /// @brief Create empty configuration
    ConfigurationData() = default;

    /// @brief Create configuration from JSON
    explicit ConfigurationData(const json& data);

    /// @brief Get underlying JSON data
    [[nodiscard]] const json& data() const { return data_; }

    /// @brief Get mutable JSON data
    json& data() { return data_; }

    /// @brief Get value for a field
    [[nodiscard]] json get_value(const std::string& field_name) const;

    /// @brief Set value for a field (marks as dirty)
    void set_value(const std::string& field_name, const json& value);

    /// @brief Mark field as dirty (edited)
    void mark_dirty(const std::string& field_name);

    /// @brief Mark field as clean (not edited)
    void mark_clean(const std::string& field_name);

    /// @brief Check if field has been edited
    [[nodiscard]] bool is_dirty(const std::string& field_name) const;

    /// @brief Mark field as focused
    void set_focused(const std::string& field_name, bool focused);

    /// @brief Add validation error for field
    void add_error(const std::string& field_name, const ValidationError& error);

    /// @brief Clear errors for field
    void clear_errors(const std::string& field_name);

    /// @brief Get all errors for field
    [[nodiscard]] const std::vector<ValidationError>& get_errors(const std::string& field_name) const;

    /// @brief Get all validation errors
    [[nodiscard]] ValidationErrors all_errors() const;

    /// @brief Check if any field has errors
    [[nodiscard]] bool has_errors() const;

    /// @brief Check if specific field has errors
    [[nodiscard]] bool has_errors(const std::string& field_name) const;

    /// @brief Check if form is dirty (any field edited)
    [[nodiscard]] bool is_dirty() const;

    /// @brief Reset form to clean state
    void reset();

    /// @brief Convert to JSON string (for saving)
    [[nodiscard]] std::string to_json_string() const;

private:
    json data_;
    std::map<std::string, FieldState> field_states_;
    static const std::vector<ValidationError> empty_errors_;
};

} // namespace core
} // namespace configgui
