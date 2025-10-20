// SPDX-License-Identifier: MIT
// Core Model Unit Tests (Schema, ConfigurationData, FormState, ValidationError, Result)

#include <gtest/gtest.h>
#include "src/core/schema.h"
#include "src/core/configuration_data.h"
#include "src/core/form_state.h"
#include "src/core/validation_error.h"
#include "src/core/result.h"
#include "common/test_fixtures.h"

using namespace configgui::core;

class SchemaTest : public SchemaTestFixture
{
};

// ========== Schema Field Tests ==========

TEST_F(SchemaTest, SchemaField_Creation)
{
    SchemaField field;
    field.name = "username";
    field.type = "string";
    field.required = true;

    EXPECT_EQ(field.name, "username");
    EXPECT_EQ(field.type, "string");
    EXPECT_TRUE(field.required);
}

TEST_F(SchemaTest, SchemaField_WithMetadata)
{
    SchemaField field;
    field.name = "email";
    field.type = "string";
    field.description = "User email address";
    field.pattern = "^[^@]+@[^@]+\\.[^@]+$";

    EXPECT_EQ(field.name, "email");
    EXPECT_EQ(field.type, "string");
    EXPECT_EQ(field.description, "User email address");
    EXPECT_EQ(field.pattern, "^[^@]+@[^@]+\\.[^@]+$");
}

TEST_F(SchemaTest, SchemaField_WithConstraints)
{
    SchemaField field;
    field.name = "age";
    field.type = "integer";
    field.minimum = 0;
    field.maximum = 150;

    EXPECT_EQ(field.name, "age");
    EXPECT_EQ(field.type, "integer");
    EXPECT_EQ(field.minimum, 0);
    EXPECT_EQ(field.maximum, 150);
}

// ========== Schema Creation Tests ==========

TEST_F(SchemaTest, Schema_Empty)
{
    Schema schema;

    EXPECT_TRUE(schema.fields.empty());
    EXPECT_TRUE(schema.required_fields.empty());
}

TEST_F(SchemaTest, Schema_AddField)
{
    Schema schema;
    SchemaField field;
    field.name = "name";
    field.type = "string";

    schema.fields.push_back(field);

    EXPECT_EQ(schema.fields.size(), 1);
    EXPECT_EQ(schema.fields[0].name, "name");
}

TEST_F(SchemaTest, Schema_MultipleFields)
{
    Schema schema;
    
    SchemaField field1;
    field1.name = "name";
    field1.type = "string";
    schema.fields.push_back(field1);
    
    SchemaField field2;
    field2.name = "age";
    field2.type = "integer";
    schema.fields.push_back(field2);

    EXPECT_EQ(schema.fields.size(), 2);
}

TEST_F(SchemaTest, Schema_FindField)
{
    Schema schema;
    SchemaField field;
    field.name = "email";
    field.type = "string";
    schema.fields.push_back(field);

    auto found = std::find_if(schema.fields.begin(), schema.fields.end(),
        [](const SchemaField& f) { return f.name == "email"; });

    EXPECT_NE(found, schema.fields.end());
    EXPECT_EQ(found->name, "email");
}

// ========== Schema Required Fields Tests ==========

TEST_F(SchemaTest, Schema_RequiredFields)
{
    Schema schema;
    schema.required_fields = {"name", "email"};

    EXPECT_EQ(schema.required_fields.size(), 2);
    EXPECT_TRUE(std::find(schema.required_fields.begin(), 
                         schema.required_fields.end(), "name") != schema.required_fields.end());
}

// ========== ConfigurationData Tests ==========

class ConfigurationDataTest : public BaseTestFixture
{
protected:
    ConfigurationData config_;
};

TEST_F(ConfigurationDataTest, ConfigurationData_Empty)
{
    EXPECT_TRUE(config_.data.empty());
    EXPECT_FALSE(config_.isDirty());
}

TEST_F(ConfigurationDataTest, ConfigurationData_SetValue)
{
    config_.setValue("name", "John");

    EXPECT_EQ(config_.data["name"], "John");
    EXPECT_TRUE(config_.isDirty());
}

TEST_F(ConfigurationDataTest, ConfigurationData_GetValue)
{
    config_.setValue("name", "John");
    
    auto value = config_.getValue("name");

    EXPECT_EQ(value, "John");
}

TEST_F(ConfigurationDataTest, ConfigurationData_MultipleValues)
{
    config_.setValue("name", "John");
    config_.setValue("age", 30);
    config_.setValue("email", "john@example.com");

    EXPECT_EQ(config_.data.size(), 3);
    EXPECT_EQ(config_.getValue("name"), "John");
    EXPECT_EQ(config_.getValue("age"), 30);
}

TEST_F(ConfigurationDataTest, ConfigurationData_DirtyTracking)
{
    EXPECT_FALSE(config_.isDirty());
    
    config_.setValue("name", "John");
    EXPECT_TRUE(config_.isDirty());
    
    config_.markClean();
    EXPECT_FALSE(config_.isDirty());
}

TEST_F(ConfigurationDataTest, ConfigurationData_Clear)
{
    config_.setValue("name", "John");
    config_.setValue("age", 30);

    config_.data.clear();

    EXPECT_TRUE(config_.data.empty());
}

// ========== FormState Tests ==========

class FormStateTest : public BaseTestFixture
{
protected:
    FormState form_state_;
};

TEST_F(FormStateTest, FormState_Empty)
{
    EXPECT_TRUE(form_state_.getFields().empty());
}

TEST_F(FormStateTest, FormState_AddField)
{
    form_state_.addField("username", "string", true);

    EXPECT_EQ(form_state_.getFields().size(), 1);
}

TEST_F(FormStateTest, FormState_GetFieldState)
{
    form_state_.addField("username", "string", true);
    
    auto state = form_state_.getFieldState("username");

    EXPECT_NE(state, nullptr);
}

TEST_F(FormStateTest, FormState_SetFieldValue)
{
    form_state_.addField("username", "string", false);
    form_state_.setFieldValue("username", "john_doe");

    auto state = form_state_.getFieldState("username");
    EXPECT_EQ(state->value, "john_doe");
}

TEST_F(FormStateTest, FormState_MultipleFields)
{
    form_state_.addField("username", "string", true);
    form_state_.addField("email", "string", true);
    form_state_.addField("age", "integer", false);

    EXPECT_EQ(form_state_.getFields().size(), 3);
}

TEST_F(FormStateTest, FormState_IsValid_Empty)
{
    EXPECT_TRUE(form_state_.isValid());
}

TEST_F(FormStateTest, FormState_IsValid_WithErrors)
{
    form_state_.addField("username", "string", true);
    form_state_.addValidationError("username", "Required field missing");

    EXPECT_FALSE(form_state_.isValid());
}

TEST_F(FormStateTest, FormState_ClearErrors)
{
    form_state_.addField("username", "string", true);
    form_state_.addValidationError("username", "Error message");

    form_state_.clearValidationErrors("username");

    EXPECT_TRUE(form_state_.isValid());
}

// ========== ValidationError Tests ==========

class ValidationErrorTest : public BaseTestFixture
{
};

TEST_F(ValidationErrorTest, ValidationError_Creation)
{
    ValidationError error;
    error.field = "email";
    error.message = "Invalid email format";
    error.error_code = "INVALID_FORMAT";

    EXPECT_EQ(error.field, "email");
    EXPECT_EQ(error.message, "Invalid email format");
    EXPECT_EQ(error.error_code, "INVALID_FORMAT");
}

TEST_F(ValidationErrorTest, ValidationError_WithContext)
{
    ValidationError error;
    error.field = "address.street";
    error.message = "Street is required";
    error.error_code = "REQUIRED_FIELD_MISSING";
    error.context = "nested object";

    EXPECT_EQ(error.field, "address.street");
    EXPECT_EQ(error.context, "nested object");
}

TEST_F(ValidationErrorTest, ValidationError_WithCode)
{
    ValidationError error;
    error.error_code = "RANGE_EXCEEDED";
    error.message = "Value must be between 0 and 100";

    EXPECT_EQ(error.error_code, "RANGE_EXCEEDED");
}

// ========== Result<T> Tests ==========

class ResultTest : public BaseTestFixture
{
};

TEST_F(ResultTest, Result_Ok_Value)
{
    Result<int> result = Result<int>::Ok(42);

    EXPECT_TRUE(result.ok);
    EXPECT_EQ(result.value, 42);
}

TEST_F(ResultTest, Result_Ok_String)
{
    Result<std::string> result = Result<std::string>::Ok("success");

    EXPECT_TRUE(result.ok);
    EXPECT_EQ(result.value, "success");
}

TEST_F(ResultTest, Result_Error)
{
    ValidationError error;
    error.message = "Something went wrong";
    error.error_code = "ERROR";

    Result<int> result = Result<int>::Error({error});

    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.errors.size(), 1);
    EXPECT_EQ(result.errors[0].message, "Something went wrong");
}

TEST_F(ResultTest, Result_MultipleErrors)
{
    ValidationError error1;
    error1.field = "field1";
    error1.message = "Error 1";

    ValidationError error2;
    error2.field = "field2";
    error2.message = "Error 2";

    Result<std::string> result = Result<std::string>::Error({error1, error2});

    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.errors.size(), 2);
}

TEST_F(ResultTest, Result_CheckStatus)
{
    Result<int> ok_result = Result<int>::Ok(10);
    Result<int> error_result = Result<int>::Error({ValidationError{"error", "", ""}});

    EXPECT_TRUE(ok_result.ok);
    EXPECT_FALSE(error_result.ok);
}

// ========== JSON Serialization Tests ==========

TEST_F(ConfigurationDataTest, ConfigurationData_ToJson)
{
    config_.setValue("name", "John");
    config_.setValue("age", 30);
    config_.setValue("active", true);

    json data_json = config_.data;

    EXPECT_EQ(data_json["name"], "John");
    EXPECT_EQ(data_json["age"], 30);
    EXPECT_EQ(data_json["active"], true);
}

TEST_F(ConfigurationDataTest, ConfigurationData_FromJson)
{
    json data_json = {
        {"name", "John"},
        {"age", 30},
        {"email", "john@example.com"}
    };

    for (auto& [key, value] : data_json.items())
    {
        config_.setValue(key, value);
    }

    EXPECT_EQ(config_.data.size(), 3);
}

// ========== Complex Configuration Tests ==========

TEST_F(ConfigurationDataTest, ComplexConfiguration_Nested)
{
    json nested = {
        {"street", "123 Main"},
        {"city", "Springfield"}
    };
    
    config_.setValue("address", nested);

    EXPECT_TRUE(config_.data["address"].is_object());
    EXPECT_EQ(config_.data["address"]["street"], "123 Main");
}

TEST_F(ConfigurationDataTest, ComplexConfiguration_Array)
{
    json items = json::array({1, 2, 3});
    
    config_.setValue("items", items);

    EXPECT_TRUE(config_.data["items"].is_array());
    EXPECT_EQ(config_.data["items"].size(), 3);
}

// ========== Edge Cases Tests ==========

TEST_F(ValidationErrorTest, ValidationError_EmptyField)
{
    ValidationError error;
    error.field = "";
    error.message = "Some error";

    EXPECT_EQ(error.field, "");
}

TEST_F(ValidationErrorTest, ValidationError_LongMessage)
{
    ValidationError error;
    error.field = "test";
    error.message = std::string(1000, 'a');

    EXPECT_EQ(error.message.length(), 1000);
}

TEST_F(ResultTest, Result_Json_Type)
{
    Result<json> result = Result<json>::Ok(json{{"key", "value"}});

    EXPECT_TRUE(result.ok);
    EXPECT_EQ(result.value["key"], "value");
}

TEST_F(ConfigurationDataTest, ConfigurationData_Null_Value)
{
    config_.setValue("nullable", nullptr);

    EXPECT_TRUE(config_.data["nullable"].is_null());
}

// ========== State Transition Tests ==========

TEST_F(FormStateTest, FormState_FieldTransition)
{
    form_state_.addField("username", "string", true);
    
    auto initial_state = form_state_.getFieldState("username");
    EXPECT_NE(initial_state, nullptr);
    
    form_state_.setFieldValue("username", "john");
    
    auto updated_state = form_state_.getFieldState("username");
    EXPECT_EQ(updated_state->value, "john");
}

TEST_F(ConfigurationDataTest, ConfigurationData_StateTransition)
{
    EXPECT_FALSE(config_.isDirty());
    
    config_.setValue("key1", "value1");
    EXPECT_TRUE(config_.isDirty());
    
    config_.markClean();
    EXPECT_FALSE(config_.isDirty());
    
    config_.setValue("key2", "value2");
    EXPECT_TRUE(config_.isDirty());
}
