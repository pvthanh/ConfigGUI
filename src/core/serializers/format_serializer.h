/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * format_serializer.h
 * Multi-format configuration save feature: Abstract serializer interface
 * Defines the contract for format-specific serialization implementations
 */

#ifndef CONFIGGUI_CORE_SERIALIZERS_FORMAT_SERIALIZER_H
#define CONFIGGUI_CORE_SERIALIZERS_FORMAT_SERIALIZER_H

#include "../models/format_type.h"
#include "../models/serialization_context.h"
#include "../models/serialization_result.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <memory>

using json = nlohmann::json;

namespace configgui::core::serializers {

using namespace models;

/**
 * @class FormatSerializer
 * Abstract base class for format-specific serialization implementations
 *
 * Defines the contract that all format serializers (JSON, INI, etc.) must implement.
 * Provides the interface for:
 * - Serializing JSON configuration to a specific format
 * - Deserializing format-specific content back to JSON
 * - Validating data before serialization
 * - Reporting format capabilities (MIME type, format name)
 *
 * This design follows the Strategy pattern, allowing different serialization
 * algorithms to be selected and used interchangeably.
 */
class FormatSerializer {
public:
    virtual ~FormatSerializer() = default;

    // Prevent copying
    FormatSerializer(const FormatSerializer&) = delete;
    FormatSerializer& operator=(const FormatSerializer&) = delete;

    // Allow moving
    FormatSerializer(FormatSerializer&&) = default;
    FormatSerializer& operator=(FormatSerializer&&) = default;

    /**
     * Serialize configuration data to format-specific content
     *
     * @param context Serialization context containing data, format, and options
     * @return Result containing serialized string if successful, error code otherwise
     *
     * @note Implementations should respect context.pretty_print and context.validate_pre_serialization
     * @note Always return meaningful error messages for debugging
     *
     * Example:
     *   auto serializer = std::make_unique<JsonSerializer>();
     *   auto context = SerializationContext::for_json(data);
     *   auto result = serializer->serialize(context);
     *   if (result) { save_to_file(result.value()); }
     */
    virtual Result<std::string> serialize(const SerializationContext& context) noexcept = 0;

    /**
     * Deserialize format-specific content to JSON
     *
     * @param content The format-specific content to deserialize
     * @return Result containing JSON object if successful, error code otherwise
     *
     * @note Must always return valid JSON on success
     * @note Return INVALID_FORMAT or INVALID_JSON on parse failure
     *
     * Example:
     *   std::string json_string = "{\"key\": \"value\"}";
     *   auto result = serializer->deserialize(json_string);
     *   if (result) { config = result.value(); }
     */
    virtual Result<json> deserialize(const std::string& content) noexcept = 0;

    /**
     * Validate configuration before serialization (optional)
     *
     * @param context Serialization context with data to validate
     * @return Empty optional if valid, error message if validation failed
     *
     * @note This is called automatically during serialize() if context.validate_pre_serialization is true
     * @note Can be overridden for format-specific validation constraints
     *
     * Example:
     *   auto error = serializer->validate_pre_serialization(context);
     *   if (error) { std::cerr << "Validation failed: " << *error << std::endl; }
     */
    virtual std::optional<std::string> validate_pre_serialization(
        const SerializationContext& context) const noexcept = 0;

    /**
     * Get the MIME type for this format
     *
     * @return MIME type string (e.g., "application/json", "text/plain")
     * @note Used for HTTP responses and file type detection
     */
    virtual std::string get_mime_type() const noexcept = 0;

    /**
     * Get human-readable format name
     *
     * @return Format name (e.g., "JSON", "INI")
     * @note Used in UI dialogs and error messages
     */
    virtual std::string get_format_name() const noexcept = 0;

    /**
     * Get the format type enum
     *
     * @return FormatType value for this serializer
     */
    virtual FormatType get_format_type() const noexcept = 0;

protected:
    /// Protected constructor for subclasses
    FormatSerializer() = default;
};

/// Unique pointer type for FormatSerializer instances
using FormatSerializerPtr = std::unique_ptr<FormatSerializer>;

} // namespace configgui::core::serializers

#endif // CONFIGGUI_CORE_SERIALIZERS_FORMAT_SERIALIZER_H
