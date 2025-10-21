// SPDX-License-Identifier: MIT
// Range Validator Unit Tests

#include <gtest/gtest.h>
#include "src/validators/range_validator.h"
#include "common/test_fixtures.h"

using namespace configgui::validators;

class RangeValidatorTest : public ValidatorTestFixture
{
protected:
    RangeValidator validator_;
};

// ========== Integer Range Tests ==========

TEST_F(RangeValidatorTest, IntegerRange_WithinRange)
{
    json value = 50;
    json schema = {{"minimum", 0}, {"maximum", 100}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, IntegerRange_AtMinimum)
{
    json value = 0;
    json schema = {{"minimum", 0}, {"maximum", 100}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, IntegerRange_AtMaximum)
{
    json value = 100;
    json schema = {{"minimum", 0}, {"maximum", 100}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, IntegerRange_BelowMinimum)
{
    json value = -1;
    json schema = {{"minimum", 0}, {"maximum", 100}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "BELOW_MINIMUM");
}

TEST_F(RangeValidatorTest, IntegerRange_AboveMaximum)
{
    json value = 101;
    json schema = {{"minimum", 0}, {"maximum", 100}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "ABOVE_MAXIMUM");
}

TEST_F(RangeValidatorTest, IntegerRange_OnlyMinimum)
{
    json value = 50;
    json schema = {{"minimum", 10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, IntegerRange_OnlyMaximum)
{
    json value = 50;
    json schema = {{"maximum", 100}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Exclusive Range Tests ==========

TEST_F(RangeValidatorTest, ExclusiveMinimum_ExactValue)
{
    json value = 10;
    json schema = {{"minimum", 10}, {"exclusiveMinimum", true}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "BELOW_MINIMUM");
}

TEST_F(RangeValidatorTest, ExclusiveMinimum_AboveValue)
{
    json value = 11;
    json schema = {{"minimum", 10}, {"exclusiveMinimum", true}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, ExclusiveMaximum_ExactValue)
{
    json value = 100;
    json schema = {{"maximum", 100}, {"exclusiveMaximum", true}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "ABOVE_MAXIMUM");
}

TEST_F(RangeValidatorTest, ExclusiveMaximum_BelowValue)
{
    json value = 99;
    json schema = {{"maximum", 100}, {"exclusiveMaximum", true}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Float Range Tests ==========

TEST_F(RangeValidatorTest, FloatRange_WithinRange)
{
    json value = 3.14;
    json schema = {{"minimum", 0.0}, {"maximum", 10.0}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, FloatRange_BelowMinimum)
{
    json value = -0.5;
    json schema = {{"minimum", 0.0}, {"maximum", 10.0}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(RangeValidatorTest, FloatRange_AboveMaximum)
{
    json value = 10.5;
    json schema = {{"minimum", 0.0}, {"maximum", 10.0}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(RangeValidatorTest, FloatRange_VerySmallNumber)
{
    json value = 1e-10;
    json schema = {{"minimum", 0.0}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== String Length Tests ==========

TEST_F(RangeValidatorTest, StringLength_WithinRange)
{
    json value = "hello";
    json schema = {{"minLength", 1}, {"maxLength", 10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, StringLength_AtMinimum)
{
    json value = "a";
    json schema = {{"minLength", 1}, {"maxLength", 10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, StringLength_AtMaximum)
{
    json value = "1234567890";
    json schema = {{"minLength", 1}, {"maxLength", 10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, StringLength_TooShort)
{
    json value = "";
    json schema = {{"minLength", 1}, {"maxLength", 10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "STRING_TOO_SHORT");
}

TEST_F(RangeValidatorTest, StringLength_TooLong)
{
    json value = "12345678901";
    json schema = {{"minLength", 1}, {"maxLength", 10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "STRING_TOO_LONG");
}

TEST_F(RangeValidatorTest, StringLength_OnlyMinimum)
{
    json value = "test";
    json schema = {{"minLength", 3}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, StringLength_OnlyMaximum)
{
    json value = "test";
    json schema = {{"maxLength", 10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Non-String, Non-Number Tests ==========

TEST_F(RangeValidatorTest, BooleanWithRange_IgnoresConstraints)
{
    json value = true;
    json schema = {{"minimum", 0}, {"maximum", 1}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, ArrayWithRange_IgnoresConstraints)
{
    json value = json::array({1, 2, 3});
    json schema = {{"minimum", 0}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, ObjectWithRange_IgnoresConstraints)
{
    json value = json::object();
    json schema = {{"maximum", 10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Edge Cases ==========

TEST_F(RangeValidatorTest, EdgeCase_NegativeRange)
{
    json value = -50;
    json schema = {{"minimum", -100}, {"maximum", -10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, EdgeCase_ZeroRange)
{
    json value = 0;
    json schema = {{"minimum", 0}, {"maximum", 0}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, EdgeCase_VeryLargeRange)
{
    json value = 1000000;
    json schema = {{"minimum", 0}, {"maximum", 10000000}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, EdgeCase_UnicodeStringLength)
{
    // "こんにちは" is 5 characters despite being multiple bytes
    json value = "こんにちは";
    json schema = {{"minLength", 5}, {"maxLength", 10}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, EdgeCase_EmptyStringWithMinimum)
{
    json value = "";
    json schema = {{"minLength", 0}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RangeValidatorTest, EdgeCase_LongString)
{
    std::string long_str(1000, 'a');
    json value = long_str;
    json schema = {{"maxLength", 1000}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Combined Errors Tests ==========

TEST_F(RangeValidatorTest, MultipleErrors_BothRangeViolations)
{
    // This validator checks one at a time, so should only report first error
    json value = -5;
    json schema = {{"minimum", 0}, {"maximum", 100}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "BELOW_MINIMUM");
}

// ========== No Constraint Tests ==========

TEST_F(RangeValidatorTest, NoConstraints_AnyNumber)
{
    json schema = json::object();  // No constraints

    ASSERT_VALID_RESULT(validator_.validate(0, schema));
    ASSERT_VALID_RESULT(validator_.validate(100, schema));
    ASSERT_VALID_RESULT(validator_.validate(-100, schema));
    ASSERT_VALID_RESULT(validator_.validate(3.14, schema));
}

TEST_F(RangeValidatorTest, NoConstraints_AnyString)
{
    json schema = json::object();  // No constraints

    ASSERT_VALID_RESULT(validator_.validate("", schema));
    ASSERT_VALID_RESULT(validator_.validate("test", schema));
    ASSERT_VALID_RESULT(validator_.validate("a very long string", schema));
}
