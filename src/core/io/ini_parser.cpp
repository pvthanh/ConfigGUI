// SPDX-License-Identifier: MIT
// INIParser - Implementation

#include "ini_parser.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <cctype>
#include <algorithm>

namespace configgui {
namespace io {

JsonResult INIParser::parse(const std::string& ini_content)
{
    try
    {
        ParseContext ctx;
        ctx.result = json::object();  // Explicitly initialize as object
        std::istringstream iss(ini_content);
        std::string line;

        while (std::getline(iss, line))
        {
            ctx.line_number++;

            // Trim whitespace
            line = trim(line);

            // Skip empty lines and comments
            if (is_comment_or_empty(line))
            {
                continue;
            }

            // Check for section header [SectionName]
            if (is_section_header(line))
            {
                ctx.current_section = extract_section_name(line);
                continue;
            }

            // Parse key-value pair
            if (is_key_value_pair(line))
            {
                auto [key, value_str] = parse_key_value(line);

                // Unescape value
                value_str = unescape_value(value_str);

                // Parse value type
                json value = parse_value(value_str);

                // Build full path with section prefix
                std::string full_path = key;
                if (!ctx.current_section.empty())
                {
                    full_path = ctx.current_section + "." + key;
                }

                // Check if this is an array index (key[0], key[1], etc.)
                if (full_path.find('[') != std::string::npos)
                {
                    set_array_value(ctx.result, full_path, value);
                }
                else
                {
                    // Regular key-value pair
                    set_nested_value(ctx.result, full_path, value);
                }

                continue;
            }

            // Invalid line - but we'll be lenient and skip it
            // In strict mode, could return error here
        }

        return JsonResult(ctx.result);
    }
    catch (const std::exception& e)
    {
        return JsonResult(std::string("INI parse error: ") + e.what());
    }
}

JsonResult INIParser::parseFile(const std::string& file_path)
{
    try
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            return JsonResult(std::string("Cannot open INI file: ") + file_path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return parse(buffer.str());
    }
    catch (const std::exception& e)
    {
        return JsonResult(std::string("Failed to read INI file: ") + e.what());
    }
}

bool INIParser::is_section_header(const std::string& line)
{
    if (line.empty() || line[0] != '[' || line[line.length() - 1] != ']')
    {
        return false;
    }
    return true;
}

std::string INIParser::extract_section_name(const std::string& line)
{
    if (line.size() < 3)
    {
        return "";
    }
    // Remove brackets and trim
    return trim(line.substr(1, line.size() - 2));
}

bool INIParser::is_key_value_pair(const std::string& line)
{
    size_t eq_pos = line.find('=');
    return eq_pos != std::string::npos && eq_pos > 0;
}

std::pair<std::string, std::string> INIParser::parse_key_value(const std::string& line)
{
    size_t eq_pos = line.find('=');
    if (eq_pos == std::string::npos)
    {
        return {"", ""};
    }

    std::string key = trim(line.substr(0, eq_pos));
    std::string value = trim(line.substr(eq_pos + 1));

    return {key, value};
}

bool INIParser::is_comment_or_empty(const std::string& line)
{
    if (line.empty())
    {
        return true;
    }

    char first_char = line[0];
    return first_char == ';' || first_char == '#';
}

std::string INIParser::trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, (end - start + 1));
}

json INIParser::parse_value(const std::string& value_str)
{
    if (value_str.empty())
    {
        return json("");
    }

    // Check for boolean
    if (is_boolean(value_str))
    {
        return value_str == "true" || value_str == "True" || value_str == "TRUE";
    }

    // Check for integer
    if (is_integer(value_str))
    {
        try
        {
            return std::stoi(value_str);
        }
        catch (...)
        {
            return json(value_str);
        }
    }

    // Check for float
    if (is_float(value_str))
    {
        try
        {
            return std::stod(value_str);
        }
        catch (...)
        {
            return json(value_str);
        }
    }

    // Default to string
    return json(value_str);
}

std::string INIParser::unescape_value(const std::string& value)
{
    std::string result = value;

    // Replace escape sequences
    size_t pos = 0;
    while ((pos = result.find("\\n", pos)) != std::string::npos)
    {
        result.replace(pos, 2, "\n");
        pos += 1;
    }

    pos = 0;
    while ((pos = result.find("\\r", pos)) != std::string::npos)
    {
        result.replace(pos, 2, "\r");
        pos += 1;
    }

    pos = 0;
    while ((pos = result.find("\\t", pos)) != std::string::npos)
    {
        result.replace(pos, 2, "\t");
        pos += 1;
    }

    return result;
}

bool INIParser::is_boolean(const std::string& str)
{
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower == "true" || lower == "false";
}

bool INIParser::is_integer(const std::string& str)
{
    if (str.empty())
    {
        return false;
    }

    size_t start = 0;
    if (str[0] == '+' || str[0] == '-')
    {
        start = 1;
    }

    if (start >= str.length())
    {
        return false;
    }

    for (size_t i = start; i < str.length(); ++i)
    {
        if (!std::isdigit(str[i]))
        {
            return false;
        }
    }

    return true;
}

bool INIParser::is_float(const std::string& str)
{
    if (str.empty())
    {
        return false;
    }

    bool has_dot = false;
    size_t start = 0;

    if (str[0] == '+' || str[0] == '-')
    {
        start = 1;
    }

    if (start >= str.length())
    {
        return false;
    }

    for (size_t i = start; i < str.length(); ++i)
    {
        if (str[i] == '.')
        {
            if (has_dot)
            {
                return false;
            }
            has_dot = true;
        }
        else if (!std::isdigit(str[i]))
        {
            return false;
        }
    }

    return has_dot && str.find_last_not_of("0123456789+-.") == std::string::npos;
}

void INIParser::set_array_value(json& obj, const std::string& key_with_index, const json& value)
{
    // Handle nested paths like "Arrays.items[0]"
    // Find the array index portion
    size_t bracket_pos = key_with_index.rfind('[');
    if (bracket_pos == std::string::npos)
    {
        // No array notation, treat as regular value
        set_nested_value(obj, key_with_index, value);
        return;
    }

    // Split path into base path and array index
    std::string base_path = key_with_index.substr(0, bracket_pos);
    std::string index_str = key_with_index.substr(bracket_pos + 1);

    // Remove closing bracket
    if (!index_str.empty() && index_str.back() == ']')
    {
        index_str.pop_back();
    }

    // Parse index
    size_t index = 0;
    try
    {
        index = static_cast<size_t>(std::stoi(index_str));
    }
    catch (...)
    {
        // Invalid index, treat as regular value
        set_nested_value(obj, key_with_index, value);
        return;
    }

    // Navigate to array and set value
    // base_path is something like "Arrays.items"
    auto parts = split(base_path, '.');

    // Navigate to the parent of the array
    json* current = &obj;
    for (size_t i = 0; i < parts.size() - 1; ++i)
    {
        const auto& part = parts[i];
        if (!current->contains(part))
        {
            (*current)[part] = json::object();
        }
        current = &((*current)[part]);
    }

    // Now current points to the parent object
    // Get the array key (last part)
    std::string array_key = parts.back();

    // Ensure we have an array
    if (!current->contains(array_key) || !(*current)[array_key].is_array())
    {
        (*current)[array_key] = json::array();
    }

    // Extend array if necessary
    auto& arr = (*current)[array_key];
    while (arr.size() <= index)
    {
        arr.push_back(json());
    }

    // Set value
    arr[index] = value;
}

void INIParser::set_nested_value(json& obj, const std::string& path, const json& value)
{
    if (path.empty())
    {
        return;
    }

    // Split path by dots
    auto parts = split(path, '.');

    // Navigate/create nested structure
    json* current = &obj;
    for (size_t i = 0; i < parts.size() - 1; ++i)
    {
        const std::string& part = parts[i];

        if (!current->contains(part))
        {
            (*current)[part] = json::object();
        }

        current = &((*current)[part]);
    }

    // Set final value
    (*current)[parts.back()] = value;
}

std::vector<std::string> INIParser::split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimiter))
    {
        result.push_back(item);
    }

    return result;
}

} // namespace io
} // namespace configgui
