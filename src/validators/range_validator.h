// SPDX-License-Identifier: MIT
// RangeValidator - Range Validation

#ifndef CONFIGGUI_VALIDATORS_RANGE_VALIDATOR_H
#define CONFIGGUI_VALIDATORS_RANGE_VALIDATOR_H

#include "ivalidator.h"

namespace configgui {
namespace validators {

/**
 * @class RangeValidator
 * @brief Validates numeric ranges and string lengths
 *
 * Checks minimum, maximum, and length constraints.
 * Works with: numbers, integers, and strings.
 */
class RangeValidator : public IValidator
{
public:
    ~RangeValidator() override = default;

    /**
     * @brief Validate range constraints
     * @param value Value to check
     * @param schema Schema with min/max constraints
     * @return ValidationResult
     */
    ValidationResult validate(const json& value, const json& schema) override;

    /**
     * @brief Get validator name
     * @return "RangeValidator"
     */
    std::string getName() const override { return "RangeValidator"; }

private:
    /**
     * @brief Validate numeric range
     * @param value Number value
     * @param schema Schema constraints
     * @return ValidationResult
     */
    ValidationResult validateNumericRange(const json& value, const json& schema);

    /**
     * @brief Validate string length
     * @param value String value
     * @param schema Schema constraints
     * @return ValidationResult
     */
    ValidationResult validateStringLength(const json& value, const json& schema);
};

} // namespace validators
} // namespace configgui

#endif // CONFIGGUI_VALIDATORS_RANGE_VALIDATOR_H
