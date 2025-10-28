/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * serializer_factory.h
 * Multi-format configuration save feature: Serializer factory
 * Creates and manages FormatSerializer instances for different formats
 */

#ifndef CONFIGGUI_CORE_SERIALIZERS_SERIALIZER_FACTORY_H
#define CONFIGGUI_CORE_SERIALIZERS_SERIALIZER_FACTORY_H

#include "format_serializer.h"
#include "../models/format_type.h"
#include "../models/serialization_result.h"
#include <string>
#include <optional>

namespace configgui::core::serializers {

using namespace models;

/**
 * @class SerializerFactory
 * Factory for creating FormatSerializer instances
 *
 * Provides static methods to create appropriate serializer instances
 * based on format type or file path. Implements the Factory pattern to
 * decouple format selection from serializer creation.
 *
 * Supported formats:
 * - JSON: Uses nlohmann_json for serialization
 * - INI: Uses mINI library with dot-notation flattening for nested objects
 */
class SerializerFactory {
public:
    /// Delete constructor - factory provides only static methods
    SerializerFactory() = delete;
    ~SerializerFactory() = delete;

    /**
     * Create a serializer for the given format type
     *
     * @param format The format type (JSON, INI)
     * @return Unique pointer to a FormatSerializer if supported, error if not
     *
     * @note Returns unique_ptr - caller owns the serializer instance
     * @note Supports JSON and INI formats
     * @note Returns INVALID_FORMAT for unknown formats
     *
     * Example:
     *   auto result = SerializerFactory::create_serializer(FormatType::JSON);
     *   if (result) {
     *       auto serializer = std::move(result.value());
     *       // Use serializer...
     *   }
     */
    static Result<FormatSerializerPtr> create_serializer(FormatType format) noexcept;

    /**
     * Create a serializer by detecting format from file path
     *
     * @param file_path The file path (e.g., "config.json", "/path/to/config.ini")
     * @return Unique pointer to a FormatSerializer if format detected, error otherwise
     *
     * @note Detects format from file extension
     * @note Returns INVALID_FORMAT if extension is not recognized
     * @note Supported extensions: .json, .ini, .cfg, .conf
     *
     * Example:
     *   auto result = SerializerFactory::create_serializer_from_path("config.ini");
     *   if (result) { auto serializer = std::move(result.value()); }
     */
    static Result<FormatSerializerPtr> create_serializer_from_path(
        const std::string& file_path) noexcept;

private:
    /// Internal helper to validate format support
    static bool is_format_supported(FormatType format) noexcept;
};

} // namespace configgui::core::serializers

#endif // CONFIGGUI_CORE_SERIALIZERS_SERIALIZER_FACTORY_H
