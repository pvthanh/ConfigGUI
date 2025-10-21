// SPDX-License-Identifier: MIT
// JsonReader - JSON File Reading

#ifndef CONFIGGUI_IO_JSON_READER_H
#define CONFIGGUI_IO_JSON_READER_H

#include "../core/result.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace configgui {
namespace io {

/**
 * @class JsonReader
 * @brief Reads JSON configuration files
 *
 * Handles JSON file parsing with error reporting.
 */
class JsonReader
{
public:
    /**
     * @brief Read JSON from file
     * @param file_path Path to JSON file
     * @return Result containing parsed JSON or error
     */
    static Result<json> readFile(const std::string& file_path);

    /**
     * @brief Read JSON from string
     * @param json_string JSON string content
     * @return Result containing parsed JSON or error
     */
    static Result<json> readString(const std::string& json_string);

private:
    JsonReader() = default;
};

} // namespace io
} // namespace configgui

#endif // CONFIGGUI_IO_JSON_READER_H
