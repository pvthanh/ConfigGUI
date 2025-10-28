/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * serialization_context.h
 * Multi-format configuration save feature: Serialization context
 * Contains data and metadata for serialization operations
 */

#ifndef CONFIGGUI_CORE_MODELS_SERIALIZATION_CONTEXT_H
#define CONFIGGUI_CORE_MODELS_SERIALIZATION_CONTEXT_H

#include "format_type.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>

using json = nlohmann::json;

namespace configgui::core::models {

/**
 * @struct SerializationContext
 * Container for configuration data and metadata during serialization operations
 *
 * Represents the complete state needed to serialize a configuration:
 * - The configuration data (JSON format)
 * - Target format (JSON or INI)
 * - Optional metadata (schema, file path, etc.)
 * - Serialization options and constraints
 */
struct SerializationContext {
    /// The configuration data to serialize (must be valid JSON)
    json configuration_data;

    /// Target format for serialization
    FormatType target_format;

    /// Optional file path for format detection and file operations
    std::optional<std::string> file_path;

    /// Optional schema for validation during serialization
    std::optional<json> schema;

    /// Whether to pretty-print the output
    bool pretty_print = true;

    /// Whether to perform pre-serialization validation
    bool validate_pre_serialization = true;

    /**
     * Create a serialization context for JSON format
     * @param data The configuration data
     * @param path Optional file path
     * @return SerializationContext configured for JSON
     */
    static SerializationContext for_json(const json& data,
                                         const std::optional<std::string>& path = std::nullopt) noexcept
    {
        return {data, FormatType::JSON, path, std::nullopt, true, true};
    }

    /**
     * Create a serialization context for INI format
     * @param data The configuration data
     * @param path Optional file path
     * @return SerializationContext configured for INI
     */
    static SerializationContext for_ini(const json& data,
                                        const std::optional<std::string>& path = std::nullopt) noexcept
    {
        return {data, FormatType::INI, path, std::nullopt, false, true};
    }

    /**
     * Set the schema for validation
     * @param json_schema The JSON Schema to use
     * @return Reference to this context for method chaining
     */
    SerializationContext& with_schema(const json& json_schema) noexcept
    {
        schema = json_schema;
        return *this;
    }

    /**
     * Set pretty-printing option
     * @param enabled Whether to enable pretty printing
     * @return Reference to this context for method chaining
     */
    SerializationContext& with_pretty_print(bool enabled) noexcept
    {
        pretty_print = enabled;
        return *this;
    }

    /**
     * Set pre-serialization validation option
     * @param enabled Whether to perform validation
     * @return Reference to this context for method chaining
     */
    SerializationContext& with_validation(bool enabled) noexcept
    {
        validate_pre_serialization = enabled;
        return *this;
    }
};

} // namespace configgui::core::models

#endif // CONFIGGUI_CORE_MODELS_SERIALIZATION_CONTEXT_H
