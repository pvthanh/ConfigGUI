// SPDX-License-Identifier: MIT
// YamlReader - YAML File Reading

#ifndef CONFIGGUI_IO_YAML_READER_H
#define CONFIGGUI_IO_YAML_READER_H

#include "../core/result.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
using configgui::core::Result;
using YamlResult = configgui::core::Result<nlohmann::json, std::string>;

namespace configgui {
namespace io {

/**
 * @class YamlReader
 * @brief Reads YAML configuration files
 *
 * Handles YAML file parsing and conversion to JSON.
 * Note: Uses libyaml for parsing.
 */
class YamlReader
{
public:
    /**
     * @brief Read YAML from file
     * @param file_path Path to YAML file
     * @return Result containing parsed JSON or error
     */
    static YamlResult readFile(const std::string& file_path);

    /**
     * @brief Read YAML from string
     * @param yaml_string YAML string content
     * @return Result containing parsed JSON or error
     */
    static YamlResult readString(const std::string& yaml_string);

private:
    YamlReader() = default;

    /**
     * @brief Convert YAML value to JSON
     * @param value YAML value
     * @return JSON representation
     */
    static json yamlToJson(const std::string& yaml_content);
};

} // namespace io
} // namespace configgui

#endif // CONFIGGUI_IO_YAML_READER_H
