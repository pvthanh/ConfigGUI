// SPDX-License-Identifier: MIT
// SchemaLoader - Loads and parses JSON schemas

#pragma once

#include "../result.h"
#include "schema.h"
#include "../error_types.h"
#include <QString>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

namespace configgui {
namespace core {

/// @brief Loads JSON schema files and creates SchemaValidator instances
class SchemaLoader
{
public:
    SchemaLoader() = default;
    ~SchemaLoader() = default;

    // Non-copyable, non-movable (MISRA C++ compliance)
    SchemaLoader(const SchemaLoader&) = delete;
    SchemaLoader& operator=(const SchemaLoader&) = delete;
    SchemaLoader(SchemaLoader&&) = delete;
    SchemaLoader& operator=(SchemaLoader&&) = delete;

    /// @brief Load schema from file path
    [[nodiscard]] Result<JSONSchema, FileError> loadSchema(const QString& file_path);

    /// @brief Load schema from JSON string
    [[nodiscard]] Result<JSONSchema, FileError> loadSchemaFromString(const QString& json_string);

private:
    /// @brief Validate schema is proper JSON
    [[nodiscard]] bool validateJson(const json& schema_json) const;
};

} // namespace core
} // namespace configgui
