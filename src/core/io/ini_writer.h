// SPDX-License-Identifier: MIT
// INIWriter - INI File Writing

#ifndef CONFIGGUI_IO_INI_WRITER_H
#define CONFIGGUI_IO_INI_WRITER_H

#include "../result.h"
#include <nlohmann/json.hpp>
#include <string>

// Use ordered_json to preserve field order as defined in schema
using json = nlohmann::ordered_json;
using configgui::core::Result;

// Use an error wrapper to avoid ambiguity when T and E are the same type
struct Error { std::string message; };

using StringResult = configgui::core::Result<std::string, Error>;

// For void results, we use an empty struct as success marker
struct Success {};
using VoidResult = configgui::core::Result<Success, Error>;

namespace configgui {
namespace io {

/**
 * @class INIWriter
 * @brief Writes JSON objects to INI format
 *
 * Inverse operation of INIParser - converts JSON objects
 * back to INI format for saving configurations.
 *
 * Conversion rules:
 * - Top-level objects become sections: obj["Database"] → [Database]
 * - Nested objects become dotted sections: obj["Server"]["Http"] → [Server.Http]
 * - Arrays become indexed keys: obj["items"][0] → items[0]=value
 * - Primitive values written as key=value pairs
 * - Boolean values: true/false → "true"/"false"
 * - Numbers: 123, 3.14 → "123", "3.14"
 * - Strings: value → "value"
 *
 * Example:
 *   {
 *     "Database": {
 *       "host": "localhost",
 *       "port": 5432,
 *       "enabled": true
 *     }
 *   }
 *
 * Becomes:
 *   [Database]
 *   host=localhost
 *   port=5432
 *   enabled=true
 */
class INIWriter
{
public:
    /**
     * @brief Write JSON to INI file
     * @param file_path Path to write to
     * @param data JSON data to write
     * @return Result indicating success or error
     */
    static VoidResult writeFile(const std::string& file_path, const json& data);

    /**
     * @brief Convert JSON to INI string format
     * @param data JSON data to convert
     * @return Result containing INI string or error
     */
    static StringResult toString(const json& data);

private:
    INIWriter() = default;

    /**
     * @brief Recursively serialize JSON to INI format
     * @param obj JSON object to serialize
     * @param section_prefix Current section prefix (for nesting)
     * @return INI formatted string
     */
    static std::string serialize_json(const json& obj, const std::string& section_prefix = "");

    /**
     * @brief Serialize value to INI format string
     * @param value JSON value to serialize
     * @return String representation
     */
    static std::string serialize_value(const json& value);

    /**
     * @brief Escape special characters in string value
     * @param value String to escape
     * @return Escaped string
     */
    static std::string escape_value(const std::string& value);
};

} // namespace io
} // namespace configgui

#endif // CONFIGGUI_IO_INI_WRITER_H
