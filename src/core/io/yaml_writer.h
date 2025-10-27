// SPDX-License-Identifier: MIT
// YamlWriter - YAML File Writing

#ifndef CONFIGGUI_IO_YAML_WRITER_H
#define CONFIGGUI_IO_YAML_WRITER_H

#include "../result.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::ordered_json;
using configgui::core::Result;
using YamlResult = configgui::core::Result<nlohmann::ordered_json, std::string>;

// Use an error wrapper to avoid ambiguity when T and E are the same type
struct Error { std::string message; };

using VoidResult = configgui::core::Result<nlohmann::json, Error>;
using StringResult = configgui::core::Result<std::string, Error>;

namespace configgui {
namespace io {

/**
 * @class YamlWriter
 * @brief Writes YAML configuration files
 *
 * Handles JSON to YAML conversion and file writing.
 */
class YamlWriter
{
public:
    /**
     * @brief Write JSON as YAML to file
     * @param file_path Path to write to
     * @param data JSON data to write as YAML
     * @return Result indicating success or error
     */
    static VoidResult writeFile(const std::string& file_path, const json& data);

    /**
     * @brief Convert JSON to YAML string
     * @param data JSON data to convert
     * @return Result containing YAML string or error
     */
    static StringResult toString(const json& data);

private:
    YamlWriter() = default;

    /**
     * @brief Convert JSON to YAML representation
     * @param value JSON value
     * @param indent Current indentation level
     * @return YAML string representation
     */
    static std::string jsonToYaml(const json& value, int indent = 0);

    /**
     * @brief Get indentation string
     * @param level Indentation level
     * @return Spaces for indentation
     */
    static std::string getIndent(int level);
};

} // namespace io
} // namespace configgui

#endif // CONFIGGUI_IO_YAML_WRITER_H
