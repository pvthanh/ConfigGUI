// SPDX-License-Identifier: MIT
// SchemaLoader - Implementation

#include "schema_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace configgui {
namespace core {

Result<JSONSchema, FileError> SchemaLoader::loadSchema(const std::string& file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        return Result<JSONSchema, FileError>(FileError::NotFound);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return loadSchemaFromString(buffer.str());
}

Result<JSONSchema, FileError> SchemaLoader::loadSchemaFromString(const std::string& json_string)
{
    try
    {
        const json schema_json = json::parse(json_string);

        if (!validateJson(schema_json))
        {
            return Result<JSONSchema, FileError>(FileError::InvalidJson);
        }

        JSONSchema schema(schema_json);
        return Result<JSONSchema, FileError>(schema);
    }
    catch (const json::exception& /*e*/)
    {
        return Result<JSONSchema, FileError>(FileError::ParseError);
    }
    catch (const std::exception& /*e*/)
    {
        return Result<JSONSchema, FileError>(FileError::UnknownError);
    }
}

bool SchemaLoader::validateJson(const json& schema_json) const
{
    // Basic JSON schema validation (Draft 7)
    // Check for required "type" field or other schema indicators
    if (schema_json.is_object())
    {
        return true;
    }
    return false;
}

} // namespace core
} // namespace configgui
