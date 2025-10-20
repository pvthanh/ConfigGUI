// SPDX-License-Identifier: MIT
// YamlWriter - YAML File Writing

#ifndef CONFIGGUI_IO_YAML_WRITER_H
#define CONFIGGUI_IO_YAML_WRITER_H

#include "../core/result.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

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
    static Result<void> writeFile(const std::string& file_path, const json& data);

    /**
     * @brief Convert JSON to YAML string
     * @param data JSON data to convert
     * @return Result containing YAML string or error
     */
    static Result<std::string> toString(const json& data);

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
