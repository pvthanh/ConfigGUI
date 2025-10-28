// SPDX-License-Identifier: MIT
// INIReader - INI File Reading

#ifndef CONFIGGUI_IO_INI_READER_H
#define CONFIGGUI_IO_INI_READER_H

#include "../result.h"
#include <nlohmann/json.hpp>
#include <string>

// Use ordered_json to preserve field order as defined in schema
using json = nlohmann::ordered_json;
using configgui::core::Result;
using JsonResult = configgui::core::Result<nlohmann::ordered_json, std::string>;

namespace configgui {
namespace io {

/**
 * @class INIReader
 * @brief Reads INI configuration files
 *
 * Wrapper around INIParser providing a consistent interface
 * with other readers (JsonReader, YamlReader).
 *
 * Parses INI format into JSON structure with support for:
 * - Section headers: [SectionName]
 * - Nested sections: [Server.Http]
 * - Type inference: bool, int, float, string
 * - Arrays: key[0]=value, key[1]=value
 * - Comments: ; and # prefixed lines
 * - Escape sequences: \n, \r, \t
 *
 * Example:
 *   [Database]
 *   host=localhost
 *   port=5432
 *   enabled=true
 *
 * Becomes:
 *   {
 *     "Database": {
 *       "host": "localhost",
 *       "port": 5432,
 *       "enabled": true
 *     }
 *   }
 */
class INIReader
{
public:
    /**
     * @brief Read INI from file
     * @param file_path Path to INI file
     * @return Result containing parsed JSON or error
     */
    static JsonResult readFile(const std::string& file_path);

    /**
     * @brief Read INI from string
     * @param ini_string INI string content
     * @return Result containing parsed JSON or error
     */
    static JsonResult readString(const std::string& ini_string);

private:
    INIReader() = default;
};

} // namespace io
} // namespace configgui

#endif // CONFIGGUI_IO_INI_READER_H
