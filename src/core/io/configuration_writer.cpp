/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * configuration_writer.cpp
 * Multi-format configuration save feature: Configuration file writer implementation
 */

#include "configuration_writer.h"
#include <fstream>
#include <filesystem>
#include <cstring>
#include <ctime>
#include <random>

namespace configgui::core::io {

std::string ConfigurationWriter::generate_temp_path(const std::string& file_path) const noexcept
{
    try {
        std::filesystem::path target_path(file_path);
        std::filesystem::path dir = target_path.parent_path();

        // Create temp filename with timestamp and random suffix
        auto now = std::time(nullptr);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);

        std::string temp_name = ".tmp_config_" + std::to_string(now) + "_" + std::to_string(dis(gen));
        return (dir / temp_name).string();
    } catch (const std::exception&) {
        // Fallback temp path
        return file_path + ".tmp";
    }
}

Result<void> ConfigurationWriter::atomic_write(
    const std::string& file_path,
    const std::string& content) noexcept
{
    try {
        std::string temp_path = generate_temp_path(file_path);

        // Write to temporary file
        {
            std::ofstream temp_file(temp_path, std::ios::binary | std::ios::trunc);
            if (!temp_file.is_open()) {
                return Result<void>::error(
                    SerializationError::FILE_IO_ERROR,
                    "Failed to open temporary file: " + temp_path);
            }

            temp_file.write(content.c_str(), static_cast<std::streamsize>(content.size()));
            if (!temp_file.good()) {
                // Try to clean up temp file
                std::filesystem::remove(temp_path);
                return Result<void>::error(
                    SerializationError::FILE_IO_ERROR,
                    "Failed to write to temporary file: " + temp_path);
            }
        } // File closed here

        // Atomically rename temp file to target path
        try {
            std::filesystem::rename(temp_path, file_path);
        } catch (const std::filesystem::filesystem_error& e) {
            // Clean up temp file if rename fails
            std::filesystem::remove(temp_path);
            return Result<void>::error(
                SerializationError::FILE_IO_ERROR,
                std::string("Failed to rename temporary file: ") + e.what());
        }

        return Result<void>::success();
    } catch (const std::exception& e) {
        return Result<void>::error(
            SerializationError::FILE_IO_ERROR,
            std::string("Atomic write failed: ") + e.what());
    }
}

Result<void> ConfigurationWriter::write_configuration_file(
    const std::string& file_path,
    const json& data,
    const std::shared_ptr<FormatSerializer>& serializer) noexcept
{
    try {
        if (!serializer) {
            return Result<void>::error(
                SerializationError::UNKNOWN_ERROR,
                "Serializer pointer is null");
        }

        // Create serialization context
        SerializationContext context = {
            data,
            serializer->get_format_type(),
            file_path,
            std::nullopt,
            true,
            true
        };

        // Serialize data to format-specific content
        auto serialize_result = serializer->serialize(context);
        if (!serialize_result) {
            return Result<void>::error(
                serialize_result.error_code(),
                serialize_result.error_msg_or_default());
        }

        // Write serialized content to file
        return write_file_content(file_path, serialize_result.value(), serializer->get_mime_type());
    } catch (const std::exception& e) {
        return Result<void>::error(
            SerializationError::UNKNOWN_ERROR,
            std::string("Failed to write configuration file: ") + e.what());
    }
}

Result<void> ConfigurationWriter::write_file_content(
    const std::string& file_path,
    const std::string& content,
    const std::string& mime_type) noexcept
{
    (void)mime_type; // Currently unused, may be used for file type tracking in future

    return atomic_write(file_path, content);
}

} // namespace configgui::core::io
