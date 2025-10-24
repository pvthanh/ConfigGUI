// SPDX-License-Identifier: MIT
// Validation Error - Implementation

#include "validation_error.h"

namespace configgui {
namespace core {

ValidationError::ValidationError(const std::string& field, ValidationErrorType type, const std::string& message,
                                 const std::string& suggestion)
    : field_(field)
    , type_(type)
    , message_(message)
    , suggestion_(suggestion)
{
}

int ValidationError::severity() const
{
    switch (type_)
    {
        case ValidationErrorType::Required:
        case ValidationErrorType::TypeMismatch:
            return 2; // High
        case ValidationErrorType::MinimumViolation:
        case ValidationErrorType::MaximumViolation:
        case ValidationErrorType::EnumViolation:
            return 2; // High
        case ValidationErrorType::MinLengthViolation:
        case ValidationErrorType::MaxLengthViolation:
        case ValidationErrorType::PatternMismatch:
            return 1; // Medium
        case ValidationErrorType::CustomValidationFailed:
            return 1; // Medium
        default:
            return 0; // Low
    }
}

std::string ValidationError::to_string() const
{
    std::string result = "ValidationError{field='" + field_ + "', type=" + ::configgui::core::to_string(type_) +
                         ", message='" + message_ + "'}";
    return result;
}

bool ValidationError::operator==(const ValidationError& other) const
{
    return field_ == other.field_ && type_ == other.type_ && message_ == other.message_;
}

bool ValidationError::operator!=(const ValidationError& other) const
{
    return !(*this == other);
}

} // namespace core
} // namespace configgui
