// SPDX-License-Identifier: MIT
// SchemaLoader - Implementation

#include "schema_loader.h"
#include <fstream>
#include <QFile>
#include <iostream>

namespace configgui {
namespace core {

Result<JSONSchema, FileError> SchemaLoader::loadSchema(const QString& file_path)
{
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (file.error() == QFile::NoError)
        {
            return Result<JSONSchema, FileError>(FileError::NotFound);
        }
        return Result<JSONSchema, FileError>(FileError::PermissionDenied);
    }

    const QByteArray content = file.readAll();
    file.close();

    return loadSchemaFromString(QString::fromUtf8(content));
}

Result<JSONSchema, FileError> SchemaLoader::loadSchemaFromString(const QString& json_string)
{
    try
    {
        const json schema_json = json::parse(json_string.toStdString());

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
