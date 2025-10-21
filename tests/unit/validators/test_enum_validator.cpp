// SPDX-License-Identifier: MIT
// Enum Validator Unit Tests

#include <gtest/gtest.h>
#include "src/validators/enum_validator.h"
#include "common/test_fixtures.h"

using namespace configgui::validators;

class EnumValidatorTest : public ValidatorTestFixture
{
protected:
    EnumValidator validator_;
};

// ========== String Enum Tests ==========

TEST_F(EnumValidatorTest, StringEnum_ValidValue)
{
    json value = "dev";
    json schema = {{"enum", json::array({"dev", "staging", "prod"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, StringEnum_InvalidValue)
{
    json value = "test";
    json schema = {{"enum", json::array({"dev", "staging", "prod"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "ENUM_MISMATCH");
}

TEST_F(EnumValidatorTest, StringEnum_CaseSensitive)
{
    json value = "DEV";
    json schema = {{"enum", json::array({"dev", "staging", "prod"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(EnumValidatorTest, StringEnum_FirstValue)
{
    json value = "dev";
    json schema = {{"enum", json::array({"dev", "staging", "prod"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, StringEnum_LastValue)
{
    json value = "prod";
    json schema = {{"enum", json::array({"dev", "staging", "prod"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Number Enum Tests ==========

TEST_F(EnumValidatorTest, NumberEnum_ValidInteger)
{
    json value = 1;
    json schema = {{"enum", json::array({0, 1, 2, 3})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, NumberEnum_ValidFloat)
{
    json value = 3.14;
    json schema = {{"enum", json::array({1.0, 2.5, 3.14, 4.0})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, NumberEnum_InvalidValue)
{
    json value = 5;
    json schema = {{"enum", json::array({0, 1, 2, 3})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(EnumValidatorTest, NumberEnum_NegativeValue)
{
    json value = -1;
    json schema = {{"enum", json::array({-2, -1, 0, 1})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, NumberEnum_Zero)
{
    json value = 0;
    json schema = {{"enum", json::array({0, 1, 2})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Boolean Enum Tests ==========

TEST_F(EnumValidatorTest, BooleanEnum_True)
{
    json value = true;
    json schema = {{"enum", json::array({true, false})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, BooleanEnum_False)
{
    json value = false;
    json schema = {{"enum", json::array({true, false})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, BooleanEnum_OnlyTrue)
{
    json value = false;
    json schema = {{"enum", json::array({true})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Null Enum Tests ==========

TEST_F(EnumValidatorTest, NullEnum_ValidNull)
{
    json value = nullptr;
    json schema = {{"enum", json::array({nullptr})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, NullEnum_WithOtherValues)
{
    json value = nullptr;
    json schema = {{"enum", json::array({"value", 1, nullptr})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Mixed Type Enum Tests ==========

TEST_F(EnumValidatorTest, MixedTypeEnum_String)
{
    json value = "test";
    json schema = {{"enum", json::array({"test", 1, true, nullptr})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, MixedTypeEnum_Number)
{
    json value = 1;
    json schema = {{"enum", json::array({"test", 1, true, nullptr})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, MixedTypeEnum_Boolean)
{
    json value = true;
    json schema = {{"enum", json::array({"test", 1, true, nullptr})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, MixedTypeEnum_Invalid)
{
    json value = 3.14;
    json schema = {{"enum", json::array({"test", 1, true, nullptr})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Single Value Enum Tests ==========

TEST_F(EnumValidatorTest, SingleValueEnum_Valid)
{
    json value = "only_option";
    json schema = {{"enum", json::array({"only_option"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, SingleValueEnum_Invalid)
{
    json value = "other_option";
    json schema = {{"enum", json::array({"only_option"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== No Enum Constraint Tests ==========

TEST_F(EnumValidatorTest, NoEnum_AnyValue)
{
    json schema = json::object();  // No enum field

    ASSERT_VALID_RESULT(validator_.validate("value", schema));
    ASSERT_VALID_RESULT(validator_.validate(42, schema));
    ASSERT_VALID_RESULT(validator_.validate(true, schema));
    ASSERT_VALID_RESULT(validator_.validate(nullptr, schema));
}

// ========== Empty Enum Tests ==========

TEST_F(EnumValidatorTest, EmptyEnum_AnyValue)
{
    json value = "anything";
    json schema = {{"enum", json::array({})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Large Enum Tests ==========

TEST_F(EnumValidatorTest, LargeEnum_ValidValue)
{
    json::array large_enum;
    for (int i = 0; i < 100; ++i)
    {
        large_enum.push_back(i);
    }

    json value = 50;
    json schema = {{"enum", large_enum}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, LargeEnum_InvalidValue)
{
    json::array large_enum;
    for (int i = 0; i < 100; ++i)
    {
        large_enum.push_back(i);
    }

    json value = 150;
    json schema = {{"enum", large_enum}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Type Mismatch in Enum Tests ==========

TEST_F(EnumValidatorTest, TypeMismatch_IntegerVsFloat)
{
    json value = 1.0;
    json schema = {{"enum", json::array({1, 2, 3})}};

    // Note: JSON treats 1.0 and 1 as equal for comparison
    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(EnumValidatorTest, TypeMismatch_StringVsNumber)
{
    json value = "1";
    json schema = {{"enum", json::array({1, 2, 3})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}
