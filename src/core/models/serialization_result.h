/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * serialization_result.h
 * Multi-format configuration save feature: Result type for serialization operations
 * Implements Result<T> error handling pattern used throughout ConfigGUI
 */

#ifndef CONFIGGUI_CORE_MODELS_SERIALIZATION_RESULT_H
#define CONFIGGUI_CORE_MODELS_SERIALIZATION_RESULT_H

#include <string>
#include <variant>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace configgui::core::models {

/**
 * @enum SerializationError
 * Error codes for serialization and deserialization operations
 */
enum class SerializationError {
    SUCCESS = 0,                    ///< Operation completed successfully
    INVALID_FORMAT = 1,             ///< Unknown or unsupported format
    INVALID_JSON = 2,               ///< JSON parsing or generation failed
    INVALID_INI = 3,                ///< INI parsing or generation failed
    SCHEMA_VALIDATION_FAILED = 4,   ///< Configuration violates schema
    FILE_IO_ERROR = 5,              ///< File read/write operation failed
    ENCODING_ERROR = 6,             ///< Character encoding error
    DATA_TYPE_MISMATCH = 7,         ///< Incompatible data type for format
    UNKNOWN_ERROR = 8               ///< Unknown or unexpected error
};

/**
 * Convert error code to human-readable message
 * @param error The error code
 * @return Error message string
 */
std::string error_message(SerializationError error) noexcept;

/**
 * @class Result<T>
 * Type-safe result wrapper for operations that can fail
 * Implements Result pattern used throughout ConfigGUI core library
 *
 * Either contains a successful value or an error code with message
 * Usage: auto result = serialize(...);
 *        if (result) { use(result.value()); }
 *        else { handle_error(result.error_code()); }
 */
template <typename T>
class Result {
public:
    /// Construct success result with value
    explicit Result(const T& value) noexcept
        : _data(value)
        , _error(SerializationError::SUCCESS)
    {
    }

    /// Construct success result with value (rvalue ref)
    explicit Result(T&& value) noexcept
        : _data(std::move(value))
        , _error(SerializationError::SUCCESS)
    {
    }

    /// Construct error result
    Result(SerializationError error, const std::string& message) noexcept
        : _data()
        , _error(error)
        , _error_message(message)
    {
    }

    /// Check if result is successful
    explicit operator bool() const noexcept
    {
        return _error == SerializationError::SUCCESS;
    }

    /// Check if result represents an error
    bool is_error() const noexcept
    {
        return _error != SerializationError::SUCCESS;
    }

    /// Get the success value (undefined if is_error())
    const T& value() const
    {
        if (is_error()) {
            throw std::runtime_error("Cannot get value from error result: " + _error_message);
        }
        return std::get<T>(_data);
    }

    /// Get the success value (move version)
    T& value()
    {
        if (is_error()) {
            throw std::runtime_error("Cannot get value from error result: " + _error_message);
        }
        return std::get<T>(_data);
    }

    /// Get the error code
    SerializationError error_code() const noexcept
    {
        return _error;
    }

    /// Get the error message
    const std::string& error_msg() const noexcept
    {
        return _error_message;
    }

    /// Get the error message or default if none set
    std::string error_msg_or_default() const noexcept
    {
        return _error_message.empty() ? error_message(_error) : _error_message;
    }

private:
    std::variant<T> _data;
    SerializationError _error;
    std::string _error_message;
};

// Specialization for void results (SUCCESS or ERROR only)
template <>
class Result<void> {
public:
    /// Construct success result
    static Result<void> success() noexcept
    {
        return Result<void>(SerializationError::SUCCESS, "");
    }

    /// Construct error result
    static Result<void> error(SerializationError code, const std::string& message = "") noexcept
    {
        return Result<void>(code, message);
    }

    /// Check if result is successful
    explicit operator bool() const noexcept
    {
        return _error == SerializationError::SUCCESS;
    }

    /// Check if result represents an error
    bool is_error() const noexcept
    {
        return _error != SerializationError::SUCCESS;
    }

    /// Get the error code
    SerializationError error_code() const noexcept
    {
        return _error;
    }

    /// Get the error message
    const std::string& error_msg() const noexcept
    {
        return _error_message;
    }

    /// Get the error message or default if none set
    std::string error_msg_or_default() const noexcept
    {
        return _error_message.empty() ? error_message(_error) : _error_message;
    }

private:
    explicit Result(SerializationError error, const std::string& message = "") noexcept
        : _error(error)
        , _error_message(message)
    {
    }

    SerializationError _error;
    std::string _error_message;
};

} // namespace configgui::core::models

#endif // CONFIGGUI_CORE_MODELS_SERIALIZATION_RESULT_H
