/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * configuration_reader.cpp
 * Multi-format configuration save feature: Configuration file reader implementation
 */

#include "configuration_reader.h"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace configgui::core::io {

Result<void> ConfigurationReader::validate_file_path(const std::string& file_path) const noexcept
{
    try {
        std::filesystem::path path(file_path);

        if (!std::filesystem::exists(path)) {
            return Result<void>::error(
                SerializationError::FILE_IO_ERROR,
                "File does not exist: " + file_path);
        }

        if (!std::filesystem::is_regular_file(path)) {
            return Result<void>::error(
                SerializationError::FILE_IO_ERROR,
                "Path is not a regular file: " + file_path);
        }

        // Check if file is readable
        std::ifstream test_file(file_path);
        if (!test_file.good()) {
            return Result<void>::error(
                SerializationError::FILE_IO_ERROR,
                "File is not readable: " + file_path);
        }

        return Result<void>::success();
    } catch (const std::exception& e) {
        return Result<void>::error(
            SerializationError::FILE_IO_ERROR,
            std::string("Failed to validate file path: ") + e.what());
    }
}

Result<std::string> ConfigurationReader::read_file_content(
    const std::string& file_path) noexcept
{
    try {
        // Validate file path first
        auto validation = validate_file_path(file_path);
        if (!validation) {
            return Result<std::string>(validation.error_code(), validation.error_msg());
        }

        // Read file content
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return Result<std::string>(
                SerializationError::FILE_IO_ERROR,
                "Failed to open file: " + file_path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        if (!buffer.good()) {
            return Result<std::string>(
                SerializationError::FILE_IO_ERROR,
                "Failed to read file: " + file_path);
        }

        return Result<std::string>(buffer.str());
    } catch (const std::exception& e) {
        return Result<std::string>(
            SerializationError::FILE_IO_ERROR,
            std::string("Exception reading file: ") + e.what());
    }
}

Result<json> ConfigurationReader::read_configuration_file(
    const std::string& file_path,
    const std::shared_ptr<FormatSerializer>& serializer) noexcept
{
    try {
        if (!serializer) {
            return Result<json>(
                SerializationError::UNKNOWN_ERROR,
                "Serializer pointer is null");
        }

        // Read raw file content
        auto read_result = read_file_content(file_path);
        if (!read_result) {
            return Result<json>(read_result.error_code(), read_result.error_msg());
        }

        // Deserialize content to JSON
        auto deserialize_result = serializer->deserialize(read_result.value());
        if (!deserialize_result) {
            return Result<json>(
                deserialize_result.error_code(),
                deserialize_result.error_msg_or_default());
        }

        return deserialize_result;
    } catch (const std::exception& e) {
        return Result<json>(
            SerializationError::UNKNOWN_ERROR,
            std::string("Failed to read configuration file: ") + e.what());
    }
}

} // namespace configgui::core::io
