// SPDX-License-Identifier: MIT
// JsonWriter - JSON File Writing

#ifndef CONFIGGUI_IO_JSON_WRITER_H
#define CONFIGGUI_IO_JSON_WRITER_H

#include "../result.h"
#include <nlohmann/json.hpp>
#include <string>

// Use ordered_json to preserve field order as defined in schema
using json = nlohmann::ordered_json;
using configgui::core::Result;
using JsonResult = configgui::core::Result<nlohmann::ordered_json, std::string>;

// Use an error wrapper to avoid ambiguity when T and E are the same type
struct Error { std::string message; };

using StringResult = configgui::core::Result<std::string, Error>;

// For void results, we use an empty struct as success marker
struct Success {};
using VoidResult = configgui::core::Result<Success, Error>;

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
    static VoidResult writeFile(const std::string& file_path, const json& data,
                                  bool pretty_print = true);

    /**
     * @brief Serialize JSON to string
     * @param data JSON data to serialize
     * @param pretty_print Use formatted output
     * @return Result containing JSON string or error
     */
    static StringResult toString(const json& data, bool pretty_print = true);

private:
    JsonWriter() = default;
};

} // namespace io
} // namespace configgui

#endif // CONFIGGUI_IO_JSON_WRITER_H
