// SPDX-License-Identifier: MIT
// RequiredValidator - Required Fields Validation

#ifndef CONFIGGUI_VALIDATORS_REQUIRED_VALIDATOR_H
#define CONFIGGUI_VALIDATORS_REQUIRED_VALIDATOR_H

#include "ivalidator.h"

namespace configgui {
namespace validators {

/**
 * @class RequiredValidator
 * @brief Validates required fields are present
 *
 * Checks that all required fields are present and non-null.
 * Operates on object schemas with required field lists.
 */
class RequiredValidator : public IValidator
{
public:
    ~RequiredValidator() override = default;

    /**
     * @brief Validate required fields
     * @param value Object to check
     * @param schema Schema with required field list
     * @return ValidationResult
     */
    ValidationResult validate(const json& value, const json& schema) override;

    /**
     * @brief Get validator name
     * @return "RequiredValidator"
     */
    std::string getName() const override { return "RequiredValidator"; }
};

} // namespace validators
} // namespace configgui

#endif // CONFIGGUI_VALIDATORS_REQUIRED_VALIDATOR_H
