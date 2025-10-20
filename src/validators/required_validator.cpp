// SPDX-License-Identifier: MIT
// RequiredValidator - Implementation

#include "required_validator.h"

namespace configgui {
namespace validators {

ValidationResult RequiredValidator::validate(const json& value, const json& schema)
{
    if (!value.is_object() || !schema.contains("required"))
    {
        return success();
    }

    const auto& required = schema["required"];
    if (!required.is_array())
    {
        return success();
    }

    std::vector<ValidationError> errors;

    for (const auto& field : required)
    {
        if (!field.is_string())
        {
            continue;
        }

        const auto field_name = field.get<std::string>();

        // Check if field exists and is not null
        if (!value.contains(field_name) || value[field_name].is_null())
        {
            std::string message = "Field '" + field_name + "' is required";
            errors.push_back(makeError(field_name, message, "REQUIRED_FIELD_MISSING"));
        }
    }

    if (errors.empty())
    {
        return success();
    }
    return failure(errors);
}

} // namespace validators
} // namespace configgui
