// SPDX-License-Identifier: MIT
// Error types enumeration for ConfigGUI

#pragma once

#include <string>

namespace configgui {
namespace core {

/// @brief File I/O error types
enum class FileError
{
    None = 0,
    NotFound,
    PermissionDenied,
    InvalidJson,
    InvalidYaml,
    SchemaMismatch,
    WriteError,
    ParseError,
    UnknownError
};

/// @brief Validation error types (constraint violations)
enum class ValidationErrorType
{
    None = 0,
    Required,
    TypeMismatch,
    MinimumViolation,
    MaximumViolation,
    MinLengthViolation,
    MaxLengthViolation,
    PatternMismatch,
    EnumViolation,
    CustomValidationFailed
};

/// @brief Convert FileError to human-readable string
inline std::string to_string(FileError error)
{
    switch (error)
    {
        case FileError::None:
            return "No error";
        case FileError::NotFound:
            return "File not found";
        case FileError::PermissionDenied:
            return "Permission denied";
        case FileError::InvalidJson:
            return "Invalid JSON format";
        case FileError::InvalidYaml:
            return "Invalid YAML format";
        case FileError::SchemaMismatch:
            return "Configuration does not match schema";
        case FileError::WriteError:
            return "Error writing to file";
        case FileError::ParseError:
            return "Error parsing file";
        case FileError::UnknownError:
            return "Unknown error";
        default:
            return "Unhandled error";
    }
}

/// @brief Convert ValidationErrorType to human-readable string
inline std::string to_string(ValidationErrorType error)
{
    switch (error)
    {
        case ValidationErrorType::None:
            return "No error";
        case ValidationErrorType::Required:
            return "Field is required";
        case ValidationErrorType::TypeMismatch:
            return "Type mismatch";
        case ValidationErrorType::MinimumViolation:
            return "Value below minimum";
        case ValidationErrorType::MaximumViolation:
            return "Value exceeds maximum";
        case ValidationErrorType::MinLengthViolation:
            return "String too short";
        case ValidationErrorType::MaxLengthViolation:
            return "String too long";
        case ValidationErrorType::PatternMismatch:
            return "Value does not match required pattern";
        case ValidationErrorType::EnumViolation:
            return "Value not in allowed values";
        case ValidationErrorType::CustomValidationFailed:
            return "Custom validation failed";
        default:
            return "Unknown validation error";
    }
}

} // namespace core
} // namespace configgui
