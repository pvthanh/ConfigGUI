// SPDX-License-Identifier: MIT
// IValidator - Base Validator Interface

#ifndef CONFIGGUI_VALIDATORS_IVALIDATOR_H
#define CONFIGGUI_VALIDATORS_IVALIDATOR_H

#include "../core/result.h"
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

namespace configgui {
namespace validators {

/**
 * @struct ValidationError
 * @brief Single validation error result
 */
struct ValidationError {
    std::string field;
    std::string message;
    std::string error_code;
};

/**
 * @struct ValidationResult
 * @brief Complete validation result
 */
struct ValidationResult {
    bool is_valid;
    std::vector<ValidationError> errors;

    explicit operator bool() const { return is_valid; }
};

/**
 * @class IValidator
 * @brief Base interface for all validators
 *
 * Defines common interface for field and schema validators.
 * All validators must implement this interface.
 */
class IValidator
{
public:
    virtual ~IValidator() = default;

    /**
     * @brief Validate a value
     * @param value JSON value to validate
     * @param schema Schema constraints
     * @return Validation result
     */
    virtual ValidationResult validate(const json& value, const json& schema) = 0;

    /**
     * @brief Get validator name
     * @return Name of this validator
     */
    virtual std::string getName() const = 0;

protected:
    /**
     * @brief Create validation error
     * @param field Field name
     * @param message Error message
     * @param code Error code
     * @return ValidationError
     */
    static ValidationError makeError(const std::string& field,
                                     const std::string& message,
                                     const std::string& code = "VALIDATION_ERROR")
    {
        return ValidationError{field, message, code};
    }

    /**
     * @brief Create successful validation result
     * @return ValidationResult with is_valid=true
     */
    static ValidationResult success()
    {
        return ValidationResult{true, {}};
    }

    /**
     * @brief Create failed validation result
     * @param errors List of validation errors
     * @return ValidationResult with is_valid=false
     */
    static ValidationResult failure(const std::vector<ValidationError>& errors)
    {
        return ValidationResult{false, errors};
    }
};

} // namespace validators
} // namespace configgui

#endif // CONFIGGUI_VALIDATORS_IVALIDATOR_H
