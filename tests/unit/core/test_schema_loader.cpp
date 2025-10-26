// SPDX-License-Identifier: MIT
// Unit tests for Schema Loader - Core module

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "core/schema/schema_loader.h"
#include "core/schema/schema.h"
#include "core/error_types.h"

using json = nlohmann::ordered_json;
namespace fs = std::filesystem;

namespace configgui {
namespace core {
namespace test {

// Test fixture for schema loader tests
class SchemaLoaderTest : public ::testing::Test {
protected:
    SchemaLoaderTest() : loader_() {
        // Create temporary directory for test files
        test_dir_ = fs::temp_directory_path() / "configgui_test";
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
        fs::create_directories(test_dir_);
    }

    ~SchemaLoaderTest() override {
        // Clean up temporary directory
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }

    void WriteJsonFile(const std::string& filename, const json& data) {
        std::ofstream file(test_dir_ / filename);
        file << data.dump(2);
        file.close();
    }

    SchemaLoader loader_;
    fs::path test_dir_;
};

// Test: Load valid schema from JSON string
TEST_F(SchemaLoaderTest, LoadSchemaFromValidJsonString) {
    json schema_json = {
        {"type", "object"},
        {"title", "Test Configuration"},
        {"description", "A test schema"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"age", {{"type", "integer"}}}
        }},
        {"required", {"name"}}
    };

    auto result = loader_.loadSchemaFromString(schema_json.dump());
    ASSERT_TRUE(result);
    auto schema = result.value();
    
    EXPECT_EQ(schema.title(), "Test Configuration");
    EXPECT_EQ(schema.description(), "A test schema");
}

// Test: Load schema with nested objects
TEST_F(SchemaLoaderTest, LoadSchemaWithNestedObjects) {
    json schema_json = {
        {"type", "object"},
        {"title", "Complex Schema"},
        {"properties", {
            {"user", {
                {"type", "object"},
                {"properties", {
                    {"name", {{"type", "string"}}},
                    {"email", {{"type", "string"}, {"format", "email"}}}
                }}
            }},
            {"settings", {
                {"type", "object"},
                {"properties", {
                    {"theme", {{"type", "string", "enum", {"light", "dark"}}}},
                    {"notifications", {{"type", "boolean"}}}
                }}
            }}
        }}
    };

    auto result = loader_.loadSchemaFromString(schema_json.dump());
    ASSERT_TRUE(result);
    auto schema = result.value();
    
    auto properties = schema.properties();
    EXPECT_TRUE(properties.contains("user"));
    EXPECT_TRUE(properties.contains("settings"));
}

// Test: Load schema with array types
TEST_F(SchemaLoaderTest, LoadSchemaWithArrays) {
    json schema_json = {
        {"type", "object"},
        {"title", "Schema with Arrays"},
        {"properties", {
            {"tags", {
                {"type", "array"},
                {"items", {{"type", "string"}}}
            }},
            {"numbers", {
                {"type", "array"},
                {"items", {{"type", "integer"}}}
            }}
        }}
    };

    auto result = loader_.loadSchemaFromString(schema_json.dump());
    ASSERT_TRUE(result);
    auto schema = result.value();
    
    auto properties = schema.properties();
    EXPECT_TRUE(properties.contains("tags"));
    EXPECT_TRUE(properties.contains("numbers"));
}

// Test: Load schema with validation rules
TEST_F(SchemaLoaderTest, LoadSchemaWithValidationRules) {
    json schema_json = {
        {"type", "object"},
        {"properties", {
            {"username", {
                {"type", "string"},
                {"minLength", 3},
                {"maxLength", 20},
                {"pattern", "^[a-zA-Z0-9_]+$"}
            }},
            {"age", {
                {"type", "integer"},
                {"minimum", 0},
                {"maximum", 150}
            }},
            {"email", {
                {"type", "string"},
                {"format", "email"}
            }}
        }}
    };

    auto result = loader_.loadSchemaFromString(schema_json.dump());
    ASSERT_TRUE(result);
    
    // Verify schema loaded successfully
    EXPECT_TRUE(result.value().is_valid());
}

// Test: Load schema from file
TEST_F(SchemaLoaderTest, LoadSchemaFromFile) {
    json test_schema = {
        {"type", "object"},
        {"title", "File Test Schema"},
        {"properties", {
            {"field1", {{"type", "string"}}}
        }}
    };
    
    WriteJsonFile("test_schema.json", test_schema);
    std::string file_path = (test_dir_ / "test_schema.json").string();

    auto result = loader_.loadSchema(file_path);
    ASSERT_TRUE(result);
    
    auto schema = result.value();
    EXPECT_EQ(schema.title(), "File Test Schema");
}

// Test: Handle invalid JSON string
TEST_F(SchemaLoaderTest, InvalidJsonStringReturnsError) {
    std::string invalid_json = "{invalid json";
    auto result = loader_.loadSchemaFromString(invalid_json);
    EXPECT_FALSE(result);
}

// Test: Handle missing file
TEST_F(SchemaLoaderTest, MissingFileReturnsError) {
    std::string file_path = (test_dir_ / "nonexistent.json").string();
    auto result = loader_.loadSchema(file_path);
    EXPECT_FALSE(result);
}

// Test: Handle empty schema
TEST_F(SchemaLoaderTest, EmptySchemaIsValid) {
    json empty_schema = json::object();
    auto result = loader_.loadSchemaFromString(empty_schema.dump());
    ASSERT_TRUE(result);
}

// Test: Load schema with default values
TEST_F(SchemaLoaderTest, LoadSchemaWithDefaults) {
    json schema_json = {
        {"type", "object"},
        {"properties", {
            {"name", {
                {"type", "string"},
                {"default", "Anonymous"}
            }},
            {"active", {
                {"type", "boolean"},
                {"default", true}
            }}
        }}
    };

    auto result = loader_.loadSchemaFromString(schema_json.dump());
    ASSERT_TRUE(result);
    auto schema = result.value();
    
    auto properties = schema.properties();
    EXPECT_TRUE(properties["name"].contains("default"));
    EXPECT_TRUE(properties["active"].contains("default"));
}

// Test: Load large schema (1000+ properties)
TEST_F(SchemaLoaderTest, LoadLargeSchemaWithManyProperties) {
    json schema_json = {
        {"type", "object"},
        {"title", "Large Schema"},
        {"properties", json::object()}
    };

    // Add 1000 properties
    for (int i = 0; i < 1000; ++i) {
        schema_json["properties"][std::string("field_") + std::to_string(i)] = {
            {"type", "string"}
        };
    }

    auto result = loader_.loadSchemaFromString(schema_json.dump());
    ASSERT_TRUE(result);
    auto schema = result.value();
    
    auto properties = schema.properties();
    EXPECT_GE(properties.size(), 1000);
}

// Test: Required fields are parsed correctly
TEST_F(SchemaLoaderTest, ParseRequiredFields) {
    json schema_json = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"email", {{"type", "string"}}},
            {"age", {{"type", "integer"}}}
        }},
        {"required", {"name", "email"}}
    };

    auto result = loader_.loadSchemaFromString(schema_json.dump());
    ASSERT_TRUE(result);
    auto schema = result.value();
    
    auto required = schema.required_fields();
    EXPECT_EQ(required.size(), 2);
    EXPECT_TRUE(std::find(required.begin(), required.end(), "name") != required.end());
    EXPECT_TRUE(std::find(required.begin(), required.end(), "email") != required.end());
}

// Test: Enum values are preserved
TEST_F(SchemaLoaderTest, PreserveEnumValues) {
    json schema_json = {
        {"type", "object"},
        {"properties", {
            {"status", {
                {"type", "string"},
                {"enum", {"active", "inactive", "pending"}}
            }}
        }}
    };

    auto result = loader_.loadSchemaFromString(schema_json.dump());
    ASSERT_TRUE(result);
    auto schema = result.value();
    
    auto properties = schema.properties();
    EXPECT_TRUE(properties["status"].contains("enum"));
    EXPECT_EQ(properties["status"]["enum"].size(), 3);
}

// Test: Load UTF-8 content
TEST_F(SchemaLoaderTest, LoadUtf8Content) {
    json schema_json = {
        {"type", "object"},
        {"title", "UTF-8 Schema 你好"},
        {"description", "Schema with 🎉 emoji"},
        {"properties", {
            {"name", {{"type", "string"}}}
        }}
    };

    auto result = loader_.loadSchemaFromString(schema_json.dump());
    ASSERT_TRUE(result);
    auto schema = result.value();
    
    EXPECT_TRUE(schema.title().find("你好") != std::string::npos);
}

// Test: Schema can be loaded multiple times
TEST_F(SchemaLoaderTest, LoadMultipleSchemasSequentially) {
    json schema1 = {
        {"type", "object"},
        {"title", "Schema 1"},
        {"properties", {{"field1", {{"type", "string"}}}}}
    };
    
    json schema2 = {
        {"type", "object"},
        {"title", "Schema 2"},
        {"properties", {{"field2", {{"type", "integer"}}}}}
    };

    auto result1 = loader_.loadSchemaFromString(schema1.dump());
    auto result2 = loader_.loadSchemaFromString(schema2.dump());
    
    ASSERT_TRUE(result1);
    ASSERT_TRUE(result2);
    EXPECT_EQ(result1.value().title(), "Schema 1");
    EXPECT_EQ(result2.value().title(), "Schema 2");
}

} // namespace test
} // namespace core
} // namespace configgui
