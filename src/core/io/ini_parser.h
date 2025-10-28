// SPDX-License-Identifier: MIT
// INIParser - INI Configuration File Parsing

#ifndef CONFIGGUI_IO_INI_PARSER_H
#define CONFIGGUI_IO_INI_PARSER_H

#include "../result.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

// Use ordered_json to preserve field order as defined in schema
using json = nlohmann::ordered_json;
using configgui::core::Result;
using JsonResult = configgui::core::Result<nlohmann::ordered_json, std::string>;

namespace configgui {
namespace io {

/**
 * @class INIParser
 * @brief Parses INI configuration files into JSON objects
 *
 * Handles INI format with:
 * - Section headers: [SectionName]
 * - Nested sections: [Server.Http]
 * - Key-value pairs: key=value
 * - Arrays: items[0]=value, items[1]=value
 * - Type inference: true/false→bool, 123→int, 3.14→float, else→string
 * - Escape sequences: \n, \r, \t
 * - Comments: lines starting with ; or #
 *
 * Transforms INI structure into nested JSON objects:
 *   [Section]
 *   key=value
 *   [Section.SubSection]
 *   subkey=value
 *
 * Becomes:
 *   {
 *     "Section": {
 *       "key": "value",
 *       "SubSection": {
 *         "subkey": "value"
 *       }
 *     }
 *   }
 */
class INIParser
{
public:
    /**
     * @brief Parse INI content from string
     * @param ini_content INI file content as string
     * @return Result containing parsed JSON or error message
     */
    static JsonResult parse(const std::string& ini_content);

    /**
     * @brief Parse INI content from file
     * @param file_path Path to INI file
     * @return Result containing parsed JSON or error message
     */
    static JsonResult parseFile(const std::string& file_path);

private:
    /**
     * @brief Internal parsing context
     */
    struct ParseContext
    {
        json result;
        std::string current_section;
        int line_number = 0;
    };

    INIParser() = default;

    /**
     * @brief Check if line is a section header [SectionName]
     * @param line Trimmed line to check
     * @return true if line is section header
     */
    static bool is_section_header(const std::string& line);

    /**
     * @brief Extract section name from header line
     * @param line Section header line: [SectionName]
     * @return Section name without brackets
     */
    static std::string extract_section_name(const std::string& line);

    /**
     * @brief Check if line is a key-value pair
     * @param line Trimmed line to check
     * @return true if line contains key=value
     */
    static bool is_key_value_pair(const std::string& line);

    /**
     * @brief Parse key-value pair from line
     * @param line Line containing key=value
     * @return Pair of (key, value_string)
     */
    static std::pair<std::string, std::string> parse_key_value(const std::string& line);

    /**
     * @brief Check if line is a comment or empty
     * @param line Trimmed line to check
     * @return true if line should be skipped
     */
    static bool is_comment_or_empty(const std::string& line);

    /**
     * @brief Trim whitespace from string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);

    /**
     * @brief Parse string value to appropriate JSON type
     *
     * Type inference:
     * - "true"/"false" → boolean
     * - "123" → integer
     * - "3.14" → float
     * - else → string (with escape sequences unescaped)
     *
     * @param value_str String value to parse
     * @return JSON value with inferred type
     */
    static json parse_value(const std::string& value_str);

    /**
     * @brief Unescape special characters in string
     * @param value String with escape sequences
     * @return Unescaped string
     */
    static std::string unescape_value(const std::string& value);

    /**
     * @brief Check if string is a valid boolean
     * @param str String to check
     * @return true if "true" or "false" (case-insensitive)
     */
    static bool is_boolean(const std::string& str);

    /**
     * @brief Check if string is a valid integer
     * @param str String to check
     * @return true if string contains only digits (with optional leading +/-)
     */
    static bool is_integer(const std::string& str);

    /**
     * @brief Check if string is a valid float
     * @param str String to check
     * @return true if string is valid floating point number
     */
    static bool is_float(const std::string& str);

    /**
     * @brief Handle array index in key: items[0], items[1]
     *
     * Transforms:
     *   items[0]=first
     *   items[1]=second
     *
     * Into:
     *   { "items": ["first", "second"] }
     *
     * @param obj JSON object to update
     * @param key_with_index Key with array index (e.g., "items[0]")
     * @param value Value to insert
     */
    static void set_array_value(json& obj, const std::string& key_with_index, const json& value);

    /**
     * @brief Set value in nested JSON structure using dot notation
     *
     * Handles:
     * - Simple paths: "key" → obj["key"]
     * - Nested paths: "section.key" → obj["section"]["key"]
     * - Array paths: "items[0]" → obj["items"][0]
     * - Mixed: "section.items[0].name" → obj["section"]["items"][0]["name"]
     *
     * @param obj JSON object to update (usually result object)
     * @param path Dot notation path (e.g., "Server.Http.Port")
     * @param value Value to set
     */
    static void set_nested_value(json& obj, const std::string& path, const json& value);

    /**
     * @brief Split string by delimiter
     * @param str String to split
     * @param delimiter Character to split on
     * @return Vector of substrings
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);
};

} // namespace io
} // namespace configgui

#endif // CONFIGGUI_IO_INI_PARSER_H
