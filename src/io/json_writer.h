// SPDX-License-Identifier: MIT
// JsonWriter - JSON File Writing

#ifndef CONFIGGUI_IO_JSON_WRITER_H
#define CONFIGGUI_IO_JSON_WRITER_H

#include "../core/result.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace configgui {
namespace io {

/**
 * @class JsonWriter
 * @brief Writes JSON configuration files with optimized serialization
 *
 * Handles JSON serialization and file writing with error reporting.
 * OPTIMIZATION: Uses nlohmann::json::dump() for efficient serialization
 * and stream writing to minimize string allocations.
 */
class JsonWriter
{
public:
    /**
     * @brief Write JSON to file
     * @param file_path Path to write to
     * @param data JSON data to write
     * @param pretty_print Use formatted output
     * @return Result indicating success or error
     */
    static Result<void> writeFile(const std::string& file_path, const json& data,
                                  bool pretty_print = true);

    /**
     * @brief Serialize JSON to string
     * @param data JSON data to serialize
     * @param pretty_print Use formatted output
     * @return Result containing JSON string or error
     */
    static Result<std::string> toString(const json& data, bool pretty_print = true);

private:
    JsonWriter() = default;
};

} // namespace io
} // namespace configgui

#endif // CONFIGGUI_IO_JSON_WRITER_H
