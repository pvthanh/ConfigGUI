// SPDX-License-Identifier: MIT
// YamlWriter - Implementation

#include "yaml_writer.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace configgui {
namespace io {

Result<void> YamlWriter::writeFile(const std::string& file_path, const json& data)
{
    try
    {
        const auto yaml_result = toString(data);
        if (!yaml_result.isOk())
        {
            return Result<void>::Error(yaml_result.getError());
        }

        std::ofstream file(file_path);
        if (!file.is_open())
        {
            return Result<void>::Error("Cannot open file for writing: " + file_path);
        }

        file << yaml_result.getValue();
        file.close();

        return Result<void>::Ok();
    }
    catch (const std::exception& e)
    {
        return Result<void>::Error(std::string("Failed to write YAML file: ") + e.what());
    }
}

Result<std::string> YamlWriter::toString(const json& data)
{
    try
    {
        const auto yaml_str = jsonToYaml(data);
        return Result<std::string>::Ok(yaml_str);
    }
    catch (const std::exception& e)
    {
        return Result<std::string>::Error(std::string("Failed to convert to YAML: ") + e.what());
    }
}

std::string YamlWriter::jsonToYaml(const json& value, int indent)
{
    std::ostringstream oss;
    const std::string indent_str = getIndent(indent);

    if (value.is_object())
    {
        if (value.empty())
        {
            oss << "{}";
        }
        else
        {
            for (auto it = value.begin(); it != value.end(); ++it)
            {
                if (it != value.begin())
                {
                    oss << "\n";
                }

                oss << indent_str << it.key() << ": ";

                if (it.value().is_object() || it.value().is_array())
                {
                    oss << jsonToYaml(it.value(), indent + 2);
                }
                else if (it.value().is_string())
                {
                    oss << it.value().get<std::string>();
                }
                else if (it.value().is_null())
                {
                    oss << "null";
                }
                else
                {
                    oss << it.value().dump();
                }
            }
        }
    }
    else if (value.is_array())
    {
        if (value.empty())
        {
            oss << "[]";
        }
        else
        {
            for (size_t i = 0; i < value.size(); ++i)
            {
                if (i > 0)
                {
                    oss << "\n";
                }

                oss << indent_str << "- ";

                if (value[i].is_object() || value[i].is_array())
                {
                    oss << jsonToYaml(value[i], indent + 2);
                }
                else if (value[i].is_string())
                {
                    oss << value[i].get<std::string>();
                }
                else if (value[i].is_null())
                {
                    oss << "null";
                }
                else
                {
                    oss << value[i].dump();
                }
            }
        }
    }
    else if (value.is_string())
    {
        oss << value.get<std::string>();
    }
    else if (value.is_null())
    {
        oss << "null";
    }
    else
    {
        oss << value.dump();
    }

    return oss.str();
}

std::string YamlWriter::getIndent(int level)
{
    return std::string(level, ' ');
}

} // namespace io
} // namespace configgui
