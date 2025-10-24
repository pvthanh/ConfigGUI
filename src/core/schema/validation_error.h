// SPDX-License-Identifier: MIT
// Validation Error - Represents constraint violations

#pragma once

#include "../error_types.h"
#include <QString>
#include <string>
#include <vector>

namespace configgui {
namespace core {

/// @brief Represents a single validation error for a field
class ValidationError
{
public:
    explicit ValidationError(const QString& field = "", ValidationErrorType type = ValidationErrorType::None,
                             const QString& message = "", const QString& suggestion = "");

    /// @brief Get the field that failed validation
    [[nodiscard]] const QString& field() const { return field_; }

    /// @brief Get the error type
    [[nodiscard]] ValidationErrorType type() const { return type_; }

    /// @brief Get the error message
    [[nodiscard]] const QString& message() const { return message_; }

    /// @brief Get suggested fix
    [[nodiscard]] const QString& suggestion() const { return suggestion_; }

    /// @brief Get error severity (0 = low, 1 = medium, 2 = high)
    [[nodiscard]] int severity() const;

    /// @brief Convert to string for logging
    [[nodiscard]] std::string to_string() const;

    /// @brief Equality operator
    bool operator==(const ValidationError& other) const;

    /// @brief Inequality operator
    bool operator!=(const ValidationError& other) const;

private:
    QString field_;
    ValidationErrorType type_;
    QString message_;
    QString suggestion_;
};

/// @brief Collection of validation errors
using ValidationErrors = std::vector<ValidationError>;

} // namespace core
} // namespace configgui
