// SPDX-License-Identifier: MIT
// YamlWriter - Implementation

#include "yaml_writer.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace configgui {
namespace io {

VoidResult YamlWriter::writeFile(const std::string& file_path, const json& data)
{
    try
    {
        const auto yaml_result = toString(data);
        if (yaml_result.is_failure())
        {
            return VoidResult(yaml_result.error());
        }

        std::ofstream file(file_path);
        if (!file.is_open())
        {
            return VoidResult(Error{"Cannot open file for writing: " + file_path});
        }

        file << yaml_result.value();
        file.close();

        return VoidResult(json::object());
    }
    catch (const std::exception& e)
    {
        return VoidResult(Error{"Failed to write YAML file: " + std::string(e.what())});
    }
}

StringResult YamlWriter::toString(const json& data)
{
    try
    {
        const auto yaml_str = jsonToYaml(data);
        return StringResult(yaml_str);
    }
    catch (const std::exception& e)
    {
        return StringResult(Error{"Failed to convert to YAML: " + std::string(e.what())});
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
    return std::string(static_cast<size_t>(level), ' ');
}

} // namespace io
} // namespace configgui
