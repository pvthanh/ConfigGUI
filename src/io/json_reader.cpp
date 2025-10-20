// SPDX-License-Identifier: MIT
// JsonReader - Implementation

#include "json_reader.h"
#include <fstream>
#include <sstream>

namespace configgui {
namespace io {

Result<json> JsonReader::readFile(const std::string& file_path)
{
    try
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            return Result<json>::Error("Cannot open file: " + file_path);
        }

        json data;
        file >> data;
        file.close();

        return Result<json>::Ok(data);
    }
    catch (const std::exception& e)
    {
        return Result<json>::Error(std::string("Failed to read JSON file: ") + e.what());
    }
}

Result<json> JsonReader::readString(const std::string& json_string)
{
    try
    {
        const auto data = json::parse(json_string);
        return Result<json>::Ok(data);
    }
    catch (const json::parse_error& e)
    {
        std::ostringstream oss;
        oss << "JSON parse error at line " << e.byte << ": " << e.what();
        return Result<json>::Error(oss.str());
    }
    catch (const std::exception& e)
    {
        return Result<json>::Error(std::string("Failed to parse JSON: ") + e.what());
    }
}

} // namespace io
} // namespace configgui
