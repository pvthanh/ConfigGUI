// SPDX-License-Identifier: MIT
// Type Validator Unit Tests

#include <gtest/gtest.h>
#include "src/validators/type_validator.h"
#include "common/test_fixtures.h"

using namespace configgui::validators;

class TypeValidatorTest : public ValidatorTestFixture
{
protected:
    TypeValidator validator_;
};

// ========== String Type Tests ==========

TEST_F(TypeValidatorTest, ValidateStringType_ValidString)
{
    json value = "hello";
    json schema = {{"type", "string"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateStringType_InvalidNumber)
{
    json value = 42;
    json schema = {{"type", "string"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "TYPE_MISMATCH");
}

TEST_F(TypeValidatorTest, ValidateStringType_InvalidBoolean)
{
    json value = true;
    json schema = {{"type", "string"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(TypeValidatorTest, ValidateStringType_EmptyString)
{
    json value = "";
    json schema = {{"type", "string"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Integer Type Tests ==========

TEST_F(TypeValidatorTest, ValidateIntegerType_ValidInteger)
{
    json value = 42;
    json schema = {{"type", "integer"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateIntegerType_ZeroValue)
{
    json value = 0;
    json schema = {{"type", "integer"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateIntegerType_NegativeInteger)
{
    json value = -100;
    json schema = {{"type", "integer"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateIntegerType_FloatNotInteger)
{
    json value = 3.14;
    json schema = {{"type", "integer"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(TypeValidatorTest, ValidateIntegerType_StringInvalid)
{
    json value = "42";
    json schema = {{"type", "integer"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Number Type Tests ==========

TEST_F(TypeValidatorTest, ValidateNumberType_ValidInteger)
{
    json value = 42;
    json schema = {{"type", "number"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateNumberType_ValidFloat)
{
    json value = 3.14;
    json schema = {{"type", "number"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateNumberType_NegativeFloat)
{
    json value = -2.71;
    json schema = {{"type", "number"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateNumberType_StringInvalid)
{
    json value = "3.14";
    json schema = {{"type", "number"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Boolean Type Tests ==========

TEST_F(TypeValidatorTest, ValidateBooleanType_True)
{
    json value = true;
    json schema = {{"type", "boolean"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateBooleanType_False)
{
    json value = false;
    json schema = {{"type", "boolean"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateBooleanType_NumberInvalid)
{
    json value = 1;
    json schema = {{"type", "boolean"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(TypeValidatorTest, ValidateBooleanType_StringInvalid)
{
    json value = "true";
    json schema = {{"type", "boolean"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Array Type Tests ==========

TEST_F(TypeValidatorTest, ValidateArrayType_EmptyArray)
{
    json value = json::array();
    json schema = {{"type", "array"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateArrayType_StringArray)
{
    json value = json::array({"a", "b", "c"});
    json schema = {{"type", "array"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateArrayType_MixedArray)
{
    json value = json::array({1, "two", 3.0, true});
    json schema = {{"type", "array"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateArrayType_ObjectInvalid)
{
    json value = json::object();
    json schema = {{"type", "array"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Object Type Tests ==========

TEST_F(TypeValidatorTest, ValidateObjectType_EmptyObject)
{
    json value = json::object();
    json schema = {{"type", "object"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateObjectType_WithProperties)
{
    json value = {{"name", "test"}, {"age", 30}};
    json schema = {{"type", "object"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateObjectType_ArrayInvalid)
{
    json value = json::array({1, 2, 3});
    json schema = {{"type", "object"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Null Type Tests ==========

TEST_F(TypeValidatorTest, ValidateNullType_ValidNull)
{
    json value = nullptr;
    json schema = {{"type", "null"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateNullType_StringInvalid)
{
    json value = "null";
    json schema = {{"type", "null"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Multiple Types (Union) Tests ==========

TEST_F(TypeValidatorTest, ValidateMultipleTypes_StringValid)
{
    json value = "hello";
    json schema = {{"type", json::array({"string", "number"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateMultipleTypes_NumberValid)
{
    json value = 42;
    json schema = {{"type", json::array({"string", "number"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateMultipleTypes_BooleanInvalid)
{
    json value = true;
    json schema = {{"type", json::array({"string", "number"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(TypeValidatorTest, ValidateMultipleTypes_StringOrNull_String)
{
    json value = "test";
    json schema = {{"type", json::array({"string", "null"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, ValidateMultipleTypes_StringOrNull_Null)
{
    json value = nullptr;
    json schema = {{"type", json::array({"string", "null"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== No Type Constraint Tests ==========

TEST_F(TypeValidatorTest, NoTypeConstraint_AnyValue)
{
    json schema = json::object();  // No type field

    // Should pass for any value
    ASSERT_VALID_RESULT(validator_.validate(42, schema));
    ASSERT_VALID_RESULT(validator_.validate("string", schema));
    ASSERT_VALID_RESULT(validator_.validate(true, schema));
    ASSERT_VALID_RESULT(validator_.validate(json::array(), schema));
    ASSERT_VALID_RESULT(validator_.validate(json::object(), schema));
}

// ========== Edge Cases ==========

TEST_F(TypeValidatorTest, EdgeCase_LargeInteger)
{
    json value = 9223372036854775807LL;  // Max int64
    json schema = {{"type", "integer"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, EdgeCase_VerySmallFloat)
{
    json value = 1e-308;
    json schema = {{"type", "number"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, EdgeCase_UnicodeString)
{
    json value = "こんにちは世界";  // "Hello World" in Japanese
    json schema = {{"type", "string"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(TypeValidatorTest, EdgeCase_NestedArray)
{
    json value = json::array({json::array({1, 2}), json::array({3, 4})});
    json schema = {{"type", "array"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}
