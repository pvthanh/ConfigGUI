// SPDX-License-Identifier: MIT
// TypeValidator - Implementation

#include "type_validator.h"

namespace configgui {
namespace validators {

ValidationResult TypeValidator::validate(const json& value, const json& schema)
{
    if (!schema.contains("type"))
    {
        return success();
    }

    const auto type_constraint = schema["type"];

    if (type_constraint.is_string())
    {
        const auto type_str = type_constraint.get<std::string>();
        if (!matchesType(value, type_str))
        {
            const auto actual_type = getJsonType(value);
            std::string message = "Expected type '" + type_str + "' but got '" + actual_type + "'";
            return failure({makeError("value", message, "TYPE_MISMATCH")});
        }
    }
    else if (type_constraint.is_array())
    {
        bool found_match = false;
        std::string allowed_types;

        for (const auto& type : type_constraint)
        {
            if (type.is_string())
            {
                const auto type_str = type.get<std::string>();
                if (!allowed_types.empty())
                {
                    allowed_types += ", ";
                }
                allowed_types += type_str;

                if (matchesType(value, type_str))
                {
                    found_match = true;
                    break;
                }
            }
        }

        if (!found_match)
        {
            const auto actual_type = getJsonType(value);
            std::string message = "Expected one of [" + allowed_types + "] but got '" + actual_type + "'";
            return failure({makeError("value", message, "TYPE_MISMATCH")});
        }
    }

    return success();
}

bool TypeValidator::matchesType(const json& value, const std::string& type_str) const
{
    if (type_str == "string")
    {
        return value.is_string();
    }
    if (type_str == "integer")
    {
        return value.is_number_integer();
    }
    if (type_str == "number")
    {
        return value.is_number();
    }
    if (type_str == "boolean")
    {
        return value.is_boolean();
    }
    if (type_str == "object")
    {
        return value.is_object();
    }
    if (type_str == "array")
    {
        return value.is_array();
    }
    if (type_str == "null")
    {
        return value.is_null();
    }

    return false;
}

std::string TypeValidator::getJsonType(const json& value) const
{
    if (value.is_string())
    {
        return "string";
    }
    if (value.is_number_integer())
    {
        return "integer";
    }
    if (value.is_number_float())
    {
        return "number";
    }
    if (value.is_boolean())
    {
        return "boolean";
    }
    if (value.is_object())
    {
        return "object";
    }
    if (value.is_array())
    {
        return "array";
    }
    if (value.is_null())
    {
        return "null";
    }

    return "unknown";
}

} // namespace validators
} // namespace configgui
