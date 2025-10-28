// SPDX-License-Identifier: MIT
// INIWriter - Implementation

#include "ini_writer.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace configgui {
namespace io {

VoidResult INIWriter::writeFile(const std::string& file_path, const json& data)
{
    try
    {
        auto result = toString(data);
        if (result.is_failure())
        {
            return VoidResult(result.error());
        }

        std::ofstream file(file_path);
        if (!file.is_open())
        {
            return VoidResult(Error{std::string("Cannot open file for writing: ") + file_path});
        }

        file << result.value();
        file.close();

        return VoidResult(Success{});
    }
    catch (const std::exception& e)
    {
        return VoidResult(Error{std::string("Failed to write INI file: ") + e.what()});
    }
}

StringResult INIWriter::toString(const json& data)
{
    try
    {
        if (!data.is_object())
        {
            return StringResult(Error{"JSON data must be an object to convert to INI"});
        }

        std::string result = serialize_json(data);
        return StringResult(result);
    }
    catch (const std::exception& e)
    {
        return StringResult(Error{std::string("Failed to serialize JSON to INI: ") + e.what()});
    }
}

std::string INIWriter::serialize_json(const json& obj, const std::string& section_prefix)
{
    std::ostringstream oss;

    // Iterate through object properties
    for (auto it = obj.begin(); it != obj.end(); ++it)
    {
        const std::string& key = it.key();
        const json& value = it.value();

        if (value.is_object())
        {
            // Nested object - create section header
            std::string new_section = section_prefix.empty() ? key : section_prefix + "." + key;

            oss << "[" << new_section << "]\n";

            // Recursively handle nested object's properties
            for (auto nested_it = value.begin(); nested_it != value.end(); ++nested_it)
            {
                const std::string& nested_key = nested_it.key();
                const json& nested_value = nested_it.value();

                if (nested_value.is_object())
                {
                    // Further nesting - recurse
                    oss << serialize_json(nested_value, new_section);
                }
                else if (nested_value.is_array())
                {
                    // Array - write each element with index
                    for (size_t i = 0; i < nested_value.size(); ++i)
                    {
                        oss << nested_key << "[" << i << "]=" << serialize_value(nested_value[i]) << "\n";
                    }
                }
                else
                {
                    // Primitive value
                    oss << nested_key << "=" << serialize_value(nested_value) << "\n";
                }
            }

            // Add blank line between sections (if not last)
            if (std::next(it) != obj.end())
            {
                oss << "\n";
            }
        }
        else if (value.is_array())
        {
            // Top-level array - write each element with index
            for (size_t i = 0; i < value.size(); ++i)
            {
                oss << key << "[" << i << "]=" << serialize_value(value[i]) << "\n";
            }
        }
        else
        {
            // Primitive value at top level (shouldn't happen normally)
            oss << key << "=" << serialize_value(value) << "\n";
        }
    }

    return oss.str();
}

std::string INIWriter::serialize_value(const json& value)
{
    if (value.is_null())
    {
        return "";
    }
    else if (value.is_boolean())
    {
        return value.get<bool>() ? "true" : "false";
    }
    else if (value.is_number_integer())
    {
        return std::to_string(value.get<int64_t>());
    }
    else if (value.is_number_float())
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << value.get<double>();
        std::string result = oss.str();

        // Remove trailing zeros after decimal point
        if (result.find('.') != std::string::npos)
        {
            result.erase(result.find_last_not_of('0') + 1, std::string::npos);
            if (result.back() == '.')
            {
                result.erase(result.length() - 1);
            }
        }

        return result;
    }
    else if (value.is_string())
    {
        return escape_value(value.get<std::string>());
    }
    else
    {
        // For arrays and objects, convert to JSON string
        return value.dump();
    }
}

std::string INIWriter::escape_value(const std::string& value)
{
    std::string result;

    for (char c : value)
    {
        switch (c)
        {
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
        default:
            result += c;
        }
    }

    return result;
}

} // namespace io
} // namespace configgui
