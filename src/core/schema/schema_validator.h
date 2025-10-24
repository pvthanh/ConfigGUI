// SPDX-License-Identifier: MIT
// SchemaValidator - Validates configuration against schema

#pragma once

#include "../result.h"
#include "schema.h"
#include "../error_types.h"
#include "validation_error.h"
#include <QString>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

using json = nlohmann::ordered_json;

namespace configgui {
namespace core {

/// @brief Validates JSON data against a JSON schema (Draft 7)
class SchemaValidator
{
public:
    explicit SchemaValidator(const json& schema_json);
    ~SchemaValidator() = default;

    // Non-copyable, non-movable
    SchemaValidator(const SchemaValidator&) = delete;
    SchemaValidator& operator=(const SchemaValidator&) = delete;
    SchemaValidator(SchemaValidator&&) = delete;
    SchemaValidator& operator=(SchemaValidator&&) = delete;

    /// @brief Validate schema format is proper JSON Schema Draft 7
    [[nodiscard]] bool validateSchemaFormat() const;

    /// @brief Validate data against schema
    [[nodiscard]] ValidationErrors validate(const json& data) const;

    /// @brief Validate single field against schema
    [[nodiscard]] ValidationErrors validateField(const QString& field_name, const json& value) const;

    /// @brief Get the schema
    [[nodiscard]] const json& schema() const { return schema_; }

private:
    json schema_;
    std::unique_ptr<nlohmann::json_schema::json_validator> validator_;

    /// @brief Create ValidationError from schema violation
    [[nodiscard]] ValidationError createError(const QString& field, ValidationErrorType type,
                                               const QString& message) const;
};

} // namespace core
} // namespace configgui
