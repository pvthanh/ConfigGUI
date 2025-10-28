/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * format_type.cpp
 * Multi-format configuration save feature: Format type implementation
 */

#include "format_type.h"
#include <algorithm>
#include <cctype>

namespace configgui::core::models {

std::string format_to_string(FormatType format) noexcept
{
    switch (format) {
    case FormatType::JSON:
        return "JSON";
    case FormatType::INI:
        return "INI";
    }
    return "UNKNOWN";
}

std::optional<FormatType> string_to_format(const std::string& format_str) noexcept
{
    std::string upper_str = format_str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    if (upper_str == "JSON") {
        return FormatType::JSON;
    } else if (upper_str == "INI") {
        return FormatType::INI;
    }
    return std::nullopt;
}

std::string get_mime_type(FormatType format) noexcept
{
    switch (format) {
    case FormatType::JSON:
        return "application/json";
    case FormatType::INI:
        return "text/plain";
    }
    return "application/octet-stream";
}

std::string get_file_extension(FormatType format) noexcept
{
    switch (format) {
    case FormatType::JSON:
        return ".json";
    case FormatType::INI:
        return ".ini";
    }
    return "";
}

std::optional<FormatType> detect_format_from_path(const std::string& file_path) noexcept
{
    // Find the last dot in the path
    size_t dot_pos = file_path.rfind('.');
    if (dot_pos == std::string::npos || dot_pos == file_path.length() - 1) {
        return std::nullopt;
    }

    // Extract extension and convert to lowercase
    std::string extension = file_path.substr(dot_pos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (extension == "json") {
        return FormatType::JSON;
    } else if (extension == "ini" || extension == "cfg" || extension == "conf") {
        return FormatType::INI;
    }

    return std::nullopt;
}

} // namespace configgui::core::models
