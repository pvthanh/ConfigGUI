/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * serializer_factory.cpp
 * Multi-format configuration save feature: Serializer factory implementation
 */

#include "serializer_factory.h"
#include "json_serializer.h"
#include "ini_serializer.h"

namespace configgui::core::serializers {

Result<FormatSerializerPtr> SerializerFactory::create_serializer(FormatType format) noexcept
{
    try {
        switch (format) {
        case FormatType::JSON:
            return Result<FormatSerializerPtr>(std::make_unique<JsonSerializer>());

        case FormatType::INI:
            return Result<FormatSerializerPtr>(std::make_unique<IniSerializer>());

        default:
            return Result<FormatSerializerPtr>(
                SerializationError::INVALID_FORMAT,
                "Unsupported format type: " + format_to_string(format));
        }
    } catch (const std::exception& e) {
        return Result<FormatSerializerPtr>(
            SerializationError::UNKNOWN_ERROR,
            std::string("Failed to create serializer: ") + e.what());
    }
}

Result<FormatSerializerPtr> SerializerFactory::create_serializer_from_path(
    const std::string& file_path) noexcept
{
    try {
        auto format_opt = detect_format_from_path(file_path);
        if (!format_opt) {
            return Result<FormatSerializerPtr>(
                SerializationError::INVALID_FORMAT,
                "Cannot detect format from file path: " + file_path);
        }

        return create_serializer(*format_opt);
    } catch (const std::exception& e) {
        return Result<FormatSerializerPtr>(
            SerializationError::UNKNOWN_ERROR,
            std::string("Failed to create serializer from path: ") + e.what());
    }
}

bool SerializerFactory::is_format_supported(FormatType format) noexcept
{
    switch (format) {
    case FormatType::JSON:
    case FormatType::INI:
        return true;
    default:
        return false;
    }
}

} // namespace configgui::core::serializers
