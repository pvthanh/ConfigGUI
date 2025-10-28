/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * json_serializer.cpp
 * Multi-format configuration save feature: JSON serializer implementation
 * Phase 3: User Story 1 - Save Configuration in JSON Format
 */

#include "json_serializer.h"

namespace configgui::core::serializers {

Result<std::string> JsonSerializer::serialize(const SerializationContext& context) noexcept
{
    try {
        // Validate pre-serialization if requested
        if (context.validate_pre_serialization) {
            auto validation = validate_pre_serialization(context);
            if (validation) {
                return Result<std::string>(
                    SerializationError::SCHEMA_VALIDATION_FAILED,
                    *validation);
            }
        }

        // Serialize JSON object to string
        if (context.pretty_print) {
            return Result<std::string>(context.configuration_data.dump(2));
        } else {
            return Result<std::string>(context.configuration_data.dump());
        }
    } catch (const std::exception& e) {
        return Result<std::string>(
            SerializationError::INVALID_JSON,
            std::string("JSON serialization failed: ") + e.what());
    }
}

Result<json> JsonSerializer::deserialize(const std::string& content) noexcept
{
    try {
        if (content.empty()) {
            return Result<json>(
                SerializationError::INVALID_JSON,
                "Cannot deserialize empty content");
        }

        auto parsed = json::parse(content);
        if (!parsed.is_object()) {
            return Result<json>(
                SerializationError::INVALID_JSON,
                "JSON must be an object at root level");
        }

        return Result<json>(parsed);
    } catch (const json::parse_error& e) {
        return Result<json>(
            SerializationError::INVALID_JSON,
            std::string("JSON parse error: ") + e.what());
    } catch (const std::exception& e) {
        return Result<json>(
            SerializationError::UNKNOWN_ERROR,
            std::string("JSON deserialization failed: ") + e.what());
    }
}

std::optional<std::string> JsonSerializer::validate_pre_serialization(
    const SerializationContext& context) const noexcept
{
    try {
        // Check that configuration data is valid JSON object
        if (!context.configuration_data.is_object()) {
            return "Configuration data must be a JSON object";
        }

        // If schema is provided, validate against it
        if (context.schema) {
            // TODO: Implement schema validation using json-schema-validator
            // This will be connected during Phase 2 final integration
            (void)context.schema;
        }

        return std::nullopt; // No validation errors
    } catch (const std::exception&) {
        return "Validation threw unexpected exception";
    }
}

std::string JsonSerializer::get_mime_type() const noexcept
{
    return "application/json";
}

std::string JsonSerializer::get_format_name() const noexcept
{
    return "JSON";
}

FormatType JsonSerializer::get_format_type() const noexcept
{
    return FormatType::JSON;
}

bool JsonSerializer::is_valid_json_object(const json& obj) noexcept
{
    return obj.is_object() && !obj.is_null();
}

} // namespace configgui::core::serializers
