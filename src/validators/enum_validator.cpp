// SPDX-License-Identifier: MIT
// EnumValidator - Implementation

#include "enum_validator.h"

namespace configgui {
namespace validators {

ValidationResult EnumValidator::validate(const json& value, const json& schema)
{
    if (!schema.contains("enum"))
    {
        return success();
    }

    const auto& enums = schema["enum"];
    if (!enums.is_array())
    {
        return success();
    }

    // Check if value is in enum list
    for (const auto& enum_val : enums)
    {
        if (value == enum_val)
        {
            return success();
        }
    }

    // Value not found in enum - build error message
    std::string allowed_values;
    for (size_t i = 0; i < enums.size(); ++i)
    {
        if (i > 0)
        {
            allowed_values += ", ";
        }
        allowed_values += valueToString(enums[i]);
    }

    std::string message = "Value must be one of: " + allowed_values;
    return failure({makeError("value", message, "ENUM_MISMATCH")});
}

std::string EnumValidator::valueToString(const json& value) const
{
    if (value.is_string())
    {
        return "\"" + value.get<std::string>() + "\"";
    }
    if (value.is_number())
    {
        return value.dump();
    }
    if (value.is_boolean())
    {
        return value.get<bool>() ? "true" : "false";
    }
    if (value.is_null())
    {
        return "null";
    }

    return value.dump();
}

} // namespace validators
} // namespace configgui
