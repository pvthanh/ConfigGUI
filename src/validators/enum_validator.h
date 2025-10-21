// SPDX-License-Identifier: MIT
// EnumValidator - Enum Validation

#ifndef CONFIGGUI_VALIDATORS_ENUM_VALIDATOR_H
#define CONFIGGUI_VALIDATORS_ENUM_VALIDATOR_H

#include "ivalidator.h"

namespace configgui {
namespace validators {

/**
 * @class EnumValidator
 * @brief Validates values against enum constraints
 *
 * Checks that values are in the allowed set of values.
 * Supports "enum" constraint from JSON Schema.
 */
class EnumValidator : public IValidator
{
public:
    ~EnumValidator() override = default;

    /**
     * @brief Validate enum constraint
     * @param value Value to check
     * @param schema Schema with enum constraint
     * @return ValidationResult
     */
    ValidationResult validate(const json& value, const json& schema) override;

    /**
     * @brief Get validator name
     * @return "EnumValidator"
     */
    std::string getName() const override { return "EnumValidator"; }

private:
    /**
     * @brief Convert JSON value to string for display
     * @param value JSON value
     * @return String representation
     */
    std::string valueToString(const json& value) const;
};

} // namespace validators
} // namespace configgui

#endif // CONFIGGUI_VALIDATORS_ENUM_VALIDATOR_H
