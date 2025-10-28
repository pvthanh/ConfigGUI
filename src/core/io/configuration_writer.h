/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * configuration_writer.h
 * Multi-format configuration save feature: Configuration file writer
 * Handles atomic file operations for saving configurations to disk
 */

#ifndef CONFIGGUI_CORE_IO_CONFIGURATION_WRITER_H
#define CONFIGGUI_CORE_IO_CONFIGURATION_WRITER_H

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
 * @class ConfigurationWriter
 * Handles atomic file writing operations for configuration files
 *
 * Provides safe, atomic file write operations with error handling:
 * - Writes to temporary file first
 * - Verifies write success
 * - Atomically renames to target path
 * - Graceful error handling with detailed error messages
 *
 * This ensures that configuration files are either completely written
 * or left untouched, preventing file corruption on failure.
 */
class ConfigurationWriter {
public:
    ConfigurationWriter() = default;
    ~ConfigurationWriter() = default;

    /**
     * Write configuration data to file in specified format
     *
     * @param file_path The target file path
     * @param data The configuration data (JSON format)
     * @param serializer The serializer to use for format conversion
     * @return Result indicating success or error
     *
     * @note Uses atomic write: temp file + rename to prevent corruption
     * @note File permissions: Creates with mode 0644 (readable by all, writable by owner)
     * @note Returns FILE_IO_ERROR on permission denied, disk full, or other I/O issues
     *
     * Example:
     *   ConfigurationWriter writer;
     *   auto result = writer.write_configuration_file(
     *       "/path/to/config.json",
     *       data,
     *       std::make_unique<JsonSerializer>()
     *   );
     *   if (!result) { std::cerr << result.error_msg_or_default(); }
     */
    Result<void> write_configuration_file(
        const std::string& file_path,
        const json& data,
        const std::shared_ptr<FormatSerializer>& serializer) noexcept;

    /**
     * Write serialized content directly to file
     *
     * @param file_path The target file path
     * @param content The pre-serialized content string
     * @param mime_type Optional MIME type for file identification
     * @return Result indicating success or error
     *
     * @note Uses atomic write: temp file + rename to prevent corruption
     * @note Useful when content is already serialized (e.g., from HTTP response)
     * @note Returns FILE_IO_ERROR on write failures
     *
     * Example:
     *   ConfigurationWriter writer;
     *   auto result = writer.write_file_content(
     *       "/path/to/config.ini",
     *       serialized_content,
     *       "text/plain"
     *   );
     */
    Result<void> write_file_content(
        const std::string& file_path,
        const std::string& content,
        const std::string& mime_type = "application/octet-stream") noexcept;

private:
    /**
     * Internal helper for atomic file write
     * Writes to temporary file then atomically renames to target path
     *
     * @param file_path The target file path
     * @param content The content to write
     * @return Result indicating success or error
     */
    Result<void> atomic_write(
        const std::string& file_path,
        const std::string& content) noexcept;

    /**
     * Generate temporary file path in the same directory as target
     * Ensures temp file is on same filesystem for atomic rename
     *
     * @param file_path The target file path
     * @return Temporary file path
     */
    std::string generate_temp_path(const std::string& file_path) const noexcept;
};

} // namespace configgui::core::io

#endif // CONFIGGUI_CORE_IO_CONFIGURATION_WRITER_H
