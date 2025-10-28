/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * ini_serializer.cpp
 * Multi-format configuration save feature: INI serializer implementation
 * Phase 4: User Story 2 - Save Configuration in INI Format
 */

#include "ini_serializer.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace configgui::core::serializers {

Result<std::string> IniSerializer::serialize(const SerializationContext& context) noexcept
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

        // Flatten JSON to key-value pairs
        auto flat_map = flatten_json(context.configuration_data);

        // Group by sections (prefix before last dot)
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> sections;
        for (const auto& [key, value] : flat_map) {
            size_t last_dot = key.rfind('.');
            std::string section = (last_dot != std::string::npos) ? key.substr(0, last_dot) : "";
            std::string var_name = (last_dot != std::string::npos) ? key.substr(last_dot + 1) : key;
            sections[section].push_back({var_name, value});
        }

        // Generate INI content
        std::ostringstream oss;
        bool first_section = true;

        for (const auto& [section, vars] : sections) {
            if (!first_section) {
                oss << "\n";
            }

            if (!section.empty()) {
                oss << "[" << section << "]\n";
            }

            for (const auto& [var, value] : vars) {
                oss << var << "=" << value << "\n";
            }

            first_section = false;
        }

        return Result<std::string>(oss.str());
    } catch (const std::exception& e) {
        return Result<std::string>(
            SerializationError::INVALID_INI,
            std::string("INI serialization failed: ") + e.what());
    }
}

Result<json> IniSerializer::deserialize(const std::string& content) noexcept
{
    try {
        // TODO: Implement mINI library parsing here
        // For now, return a basic implementation that can be extended
        if (content.empty()) {
            return Result<json>(
                SerializationError::INVALID_INI,
                "Cannot deserialize empty content");
        }

        // Placeholder: Return empty JSON object
        // Will be replaced with full mINI parsing in Phase 4 implementation
        return Result<json>(json::object());
    } catch (const std::exception& e) {
        return Result<json>(
            SerializationError::INVALID_INI,
            std::string("INI deserialization failed: ") + e.what());
    }
}

std::optional<std::string> IniSerializer::validate_pre_serialization(
    const SerializationContext& context) const noexcept
{
    try {
        // Check that configuration data is valid JSON object
        if (!context.configuration_data.is_object()) {
            return "Configuration data must be a JSON object";
        }

        // INI has limitations with certain data types
        // Complex nested structures are flattened, which is acceptable
        return std::nullopt; // No validation errors
    } catch (const std::exception&) {
        return "Validation threw unexpected exception";
    }
}

std::string IniSerializer::get_mime_type() const noexcept
{
    return "text/plain";
}

std::string IniSerializer::get_format_name() const noexcept
{
    return "INI";
}

FormatType IniSerializer::get_format_type() const noexcept
{
    return FormatType::INI;
}

std::map<std::string, std::string> IniSerializer::flatten_json(
    const json& obj,
    const std::string& prefix) noexcept
{
    std::map<std::string, std::string> result;

    try {
        if (obj.is_object()) {
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();
                const auto& value = it.value();

                if (value.is_object()) {
                    // Recursively flatten nested objects
                    auto nested = flatten_json(value, key);
                    result.insert(nested.begin(), nested.end());
                } else if (value.is_array()) {
                    // Handle arrays with indexed notation
                    for (size_t i = 0; i < value.size(); ++i) {
                        std::string array_key = key + "." + std::to_string(i);
                        const auto& elem = value[i];

                        if (elem.is_object()) {
                            auto nested = flatten_json(elem, array_key);
                            result.insert(nested.begin(), nested.end());
                        } else if (!elem.is_null()) {
                            result[array_key] = json_value_to_ini_string(elem);
                        }
                    }
                } else if (!value.is_null()) {
                    // Store primitive types
                    result[key] = json_value_to_ini_string(value);
                }
            }
        }
    } catch (const std::exception&) {
        // Return partial results on error
    }

    return result;
}

json IniSerializer::unflatten_map(const std::map<std::string, std::string>& flat_map) noexcept
{
    json result = json::object();

    try {
        for (const auto& [key, value] : flat_map) {
            // Parse the dot-notation key
            std::vector<std::string> parts;
            std::string current;
            for (char c : key) {
                if (c == '.') {
                    if (!current.empty()) {
                        parts.push_back(current);
                        current.clear();
                    }
                } else {
                    current += c;
                }
            }
            if (!current.empty()) {
                parts.push_back(current);
            }

            // Navigate/create nested structure
            json* current_obj = &result;
            for (size_t i = 0; i < parts.size() - 1; ++i) {
                const auto& part = parts[i];

                // Check if this part is an array index
                if (std::all_of(part.begin(), part.end(), [](char c) { return std::isdigit(c); })) {
                    // This is an array index - skip, will be handled by parent
                    continue;
                }

                if (!current_obj->contains(part)) {
                    (*current_obj)[part] = json::object();
                }
                current_obj = &(*current_obj)[part];
            }

            // Set the final value
            if (!parts.empty()) {
                (*current_obj)[parts.back()] = ini_string_to_json_value(value);
            }
        }
    } catch (const std::exception&) {
        // Return partial results on error
    }

    return result;
}

std::string IniSerializer::json_value_to_ini_string(const json& val) noexcept
{
    try {
        if (val.is_string()) {
            return escape_ini_value(val.get<std::string>());
        } else if (val.is_number_integer()) {
            return std::to_string(val.get<long long>());
        } else if (val.is_number_float()) {
            return std::to_string(val.get<double>());
        } else if (val.is_boolean()) {
            return val.get<bool>() ? "true" : "false";
        }
    } catch (const std::exception&) {
    }

    return "";
}

json IniSerializer::ini_string_to_json_value(const std::string& value_str) noexcept
{
    try {
        if (value_str == "true") {
            return json(true);
        } else if (value_str == "false") {
            return json(false);
        } else if (value_str == "null") {
            return json(nullptr);
        } else {
            // Try to parse as number
            try {
                if (value_str.find('.') != std::string::npos) {
                    return json(std::stod(value_str));
                } else {
                    return json(std::stoll(value_str));
                }
            } catch (const std::exception&) {
                // Fall back to string
                return json(unescape_ini_value(value_str));
            }
        }
    } catch (const std::exception&) {
        return json(unescape_ini_value(value_str));
    }
}

std::string IniSerializer::escape_ini_value(const std::string& str) noexcept
{
    std::string result;
    for (char c : str) {
        switch (c) {
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        case '\\':
            result += "\\\\";
            break;
        case '=':
            result += "\\=";
            break;
        default:
            result += c;
        }
    }
    return result;
}

std::string IniSerializer::unescape_ini_value(const std::string& str) noexcept
{
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            char next = str[++i];
            switch (next) {
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            case '\\':
                result += '\\';
                break;
            case '=':
                result += '=';
                break;
            default:
                result += next;
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

} // namespace configgui::core::serializers
