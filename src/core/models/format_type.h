/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * format_type.h
 * Multi-format configuration save feature: Format type enumeration
 * Defines supported serialization formats (JSON, INI)
 */

#ifndef CONFIGGUI_CORE_MODELS_FORMAT_TYPE_H
#define CONFIGGUI_CORE_MODELS_FORMAT_TYPE_H

#include <string>
#include <optional>

namespace configgui::core::models {

/**
 * @enum FormatType
 * Supported configuration file formats for serialization and deserialization
 */
enum class FormatType {
    JSON, ///< JSON format (primary, schema-validated)
    INI   ///< INI format (secondary, flattened structure)
};

/**
 * Convert FormatType enum to human-readable string
 * @param format The format type
 * @return String representation (e.g., "JSON", "INI")
 */
std::string format_to_string(FormatType format) noexcept;

/**
 * Convert string to FormatType enum
 * Supports: "json", "JSON", "ini", "INI"
 * @param format_str The format string
 * @return FormatType if valid, std::nullopt if invalid
 */
std::optional<FormatType> string_to_format(const std::string& format_str) noexcept;

/**
 * Get MIME type for the given format
 * @param format The format type
 * @return MIME type string (e.g., "application/json", "text/plain")
 */
std::string get_mime_type(FormatType format) noexcept;

/**
 * Get file extension for the given format
 * @param format The format type
 * @return File extension including dot (e.g., ".json", ".ini")
 */
std::string get_file_extension(FormatType format) noexcept;

/**
 * Detect format from file path
 * @param file_path The file path
 * @return FormatType if detected, std::nullopt if unrecognized
 */
std::optional<FormatType> detect_format_from_path(const std::string& file_path) noexcept;

} // namespace configgui::core::models

#endif // CONFIGGUI_CORE_MODELS_FORMAT_TYPE_H
