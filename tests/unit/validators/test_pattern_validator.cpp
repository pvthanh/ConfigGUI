// SPDX-License-Identifier: MIT
// Pattern Validator Unit Tests

#include <gtest/gtest.h>
#include "src/validators/pattern_validator.h"
#include "common/test_fixtures.h"

using namespace configgui::validators;

class PatternValidatorTest : public ValidatorTestFixture
{
protected:
    PatternValidator validator_;
};

// ========== Basic Pattern Tests ==========

TEST_F(PatternValidatorTest, SimplePattern_ValidMatch)
{
    json value = "hello";
    json schema = {{"pattern", "^hello$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, SimplePattern_NoMatch)
{
    json value = "world";
    json schema = {{"pattern", "^hello$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
    EXPECT_EQ(result.errors[0].error_code, "PATTERN_MISMATCH");
}

TEST_F(PatternValidatorTest, SimplePattern_PartialMatch)
{
    json value = "hello world";
    json schema = {{"pattern", "^hello$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Email Pattern Tests ==========

TEST_F(PatternValidatorTest, EmailPattern_ValidEmail)
{
    std::string email_pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
    
    json value = "user@example.com";
    json schema = {{"pattern", email_pattern}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, EmailPattern_InvalidEmail_NoAt)
{
    std::string email_pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
    
    json value = "user.example.com";
    json schema = {{"pattern", email_pattern}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(PatternValidatorTest, EmailPattern_InvalidEmail_NoDomain)
{
    std::string email_pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
    
    json value = "user@";
    json schema = {{"pattern", email_pattern}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(PatternValidatorTest, EmailPattern_ValidWithNumbers)
{
    std::string email_pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
    
    json value = "user123@example456.co.uk";
    json schema = {{"pattern", email_pattern}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Phone Number Pattern Tests ==========

TEST_F(PatternValidatorTest, PhonePattern_USFormat)
{
    json value = "555-123-4567";
    json schema = {{"pattern", "^\\d{3}-\\d{3}-\\d{4}$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, PhonePattern_InvalidFormat)
{
    json value = "5551234567";
    json schema = {{"pattern", "^\\d{3}-\\d{3}-\\d{4}$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Alphanumeric Pattern Tests ==========

TEST_F(PatternValidatorTest, AlphanumericPattern_ValidAlphanumeric)
{
    json value = "abc123XYZ";
    json schema = {{"pattern", "^[a-zA-Z0-9]+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, AlphanumericPattern_WithSpecialChar)
{
    json value = "abc-123";
    json schema = {{"pattern", "^[a-zA-Z0-9]+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(PatternValidatorTest, AlphanumericPattern_WithSpace)
{
    json value = "abc 123";
    json schema = {{"pattern", "^[a-zA-Z0-9]+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Case Sensitivity Tests ==========

TEST_F(PatternValidatorTest, CaseSensitivePattern_LowercaseRequired)
{
    json value = "Hello";
    json schema = {{"pattern", "^[a-z]+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(PatternValidatorTest, CaseSensitivePattern_LowercaseMatch)
{
    json value = "hello";
    json schema = {{"pattern", "^[a-z]+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Quantifier Tests ==========

TEST_F(PatternValidatorTest, QuantifierPlus_OneOrMore)
{
    json value = "aaa";
    json schema = {{"pattern", "^a+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, QuantifierPlus_Zero)
{
    json value = "";
    json schema = {{"pattern", "^a+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(PatternValidatorTest, QuantifierStar_ZeroOrMore)
{
    json value = "aaa";
    json schema = {{"pattern", "^a*$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, QuantifierStar_EmptyString)
{
    json value = "";
    json schema = {{"pattern", "^a*$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, QuantifierQuestion_ZeroOrOne)
{
    json value = "a";
    json schema = {{"pattern", "^a?$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, QuantifierQuestion_Multiple)
{
    json value = "aa";
    json schema = {{"pattern", "^a?$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Character Class Tests ==========

TEST_F(PatternValidatorTest, CharacterClass_Digit)
{
    json value = "12345";
    json schema = {{"pattern", "^\\d+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, CharacterClass_NonDigit)
{
    json value = "abcde";
    json schema = {{"pattern", "^\\d+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(PatternValidatorTest, CharacterClass_Word)
{
    json value = "hello_world";
    json schema = {{"pattern", "^\\w+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Anchors Tests ==========

TEST_F(PatternValidatorTest, Anchor_StartOfString)
{
    json value = "hello world";
    json schema = {{"pattern", "^hello"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, Anchor_StartOfString_NoMatch)
{
    json value = " hello world";
    json schema = {{"pattern", "^hello"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

TEST_F(PatternValidatorTest, Anchor_EndOfString)
{
    json value = "hello world";
    json schema = {{"pattern", "world$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, Anchor_EndOfString_NoMatch)
{
    json value = "hello world ";
    json schema = {{"pattern", "world$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}

// ========== Non-String Values Tests ==========

TEST_F(PatternValidatorTest, NonStringValue_Number)
{
    json value = 12345;
    json schema = {{"pattern", "^\\d+$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, NonStringValue_Boolean)
{
    json value = true;
    json schema = {{"pattern", ".*"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, NonStringValue_Null)
{
    json value = nullptr;
    json schema = {{"pattern", ".*"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== No Pattern Constraint Tests ==========

TEST_F(PatternValidatorTest, NoPattern_AnyString)
{
    json schema = json::object();  // No pattern

    ASSERT_VALID_RESULT(validator_.validate("hello", schema));
    ASSERT_VALID_RESULT(validator_.validate("12345", schema));
    ASSERT_VALID_RESULT(validator_.validate("!@#$%", schema));
}

// ========== Special Characters Tests ==========

TEST_F(PatternValidatorTest, SpecialCharacters_Escaped)
{
    json value = "price: $19.99";
    json schema = {{"pattern", "price: \\$[0-9]+\\.[0-9]{2}"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, SpecialCharacters_Brackets)
{
    json value = "[test]";
    json schema = {{"pattern", "^\\[.*\\]$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

// ========== Complex Patterns Tests ==========

TEST_F(PatternValidatorTest, ComplexPattern_URLFormat)
{
    json value = "https://example.com/path";
    json schema = {{"pattern", "^https?://[a-zA-Z0-9.-]+(:[0-9]+)?(/[a-zA-Z0-9._~:/?#\\[\\]@!$&'()*+,;=-]*)?$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, ComplexPattern_HexColor)
{
    json value = "#FF5733";
    json schema = {{"pattern", "^#[0-9A-Fa-f]{6}$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_VALID_RESULT(result);
}

TEST_F(PatternValidatorTest, ComplexPattern_HexColor_Invalid)
{
    json value = "#FF57";
    json schema = {{"pattern", "^#[0-9A-Fa-f]{6}$"}};

    ValidationResult result = validator_.validate(value, schema);

    ASSERT_INVALID_RESULT(result, 1);
}
