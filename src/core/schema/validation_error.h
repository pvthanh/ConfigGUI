// SPDX-License-Identifier: MIT
// Validation Error - Represents constraint violations

#pragma once

#include "../error_types.h"
#include <string>
#include <vector>

namespace configgui {
namespace core {

/// @brief Represents a single validation error for a field
class ValidationError
{
public:
    explicit ValidationError(const std::string& field = "", ValidationErrorType type = ValidationErrorType::None,
                             const std::string& message = "", const std::string& suggestion = "");

    /// @brief Get the field that failed validation
    [[nodiscard]] const std::string& field() const { return field_; }

    /// @brief Get the error type
    [[nodiscard]] ValidationErrorType type() const { return type_; }

    /// @brief Get the error message
    [[nodiscard]] const std::string& message() const { return message_; }

    /// @brief Get suggested fix
    [[nodiscard]] const std::string& suggestion() const { return suggestion_; }

    /// @brief Get error severity (0 = low, 1 = medium, 2 = high)
    [[nodiscard]] int severity() const;

    /// @brief Convert to string for logging
    [[nodiscard]] std::string to_string() const;

    /// @brief Equality operator
    bool operator==(const ValidationError& other) const;

    /// @brief Inequality operator
    bool operator!=(const ValidationError& other) const;

private:
    std::string field_;
    ValidationErrorType type_;
    std::string message_;
    std::string suggestion_;
};

/// @brief Collection of validation errors
using ValidationErrors = std::vector<ValidationError>;

} // namespace core
} // namespace configgui
