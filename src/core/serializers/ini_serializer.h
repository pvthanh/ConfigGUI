/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * ini_serializer.h
 * Multi-format configuration save feature: INI serializer implementation
 * Phase 4: User Story 2 - Save Configuration in INI Format
 *
 * INI format mapping strategy:
 * - Nested JSON objects are represented using dot-notation keys
 * - JSON arrays are indexed using numeric suffixes (e.g., servers.0, servers.1)
 * - INI sections [section.subsection] preserve hierarchy
 * - Primitive types (string, number, boolean) map to INI values
 * - JSON null values are omitted from INI output
 *
 * Example mapping:
 *   JSON:
 *   {
 *     "database": {
 *       "hosts": [
 *         {"name": "primary", "port": 5432}
 *       ],
 *       "timeout": 30
 *     }
 *   }
 *
 *   INI:
 *   [database.hosts.0]
 *   name=primary
 *   port=5432
 *
 *   [database]
 *   timeout=30
 */

#ifndef CONFIGGUI_CORE_SERIALIZERS_INI_SERIALIZER_H
#define CONFIGGUI_CORE_SERIALIZERS_INI_SERIALIZER_H

#include "format_serializer.h"
#include <map>
#include <vector>

namespace configgui::core::serializers {

/**
 * @class IniSerializer
 * FormatSerializer implementation for INI format
 *
 * Handles serialization and deserialization of INI format using mINI library.
 * Provides JSON-to-INI mapping with dot-notation flattening for nested structures.
 *
 * This implementation:
 * - Flattens nested JSON objects to dot-notation keys
 * - Handles JSON arrays with indexed keys (key.0, key.1, etc.)
 * - Preserves INI format with [section] headers
 * - Supports round-trip conversion: JSON → INI → JSON
 * - Escapes special characters in values
 */
class IniSerializer : public FormatSerializer {
public:
    IniSerializer() = default;
    ~IniSerializer() override = default;

    // Prevent copying
    IniSerializer(const IniSerializer&) = delete;
    IniSerializer& operator=(const IniSerializer&) = delete;

    // Allow moving
    IniSerializer(IniSerializer&&) = default;
    IniSerializer& operator=(IniSerializer&&) = default;

    /**
     * Serialize JSON data to INI string
     * @param context Serialization context with JSON data
     * @return Result containing formatted INI string
     */
    Result<std::string> serialize(const SerializationContext& context) noexcept override;

    /**
     * Deserialize INI string to JSON object
     * @param content INI string to parse
     * @return Result containing reconstructed JSON object
     */
    Result<json> deserialize(const std::string& content) noexcept override;

    /**
     * Validate INI before serialization
     * @param context Serialization context to validate
     * @return Optional error message if validation fails
     */
    std::optional<std::string> validate_pre_serialization(
        const SerializationContext& context) const noexcept override;

    /**
     * Get MIME type for INI
     * @return "text/plain"
     */
    std::string get_mime_type() const noexcept override;

    /**
     * Get format name
     * @return "INI"
     */
    std::string get_format_name() const noexcept override;

    /**
     * Get format type
     * @return FormatType::INI
     */
    FormatType get_format_type() const noexcept override;

private:
    /**
     * Flatten nested JSON object to flat map with dot-notation keys
     *
     * @param obj JSON object to flatten
     * @param prefix Current key prefix (for recursion)
     * @return Flat map of key-value pairs
     *
     * Example:
     *   Input:  {"db": {"host": "localhost", "port": 5432}}
     *   Output: {"db.host": "localhost", "db.port": "5432"}
     */
    static std::map<std::string, std::string> flatten_json(
        const json& obj,
        const std::string& prefix = "") noexcept;

    /**
     * Unflatten flat map back to nested JSON object
     *
     * @param flat_map Flat map with dot-notation keys
     * @return Nested JSON object
     *
     * Example:
     *   Input:  {"db.host": "localhost", "db.port": "5432"}
     *   Output: {"db": {"host": "localhost", "port": 5432}}
     */
    static json unflatten_map(const std::map<std::string, std::string>& flat_map) noexcept;

    /**
     * Convert JSON value to INI-compatible string
     * Handles type conversions and escaping
     *
     * @param val JSON value to convert
     * @return String representation for INI value
     */
    static std::string json_value_to_ini_string(const json& val) noexcept;

    /**
     * Parse INI-format string to value, attempting type reconstruction
     *
     * @param value_str String value from INI
     * @return JSON value (may be string, number, boolean, or null)
     */
    static json ini_string_to_json_value(const std::string& value_str) noexcept;

    /**
     * Escape special characters in INI value
     * Handles newlines, tabs, and other special characters
     *
     * @param str String to escape
     * @return Escaped string
     */
    static std::string escape_ini_value(const std::string& str) noexcept;

    /**
     * Unescape special characters in INI value
     *
     * @param str Escaped string
     * @return Unescaped string
     */
    static std::string unescape_ini_value(const std::string& str) noexcept;
};

} // namespace configgui::core::serializers

#endif // CONFIGGUI_CORE_SERIALIZERS_INI_SERIALIZER_H
