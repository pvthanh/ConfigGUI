/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * json_serializer.h
 * Multi-format configuration save feature: JSON serializer implementation
 * Phase 3: User Story 1 - Save Configuration in JSON Format
 */

#ifndef CONFIGGUI_CORE_SERIALIZERS_JSON_SERIALIZER_H
#define CONFIGGUI_CORE_SERIALIZERS_JSON_SERIALIZER_H

#include "format_serializer.h"

namespace configgui::core::serializers {

/**
 * @class JsonSerializer
 * FormatSerializer implementation for JSON format
 *
 * Handles serialization and deserialization of JSON format using nlohmann_json.
 * Provides JSON-specific validation and formatting options.
 *
 * This implementation:
 * - Serializes JSON objects to formatted JSON strings
 * - Deserializes JSON strings back to JSON objects
 * - Supports pretty-printing with indentation
 * - Validates JSON structure and syntax
 */
class JsonSerializer : public FormatSerializer {
public:
    JsonSerializer() = default;
    ~JsonSerializer() override = default;

    // Prevent copying
    JsonSerializer(const JsonSerializer&) = delete;
    JsonSerializer& operator=(const JsonSerializer&) = delete;

    // Allow moving
    JsonSerializer(JsonSerializer&&) = default;
    JsonSerializer& operator=(JsonSerializer&&) = default;

    /**
     * Serialize JSON data to JSON string
     * @param context Serialization context with JSON data
     * @return Result containing formatted JSON string
     */
    Result<std::string> serialize(const SerializationContext& context) noexcept override;

    /**
     * Deserialize JSON string to JSON object
     * @param content JSON string to parse
     * @return Result containing parsed JSON object
     */
    Result<json> deserialize(const std::string& content) noexcept override;

    /**
     * Validate JSON before serialization
     * @param context Serialization context to validate
     * @return Optional error message if validation fails
     */
    std::optional<std::string> validate_pre_serialization(
        const SerializationContext& context) const noexcept override;

    /**
     * Get MIME type for JSON
     * @return "application/json"
     */
    std::string get_mime_type() const noexcept override;

    /**
     * Get format name
     * @return "JSON"
     */
    std::string get_format_name() const noexcept override;

    /**
     * Get format type
     * @return FormatType::JSON
     */
    FormatType get_format_type() const noexcept override;

private:
    /// Helper to check if JSON object is valid
    static bool is_valid_json_object(const json& obj) noexcept;
};

} // namespace configgui::core::serializers

#endif // CONFIGGUI_CORE_SERIALIZERS_JSON_SERIALIZER_H
