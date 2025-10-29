// SPDX-License-Identifier: MIT
// SchemaValidator - Implementation

#include "schema_validator.h"

namespace configgui {
namespace core {

SchemaValidator::SchemaValidator(const json& schema_json) : schema_(schema_json)
{
    try
    {
        validator_ = std::make_unique<nlohmann::json_schema::json_validator>(schema_json);
    }
    catch (const std::exception& /*e*/)
    {
        // Invalid schema
        validator_ = nullptr;
    }
}

bool SchemaValidator::validateSchemaFormat() const
{
    return validator_ != nullptr;
}

ValidationErrors SchemaValidator::validate(const json& data) const
{
    ValidationErrors errors;

    if (!validator_)
    {
        errors.push_back(ValidationError("", ValidationErrorType::None, "Schema validator not initialized", ""));
        return errors;
    }

    try
    {
        validator_->validate(data);
    }
    catch (const std::exception& e)
    {
        // Note: In production, parse the validation_error message for detailed field info
        errors.push_back(ValidationError("", ValidationErrorType::None, std::string("Validation failed: ") + e.what(), ""));
    }

    return errors;
}

ValidationErrors SchemaValidator::validateField(const std::string& field_name, const json& value) const
{
    ValidationErrors errors;

    if (!validator_)
    {
        return errors;
    }

    // For individual field validation, we would create a sub-schema
    // This is a simplified implementation
    try
    {
        json test_data;
        test_data[field_name] = value;
        validator_->validate(test_data);
    }
    catch (const std::exception& e)
    {
        errors.push_back(createError(field_name, ValidationErrorType::None, std::string(e.what())));
    }

    return errors;
}

ValidationError SchemaValidator::createError(const std::string& field, ValidationErrorType type,
                                              const std::string& message) const
{
    return ValidationError(field, type, message, "");
}

} // namespace core
} // namespace configgui
