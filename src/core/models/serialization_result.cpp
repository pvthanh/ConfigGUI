/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * serialization_result.cpp
 * Multi-format configuration save feature: Result type implementation
 */

#include "models/serialization_result.h"

namespace configgui::core::models {

std::string error_message(SerializationError error) noexcept
{
    switch (error) {
    case SerializationError::SUCCESS:
        return "Operation completed successfully";
    case SerializationError::INVALID_FORMAT:
        return "Unknown or unsupported format";
    case SerializationError::INVALID_JSON:
        return "JSON parsing or generation failed";
    case SerializationError::INVALID_INI:
        return "INI parsing or generation failed";
    case SerializationError::SCHEMA_VALIDATION_FAILED:
        return "Configuration violates schema";
    case SerializationError::FILE_IO_ERROR:
        return "File read/write operation failed";
    case SerializationError::ENCODING_ERROR:
        return "Character encoding error";
    case SerializationError::DATA_TYPE_MISMATCH:
        return "Incompatible data type for format";
    case SerializationError::UNKNOWN_ERROR:
        return "Unknown or unexpected error";
    }
    return "Unrecognized error code";
}

} // namespace configgui::core::models
