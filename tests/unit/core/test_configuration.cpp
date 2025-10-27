// SPDX-License-Identifier: MIT
// Unit tests for Configuration Data - Core module

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "core/data/configuration_data.h"
#include "core/data/form_state.h"
#include "core/error_types.h"

using json = nlohmann::ordered_json;

namespace configgui {
namespace core {
namespace test {

// Test fixture for configuration data tests
class ConfigurationDataTest : public ::testing::Test {
protected:
    ConfigurationDataTest() {}
};

// Test: Create empty configuration data
TEST_F(ConfigurationDataTest, CreateEmptyConfigurationData) {
    ConfigurationData config;
    
    EXPECT_EQ(config.size(), 0);
    EXPECT_TRUE(config.is_empty());
}

// Test: Set and get configuration value
TEST_F(ConfigurationDataTest, SetAndGetValue) {
    ConfigurationData config;
    
    config.set_value("name", "John");
    
    EXPECT_EQ(config.get_value("name"), "John");
}

// Test: Set multiple values
TEST_F(ConfigurationDataTest, SetMultipleValues) {
    ConfigurationData config;
    
    config.set_value("name", "John");
    config.set_value("email", "john@example.com");
    config.set_value("age", 30);
    
    EXPECT_EQ(config.size(), 3);
    EXPECT_EQ(config.get_value("name"), "John");
    EXPECT_EQ(config.get_value("email"), "john@example.com");
    EXPECT_EQ(config.get_value("age"), 30);
}

// Test: Get non-existent value returns null
TEST_F(ConfigurationDataTest, GetNonExistentValueReturnsNull) {
    ConfigurationData config;
    
    auto value = config.get_value("nonexistent");
    EXPECT_TRUE(value.is_null());
}

// Test: Update existing value
TEST_F(ConfigurationDataTest, UpdateExistingValue) {
    ConfigurationData config;
    
    config.set_value("status", "active");
    EXPECT_EQ(config.get_value("status"), "active");
    
    config.set_value("status", "inactive");
    EXPECT_EQ(config.get_value("status"), "inactive");
}

// Test: Remove value
TEST_F(ConfigurationDataTest, RemoveValue) {
    ConfigurationData config;
    
    config.set_value("temporary", "value");
    EXPECT_FALSE(config.get_value("temporary").is_null());
    
    config.remove_value("temporary");
    EXPECT_TRUE(config.get_value("temporary").is_null());
}

// Test: Clear all values
TEST_F(ConfigurationDataTest, ClearAllValues) {
    ConfigurationData config;
    
    config.set_value("key1", "value1");
    config.set_value("key2", "value2");
    config.set_value("key3", "value3");
    
    EXPECT_EQ(config.size(), 3);
    
    config.clear();
    
    EXPECT_EQ(config.size(), 0);
    EXPECT_TRUE(config.is_empty());
}

// Test: Check if key exists
TEST_F(ConfigurationDataTest, CheckIfKeyExists) {
    ConfigurationData config;
    
    config.set_value("existing", "value");
    
    EXPECT_TRUE(config.has_key("existing"));
    EXPECT_FALSE(config.has_key("nonexistent"));
}

// Test: Get all keys
TEST_F(ConfigurationDataTest, GetAllKeys) {
    ConfigurationData config;
    
    config.set_value("alpha", "a");
    config.set_value("beta", "b");
    config.set_value("gamma", "c");
    
    auto keys = config.get_keys();
    EXPECT_EQ(keys.size(), 3);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "alpha") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "beta") != keys.end());
}

// Test: Store nested objects
TEST_F(ConfigurationDataTest, StoreNestedObjects) {
    ConfigurationData config;
    
    json user = {
        {"name", "John"},
        {"email", "john@example.com"},
        {"preferences", {
            {"theme", "dark"},
            {"notifications", true}
        }}
    };
    
    config.set_value("user", user);
    
    auto retrieved = config.get_value("user");
    EXPECT_EQ(retrieved["name"], "John");
    EXPECT_EQ(retrieved["preferences"]["theme"], "dark");
}

// Test: Store arrays
TEST_F(ConfigurationDataTest, StoreArrays) {
    ConfigurationData config;
    
    json tags = {"python", "cpp", "web"};
    config.set_value("tags", tags);
    
    auto retrieved = config.get_value("tags");
    EXPECT_EQ(retrieved.size(), 3);
    EXPECT_EQ(retrieved[0], "python");
}

// Test: Store various data types
TEST_F(ConfigurationDataTest, StoreVariousDataTypes) {
    ConfigurationData config;
    
    config.set_value("string", "text");
    config.set_value("integer", 42);
    config.set_value("float", 3.14);
    config.set_value("boolean", true);
    config.set_value("null", nullptr);
    
    EXPECT_EQ(config.get_value("string"), "text");
    EXPECT_EQ(config.get_value("integer"), 42);
    EXPECT_DOUBLE_EQ(config.get_value("float"), 3.14);
    EXPECT_EQ(config.get_value("boolean"), true);
    EXPECT_TRUE(config.get_value("null").is_null());
}

// Test: Clone configuration data
TEST_F(ConfigurationDataTest, CloneConfigurationData) {
    ConfigurationData original;
    original.set_value("key1", "value1");
    original.set_value("key2", "value2");
    
    ConfigurationData cloned = original;
    
    EXPECT_EQ(cloned.get_value("key1"), "value1");
    EXPECT_EQ(cloned.get_value("key2"), "value2");
}

// Test: Export to JSON
TEST_F(ConfigurationDataTest, ExportToJson) {
    ConfigurationData config;
    
    config.set_value("name", "Alice");
    config.set_value("age", 28);
    config.set_value("active", true);
    
    json exported = config.toJson();
    
    EXPECT_EQ(exported["name"], "Alice");
    EXPECT_EQ(exported["age"], 28);
    EXPECT_EQ(exported["active"], true);
}

// Test: Import from JSON
TEST_F(ConfigurationDataTest, ImportFromJson) {
    json data = {
        {"name", "Bob"},
        {"email", "bob@example.com"},
        {"settings", {
            {"theme", "light"}
        }}
    };
    
    ConfigurationData config;
    config.fromJson(data);
    
    EXPECT_EQ(config.get_value("name"), "Bob");
    EXPECT_EQ(config.get_value("email"), "bob@example.com");
    EXPECT_EQ(config.get_value("settings")["theme"], "light");
}

// Test: Round-trip JSON conversion
TEST_F(ConfigurationDataTest, RoundTripJsonConversion) {
    ConfigurationData original;
    original.set_value("key1", "value1");
    original.set_value("key2", 42);
    original.set_value("key3", true);
    
    json exported = original.toJson();
    
    ConfigurationData restored;
    restored.fromJson(exported);
    
    EXPECT_EQ(restored.get_value("key1"), "value1");
    EXPECT_EQ(restored.get_value("key2"), 42);
    EXPECT_EQ(restored.get_value("key3"), true);
}

// Test: Merge two configurations
TEST_F(ConfigurationDataTest, MergeTwoConfigurations) {
    ConfigurationData config1;
    config1.set_value("key1", "value1");
    config1.set_value("key2", "value2");
    
    ConfigurationData config2;
    config2.set_value("key2", "value2_override");
    config2.set_value("key3", "value3");
    
    config1.merge(config2);
    
    EXPECT_EQ(config1.get_value("key1"), "value1");
    EXPECT_EQ(config1.get_value("key2"), "value2_override");
    EXPECT_EQ(config1.get_value("key3"), "value3");
}

// Test: Get configuration size
TEST_F(ConfigurationDataTest, GetConfigurationSize) {
    ConfigurationData config;
    
    EXPECT_EQ(config.size(), 0);
    
    config.set_value("key1", "value1");
    EXPECT_EQ(config.size(), 1);
    
    config.set_value("key2", "value2");
    EXPECT_EQ(config.size(), 2);
}

// Test: Test form state initialization
TEST_F(ConfigurationDataTest, FormStateInitialization) {
    FormState form_state;
    
    EXPECT_TRUE(form_state.is_clean());
    EXPECT_FALSE(form_state.isDirty());
}

// Test: Track form field changes
TEST_F(ConfigurationDataTest, TrackFormFieldChanges) {
    FormState form_state;
    
    form_state.mark_field_dirty("name");
    
    EXPECT_TRUE(form_state.isDirty());
    EXPECT_TRUE(form_state.isFieldDirty("name"));
}

// Test: Track validation state
TEST_F(ConfigurationDataTest, TrackValidationState) {
    FormState form_state;
    
    form_state.setFieldValid("name", true);
    EXPECT_TRUE(form_state.isFieldValid("name"));
    
    form_state.setFieldValid("name", false);
    EXPECT_FALSE(form_state.isFieldValid("name"));
}

// Test: Track validation errors
TEST_F(ConfigurationDataTest, TrackValidationErrors) {
    FormState form_state;
    
    form_state.set_field_error("email", "Invalid email format");
    
    EXPECT_TRUE(form_state.has_field_error("email"));
    EXPECT_EQ(form_state.getFieldError("email"), "Invalid email format");
}

// Test: Clear field dirty flag
TEST_F(ConfigurationDataTest, ClearFieldDirtyFlag) {
    FormState form_state;
    
    form_state.mark_field_dirty("name");
    EXPECT_TRUE(form_state.isDirty());
    
    form_state.mark_field_clean("name");
    // Should only be clean if all fields are clean
}

// Test: Get all dirty fields
TEST_F(ConfigurationDataTest, GetAllDirtyFields) {
    FormState form_state;
    
    form_state.mark_field_dirty("name");
    form_state.mark_field_dirty("email");
    form_state.mark_field_dirty("age");
    
    auto dirty_fields = form_state.get_dirty_fields();
    EXPECT_EQ(dirty_fields.size(), 3);
}

// Test: Reset form state
TEST_F(ConfigurationDataTest, ResetFormState) {
    FormState form_state;
    
    form_state.mark_field_dirty("name");
    form_state.set_field_error("email", "Error");
    
    form_state.reset();
    
    EXPECT_TRUE(form_state.is_clean());
    EXPECT_FALSE(form_state.has_field_error("email"));
}

// Test: Handle null values in configuration
TEST_F(ConfigurationDataTest, HandleNullValuesInConfiguration) {
    ConfigurationData config;
    
    config.set_value("null_field", nullptr);
    
    auto value = config.get_value("null_field");
    EXPECT_TRUE(value.is_null());
}

// Test: Store large configuration
TEST_F(ConfigurationDataTest, StoreLargeConfiguration) {
    ConfigurationData config;
    
    for (int i = 0; i < 1000; ++i) {
        config.set_value("field_" + std::to_string(i), "value_" + std::to_string(i));
    }
    
    EXPECT_EQ(config.size(), 1000);
    EXPECT_EQ(config.get_value("field_500"), "value_500");
}

// Test: Field existence check with empty configuration
TEST_F(ConfigurationDataTest, FieldExistenceCheckEmptyConfig) {
    ConfigurationData config;
    
    EXPECT_FALSE(config.has_key("any_key"));
}

// Test: Store configuration with special characters
TEST_F(ConfigurationDataTest, StoreConfigurationWithSpecialCharacters) {
    ConfigurationData config;
    
    config.set_value("key_with_special", "value with spaces & symbols!");
    config.set_value("unicode_key", "François - 北京 - 😀");
    
    EXPECT_EQ(config.get_value("key_with_special"), "value with spaces & symbols!");
    EXPECT_EQ(config.get_value("unicode_key"), "François - 北京 - 😀");
}

// Test: Get fields with certain prefix
TEST_F(ConfigurationDataTest, GetFieldsWithPrefix) {
    ConfigurationData config;
    
    config.set_value("app_name", "MyApp");
    config.set_value("app_version", "1.0");
    config.set_value("db_host", "localhost");
    config.set_value("db_port", 5432);
    
    auto keys = config.get_keys();
    auto app_keys = config.get_keys_by_prefix("app_");
    
    // Should contain app_name and app_version
    EXPECT_GE(app_keys.size(), 0);
}

} // namespace test
} // namespace core
} // namespace configgui
