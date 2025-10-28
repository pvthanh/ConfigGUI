/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * configuration_reader.h
 * Multi-format configuration save feature: Configuration file reader
 * Handles file operations for loading configurations from disk
 */

#ifndef CONFIGGUI_CORE_IO_CONFIGURATION_READER_H
#define CONFIGGUI_CORE_IO_CONFIGURATION_READER_H

#include "../models/serialization_result.h"
#include "../serializers/format_serializer.h"
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace configgui::core::io {

using namespace models;
using namespace serializers;

/**
 * @class ConfigurationReader
 * Handles file reading operations for configuration files
 *
 * Provides safe file reading with automatic format detection:
 * - Reads configuration files from disk
 * - Detects format from file extension or content
 * - Deserializes format-specific content to JSON
 * - Returns detailed error messages on failure
 *
 * Supports reading configurations in JSON and INI formats,
 * automatically converting them to internal JSON representation.
 */
class ConfigurationReader {
public:
    ConfigurationReader() = default;
    ~ConfigurationReader() = default;

    /**
     * Read configuration file and deserialize to JSON
     *
     * @param file_path The file path to read
     * @param serializer The serializer to use for deserialization
     * @return Result containing JSON data if successful, error code otherwise
     *
     * @note Auto-detects format from file extension
     * @note Returns FILE_IO_ERROR if file not found or not readable
     * @note Returns INVALID_JSON/INVALID_INI if deserialization fails
     *
     * Example:
     *   ConfigurationReader reader;
     *   auto result = reader.read_configuration_file(
     *       "/path/to/config.ini",
     *       std::make_unique<IniSerializer>()
     *   );
     *   if (result) { config_data = result.value(); }
     */
    Result<json> read_configuration_file(
        const std::string& file_path,
        const std::shared_ptr<FormatSerializer>& serializer) noexcept;

    /**
     * Read raw file content without deserialization
     *
     * @param file_path The file path to read
     * @return Result containing file content if successful, error code otherwise
     *
     * @note Does not perform deserialization
     * @note Useful for preprocessing or format detection
     * @note Returns FILE_IO_ERROR if read fails
     *
     * Example:
     *   auto result = reader.read_file_content("/path/to/config.json");
     *   if (result) { content = result.value(); }
     */
    Result<std::string> read_file_content(const std::string& file_path) noexcept;

private:
    /**
     * Internal helper to validate file path
     * Checks for existence and readability
     *
     * @param file_path The file path to validate
     * @return Result::success() if valid, error if not
     */
    Result<void> validate_file_path(const std::string& file_path) const noexcept;
};

} // namespace configgui::core::io

#endif // CONFIGGUI_CORE_IO_CONFIGURATION_READER_H
