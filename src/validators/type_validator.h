// SPDX-License-Identifier: MIT
// TypeValidator - Type Validation

#ifndef CONFIGGUI_VALIDATORS_TYPE_VALIDATOR_H
#define CONFIGGUI_VALIDATORS_TYPE_VALIDATOR_H

#include "ivalidator.h"

namespace configgui {
namespace validators {

/**
 * @class TypeValidator
 * @brief Validates JSON value types
 *
 * Checks that values match the schema type constraints.
 * Supports: string, integer, number, boolean, object, array
 */
class TypeValidator : public IValidator
{
public:
    ~TypeValidator() override = default;

    /**
     * @brief Validate type of value
     * @param value Value to check
     * @param schema Schema with type constraint
     * @return ValidationResult
     */
    ValidationResult validate(const json& value, const json& schema) override;

    /**
     * @brief Get validator name
     * @return "TypeValidator"
     */
    std::string getName() const override { return "TypeValidator"; }

private:
    /**
     * @brief Check if value matches type string
     * @param value Value to check
     * @param type_str Type string from schema
     * @return True if matches
     */
    bool matchesType(const json& value, const std::string& type_str) const;

    /**
     * @brief Get JSON type string
     * @param value Value to inspect
     * @return Type name as string
     */
    std::string getJsonType(const json& value) const;
};

} // namespace validators
} // namespace configgui

#endif // CONFIGGUI_VALIDATORS_TYPE_VALIDATOR_H
