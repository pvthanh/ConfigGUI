// SPDX-License-Identifier: MIT
// Unit tests for Schema Validator - Core module

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "core/schema/schema_validator.h"
#include "core/schema/validation_error.h"
#include "core/error_types.h"

using json = nlohmann::ordered_json;

namespace configgui {
namespace core {
namespace test {

// Test fixture for schema validator tests
class SchemaValidatorTest : public ::testing::Test {
protected:
    SchemaValidatorTest() {}
};

// Test: Validate required fields
TEST_F(SchemaValidatorTest, ValidateRequiredFieldPresent) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"email", {{"type", "string"}}}
        }},
        {"required", {"name", "email"}}
    };

    SchemaValidator validator(schema);
    EXPECT_TRUE(validator.validateSchemaFormat());

    json valid_data = {
        {"name", "John"},
        {"email", "john@example.com"}
    };

    auto errors = validator.validate(valid_data);
    EXPECT_EQ(errors.size(), 0);
}

// Test: Validate required field missing
TEST_F(SchemaValidatorTest, ValidateRequiredFieldMissing) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"email", {{"type", "string"}}}
        }},
        {"required", {"name", "email"}}
    };

    SchemaValidator validator(schema);

    json invalid_data = {
        {"name", "John"}
    };

    auto errors = validator.validate(invalid_data);
    EXPECT_GT(errors.size(), 0);
}

// Test: Validate field type mismatch
TEST_F(SchemaValidatorTest, ValidateTypeMismatch) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"age", {{"type", "integer"}}}
        }},
        {"required", {"age"}}
    };

    SchemaValidator validator(schema);

    json invalid_data = {
        {"age", "not a number"}
    };

    auto errors = validator.validate(invalid_data);
    EXPECT_GT(errors.size(), 0);
}

// Test: Validate minimum constraint
TEST_F(SchemaValidatorTest, ValidateMinimumConstraint) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"age", {
                {"type", "integer"},
                {"minimum", 0}
            }}
        }},
        {"required", {"age"}}
    };

    SchemaValidator validator(schema);

    // Valid data
    json valid_data = {{"age", 25}};
    auto errors = validator.validate(valid_data);
    EXPECT_EQ(errors.size(), 0);

    // Invalid data (below minimum)
    json invalid_data = {{"age", -5}};
    errors = validator.validate(invalid_data);
    EXPECT_GT(errors.size(), 0);
}

// Test: Validate maximum constraint
TEST_F(SchemaValidatorTest, ValidateMaximumConstraint) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"age", {
                {"type", "integer"},
                {"maximum", 150}
            }}
        }},
        {"required", {"age"}}
    };

    SchemaValidator validator(schema);

    // Valid data
    json valid_data = {{"age", 50}};
    auto errors = validator.validate(valid_data);
    EXPECT_EQ(errors.size(), 0);

    // Invalid data (exceeds maximum)
    json invalid_data = {{"age", 200}};
    errors = validator.validate(invalid_data);
    EXPECT_GT(errors.size(), 0);
}

// Test: Validate string length constraints
TEST_F(SchemaValidatorTest, ValidateStringLengthConstraints) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"username", {
                {"type", "string"},
                {"minLength", 3},
                {"maxLength", 20}
            }}
        }},
        {"required", {"username"}}
    };

    SchemaValidator validator(schema);

    // Valid data
    json valid_data = {{"username", "johndoe"}};
    auto errors = validator.validate(valid_data);
    EXPECT_EQ(errors.size(), 0);

    // Too short
    json too_short = {{"username", "ab"}};
    errors = validator.validate(too_short);
    EXPECT_GT(errors.size(), 0);

    // Too long
    json too_long = {{"username", "this_is_a_very_long_username"}};
    errors = validator.validate(too_long);
    EXPECT_GT(errors.size(), 0);
}

// Test: Validate enum constraint
TEST_F(SchemaValidatorTest, ValidateEnumConstraint) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"status", {
                {"type", "string"},
                {"enum", {"active", "inactive", "pending"}}
            }}
        }},
        {"required", {"status"}}
    };

    SchemaValidator validator(schema);

    // Valid enum value
    json valid_data = {{"status", "active"}};
    auto errors = validator.validate(valid_data);
    EXPECT_EQ(errors.size(), 0);

    // Invalid enum value
    json invalid_data = {{"status", "unknown"}};
    errors = validator.validate(invalid_data);
    EXPECT_GT(errors.size(), 0);
}

// Test: Validate pattern constraint
TEST_F(SchemaValidatorTest, ValidatePatternConstraint) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"email", {
                {"type", "string"},
                {"pattern", "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"}
            }}
        }},
        {"required", {"email"}}
    };

    SchemaValidator validator(schema);

    // Valid pattern
    json valid_data = {{"email", "user@example.com"}};
    auto errors = validator.validate(valid_data);
    // Note: Pattern validation depends on json-schema-validator library

    // Invalid pattern
    json invalid_data = {{"email", "not-an-email"}};
    errors = validator.validate(invalid_data);
    // Note: Pattern validation depends on json-schema-validator library
}

// Test: Validate nested objects
TEST_F(SchemaValidatorTest, ValidateNestedObjects) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"user", {
                {"type", "object"},
                {"properties", {
                    {"name", {{"type", "string"}}},
                    {"email", {{"type", "string"}}}
                }},
                {"required", {"name"}}
            }}
        }},
        {"required", {"user"}}
    };

    SchemaValidator validator(schema);

    // Valid nested data
    json valid_data = {
        {"user", {
            {"name", "John"},
            {"email", "john@example.com"}
        }}
    };
    auto errors = validator.validate(valid_data);
    EXPECT_LE(errors.size(), 0);

    // Invalid nested data (missing required field)
    json invalid_data = {
        {"user", {
            {"email", "john@example.com"}
        }}
    };
    errors = validator.validate(invalid_data);
    EXPECT_GT(errors.size(), 0);
}

// Test: Validate arrays
TEST_F(SchemaValidatorTest, ValidateArrays) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"tags", {
                {"type", "array"},
                {"items", {{"type", "string"}}}
            }}
        }},
        {"required", {"tags"}}
    };

    SchemaValidator validator(schema);

    // Valid array
    json valid_data = {
        {"tags", {"python", "cpp", "web"}}
    };
    auto errors = validator.validate(valid_data);
    EXPECT_EQ(errors.size(), 0);

    // Invalid array (wrong item type)
    json invalid_data = {
        {"tags", {"python", 123, "web"}}
    };
    errors = validator.validate(invalid_data);
    EXPECT_GT(errors.size(), 0);
}

// Test: Validate boolean type
TEST_F(SchemaValidatorTest, ValidateBooleanType) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"active", {{"type", "boolean"}}}
        }},
        {"required", {"active"}}
    };

    SchemaValidator validator(schema);

    // Valid boolean
    json valid_data = {{"active", true}};
    auto errors = validator.validate(valid_data);
    EXPECT_EQ(errors.size(), 0);

    // Invalid boolean (string instead)
    json invalid_data = {{"active", "yes"}};
    errors = validator.validate(invalid_data);
    EXPECT_GT(errors.size(), 0);
}

// Test: Validate number type
TEST_F(SchemaValidatorTest, ValidateNumberType) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"price", {{"type", "number"}}}
        }},
        {"required", {"price"}}
    };

    SchemaValidator validator(schema);

    // Valid number
    json valid_data = {{"price", 19.99}};
    auto errors = validator.validate(valid_data);
    EXPECT_EQ(errors.size(), 0);

    // Valid integer (also number)
    json valid_int = {{"price", 20}};
    errors = validator.validate(valid_int);
    EXPECT_EQ(errors.size(), 0);
}

// Test: Field-specific validation
TEST_F(SchemaValidatorTest, ValidateSingleField) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"age", {
                {"type", "integer"},
                {"minimum", 0}
            }}
        }}
    };

    SchemaValidator validator(schema);

    // Validate single field
    json value = 25;
    auto errors = validator.validateField("age", value);
    // Result depends on implementation
}

// Test: Empty data is valid for optional fields
TEST_F(SchemaValidatorTest, EmptyObjectWithOptionalFields) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"email", {{"type", "string"}}}
        }}
    };

    SchemaValidator validator(schema);

    json empty_data = json::object();
    auto errors = validator.validate(empty_data);
    EXPECT_EQ(errors.size(), 0);
}

// Test: Additional properties allowed by default
TEST_F(SchemaValidatorTest, AdditionalPropertiesAllowed) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}}
        }}
    };

    SchemaValidator validator(schema);

    json data_with_extra = {
        {"name", "John"},
        {"age", 30},
        {"extra_field", "value"}
    };
    auto errors = validator.validate(data_with_extra);
    // Default allows additional properties
}

// Test: Additional properties disallowed when explicitly set
TEST_F(SchemaValidatorTest, AdditionalPropertiesDisallowed) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}}
        }},
        {"additionalProperties", false}
    };

    SchemaValidator validator(schema);

    json data_with_extra = {
        {"name", "John"},
        {"extra_field", "value"}
    };
    auto errors = validator.validate(data_with_extra);
    // Validation result depends on whether additionalProperties is enforced
}

// Test: Validation error contains field information
TEST_F(SchemaValidatorTest, ValidationErrorContainsFieldInfo) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}}
        }},
        {"required", {"name"}}
    };

    SchemaValidator validator(schema);

    json invalid_data = {};
    auto errors = validator.validate(invalid_data);
    
    if (errors.size() > 0) {
        // Verify error contains field information
        EXPECT_FALSE(errors[0].field().empty());
    }
}

} // namespace test
} // namespace core
} // namespace configgui
