// SPDX-License-Identifier: MIT
// RangeValidator - Implementation

#include "range_validator.h"
#include <sstream>
#include <iomanip>

namespace configgui {
namespace validators {

ValidationResult RangeValidator::validate(const json& value, const json& schema)
{
    if (value.is_number())
    {
        return validateNumericRange(value, schema);
    }
    if (value.is_string())
    {
        return validateStringLength(value, schema);
    }

    return success();
}

ValidationResult RangeValidator::validateNumericRange(const json& value, const json& schema)
{
    std::vector<ValidationError> errors;

    // OPTIMIZATION: Early exit on first error if no minimum/maximum constraints
    if (!schema.contains("minimum") && !schema.contains("maximum"))
    {
        return success();
    }

    if (schema.contains("minimum"))
    {
        const auto minimum = schema["minimum"];
        const bool exclusive = schema.contains("exclusiveMinimum") && schema["exclusiveMinimum"].is_boolean()
                                   ? schema["exclusiveMinimum"].get<bool>()
                                   : false;

        if (minimum.is_number())
        {
            const auto min_val = minimum.get<double>();
            const auto val = value.get<double>();

            if (exclusive && val <= min_val)
            {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << "Value must be greater than " << min_val;
                errors.push_back(makeError("value", oss.str(), "BELOW_MINIMUM"));
                // OPTIMIZATION: Return early for exclusive minimum failure
                return failure(errors);
            }
            else if (!exclusive && val < min_val)
            {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << "Value must be at least " << min_val;
                errors.push_back(makeError("value", oss.str(), "BELOW_MINIMUM"));
                // OPTIMIZATION: Return early for inclusive minimum failure
                return failure(errors);
            }
        }
    }

    if (schema.contains("maximum"))
    {
        const auto maximum = schema["maximum"];
        const bool exclusive = schema.contains("exclusiveMaximum") && schema["exclusiveMaximum"].is_boolean()
                                   ? schema["exclusiveMaximum"].get<bool>()
                                   : false;

        if (maximum.is_number())
        {
            const auto max_val = maximum.get<double>();
            const auto val = value.get<double>();

            if (exclusive && val >= max_val)
            {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << "Value must be less than " << max_val;
                errors.push_back(makeError("value", oss.str(), "ABOVE_MAXIMUM"));
                return failure(errors);
            }
            else if (!exclusive && val > max_val)
            {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << "Value must be at most " << max_val;
                errors.push_back(makeError("value", oss.str(), "ABOVE_MAXIMUM"));
                return failure(errors);
            }
        }
    }

    if (errors.empty())
    {
        return success();
    }
    return failure(errors);
}

ValidationResult RangeValidator::validateStringLength(const json& value, const json& schema)
{
    if (!value.is_string())
    {
        return success();
    }

    std::vector<ValidationError> errors;
    const auto str = value.get<std::string>();
    const auto length = str.length();

    if (schema.contains("minLength"))
    {
        if (schema["minLength"].is_number_integer())
        {
            const auto min_length = schema["minLength"].get<int>();
            if (static_cast<int>(length) < min_length)
            {
                std::ostringstream oss;
                oss << "String must be at least " << min_length << " characters";
                errors.push_back(makeError("value", oss.str(), "STRING_TOO_SHORT"));
            }
        }
    }

    if (schema.contains("maxLength"))
    {
        if (schema["maxLength"].is_number_integer())
        {
            const auto max_length = schema["maxLength"].get<int>();
            if (static_cast<int>(length) > max_length)
            {
                std::ostringstream oss;
                oss << "String must be at most " << max_length << " characters";
                errors.push_back(makeError("value", oss.str(), "STRING_TOO_LONG"));
            }
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
