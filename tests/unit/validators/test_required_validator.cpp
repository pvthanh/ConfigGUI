// SPDX-License-Identifier: MIT
// Required Validator Unit Tests

#include <gtest/gtest.h>
#include "src/validators/required_validator.h"
#include "common/test_fixtures.h"

using namespace configgui::validators;

class RequiredValidatorTest : public ValidatorTestFixture
{
protected:
    RequiredValidator validator_;
};

// ========== Single Required Field Tests ==========

TEST_F(RequiredValidatorTest, SingleRequiredField_Present)
{
    json value = {{"name", "John"}};
    json schema = {{"required", json::array({"name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, SingleRequiredField_Missing)
{
    json value = json::object();
    json schema = {{"required", json::array({"name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].field, "name");
    EXPECT_EQ(result.errors[0].error_code, "REQUIRED_FIELD_MISSING");
}

TEST_F(RequiredValidatorTest, SingleRequiredField_Null)
{
    json value = {{"name", nullptr}};
    json schema = {{"required", json::array({"name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(RequiredValidatorTest, SingleRequiredField_Empty)
{
    json value = {{"name", ""}};
    json schema = {{"required", json::array({"name"})}};

    ValidationResult result = validator_.validate(value, schema);

    // Empty string is still present (not null)
    ASSERT_VALID_RESULT(result);
}

// ========== Multiple Required Fields Tests ==========

TEST_F(RequiredValidatorTest, MultipleRequiredFields_AllPresent)
{
    json value = {
        {"name", "John"},
        {"email", "john@example.com"},
        {"age", 30}
    };
    json schema = {{"required", json::array({"name", "email", "age"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, MultipleRequiredFields_OneMissing)
{
    json value = {
        {"name", "John"},
        {"age", 30}
    };
    json schema = {{"required", json::array({"name", "email", "age"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].field, "email");
}

TEST_F(RequiredValidatorTest, MultipleRequiredFields_MultipleMissing)
{
    json value = {{"name", "John"}};
    json schema = {{"required", json::array({"name", "email", "age"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 2);
}

TEST_F(RequiredValidatorTest, MultipleRequiredFields_AllMissing)
{
    json value = json::object();
    json schema = {{"required", json::array({"name", "email", "age"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 3);
}

// ========== Extra Fields Tests ==========

TEST_F(RequiredValidatorTest, ExtraFields_Allowed)
{
    json value = {
        {"name", "John"},
        {"email", "john@example.com"},
        {"phone", "555-1234"}  // Extra field
    };
    json schema = {{"required", json::array({"name", "email"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, ExtraFields_MultipleExtra)
{
    json value = {
        {"name", "John"},
        {"address", "123 Main St"},
        {"phone", "555-1234"},
        {"notes", "Test user"}
    };
    json schema = {{"required", json::array({"name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Non-Object Value Tests ==========

TEST_F(RequiredValidatorTest, NonObjectValue_String)
{
    json value = "not an object";
    json schema = {{"required", json::array({"name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, NonObjectValue_Array)
{
    json value = json::array({1, 2, 3});
    json schema = {{"required", json::array({"name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, NonObjectValue_Number)
{
    json value = 42;
    json schema = {{"required", json::array({"name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== No Required Constraint Tests ==========

TEST_F(RequiredValidatorTest, NoRequired_AnyObject)
{
    json schema = json::object();  // No required field

    ASSERT_VALID_RESULT(validator_.validate(json::object(), schema));
    ASSERT_VALID_RESULT(validator_.validate({{"name", "John"}}, schema));
    ASSERT_VALID_RESULT(validator_.validate({{"random", "field"}}, schema));
}

// ========== Empty Required List Tests ==========

TEST_F(RequiredValidatorTest, EmptyRequired_AnyObject)
{
    json value = json::object();
    json schema = {{"required", json::array({})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Nested Objects Tests ==========

TEST_F(RequiredValidatorTest, NestedObject_RequiredAtTopLevel)
{
    json value = {
        {"name", "John"},
        {"address", {
            {"street", "123 Main"},
            {"city", "Springfield"}
        }}
    };
    json schema = {{"required", json::array({"name", "address"})}};

    ValidationResult result = validator_.validate(value, schema);

    // This validator only checks top-level required fields
    ASSERT_VALID_RESULT(result);
}

// ========== Special Field Names Tests ==========

TEST_F(RequiredValidatorTest, SpecialFieldNames_WithSpaces)
{
    json value = {{"user name", "John"}};
    json schema = {{"required", json::array({"user name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, SpecialFieldNames_WithUnicode)
{
    json value = {{"名前", "John"}};
    json schema = {{"required", json::array({"名前"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, SpecialFieldNames_WithSymbols)
{
    json value = {{"-_@#$", "John"}};
    json schema = {{"required", json::array({"-_@#$"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Case Sensitivity Tests ==========

TEST_F(RequiredValidatorTest, CaseSensitive_WrongCase)
{
    json value = {{"name", "John"}};
    json schema = {{"required", json::array({"Name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(RequiredValidatorTest, CaseSensitive_CorrectCase)
{
    json value = {{"Name", "John"}};
    json schema = {{"required", json::array({"Name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Values of Different Types Tests ==========

TEST_F(RequiredValidatorTest, Values_String)
{
    json value = {{"name", "John"}};
    json schema = {{"required", json::array({"name"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, Values_Number)
{
    json value = {{"age", 30}};
    json schema = {{"required", json::array({"age"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, Values_Boolean)
{
    json value = {{"active", true}};
    json schema = {{"required", json::array({"active"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, Values_Array)
{
    json value = {{"tags", json::array({"a", "b"})}};
    json schema = {{"required", json::array({"tags"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, Values_Object)
{
    json value = {{"metadata", {{"key", "value"}}}};
    json schema = {{"required", json::array({"metadata"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, Values_EmptyArray)
{
    json value = {{"items", json::array({})}};
    json schema = {{"required", json::array({"items"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, Values_EmptyObject)
{
    json value = {{"data", json::object()}};
    json schema = {{"required", json::array({"data"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Zero Value Tests ==========

TEST_F(RequiredValidatorTest, ZeroValue_Integer)
{
    json value = {{"count", 0}};
    json schema = {{"required", json::array({"count"})}};

    ValidationResult result = validator_.validate(value, schema);

    // 0 is a valid value, not missing
    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, ZeroValue_Float)
{
    json value = {{"price", 0.0}};
    json schema = {{"required", json::array({"price"})}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, FalseValue_Boolean)
{
    json value = {{"flag", false}};
    json schema = {{"required", json::array({"flag"})}};

    ValidationResult result = validator_.validate(value, schema);

    // false is a valid value, not missing
    ASSERT_VALID_RESULT(result);
}

// ========== Large Object Tests ==========

TEST_F(RequiredValidatorTest, LargeObject_ManyFields)
{
    json value = json::object();
    for (int i = 0; i < 100; ++i)
    {
        value["field_" + std::to_string(i)] = i;
    }

    json required_fields = json::array();
    for (int i = 0; i < 50; ++i)
    {
        required_fields.push_back("field_" + std::to_string(i));
    }

    json schema = {{"required", required_fields}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(RequiredValidatorTest, LargeObject_ManyRequiredMissing)
{
    json value = json::object();
    for (int i = 0; i < 10; ++i)
    {
        value["field_" + std::to_string(i)] = i;
    }

    json required_fields = json::array();
    for (int i = 0; i < 50; ++i)
    {
        required_fields.push_back("field_" + std::to_string(i));
    }

    json schema = {{"required", required_fields}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 40);
}
