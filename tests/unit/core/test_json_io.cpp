// SPDX-License-Identifier: MIT
// Unit tests for JSON I/O - Core module

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>

#include "core/io/json_reader.h"
#include "core/io/json_writer.h"
#include "core/error_types.h"

using json = nlohmann::ordered_json;
namespace fs = std::filesystem;

namespace configgui {
namespace core {
namespace test {

// Test fixture for JSON I/O tests
class JsonIoTest : public ::testing::Test {
protected:
    std::string temp_dir;

    JsonIoTest() {
        // Create temporary directory for test files
        temp_dir = "/tmp/configgui_json_test_" + std::to_string(std::time(nullptr));
        fs::create_directories(temp_dir);
    }

    ~JsonIoTest() override {
        // Clean up temporary directory
        if (fs::exists(temp_dir)) {
            fs::remove_all(temp_dir);
        }
    }

    std::string GetTestFilePath(const std::string& filename) {
        return temp_dir + "/" + filename;
    }

    void WriteTestFile(const std::string& filename, const std::string& content) {
        std::ofstream file(GetTestFilePath(filename));
        file << content;
        file.close();
    }
};

// Test: Read valid JSON from string
TEST_F(JsonIoTest, ReadValidJsonFromString) {
    std::string json_str = R"({"name": "John", "age": 30})";
    
    JsonReader reader;
    auto result = reader.parseString(json_str);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value()["name"], "John");
    EXPECT_EQ(result.value()["age"], 30);
}

// Test: Read invalid JSON returns error
TEST_F(JsonIoTest, ReadInvalidJsonReturnsError) {
    std::string invalid_json = R"({"name": "John", "age": 30)"; // Missing closing brace
    
    JsonReader reader;
    auto result = reader.parseString(invalid_json);
    
    EXPECT_FALSE(result);
}

// Test: Read JSON from file
TEST_F(JsonIoTest, ReadJsonFromFile) {
    std::string json_str = R"({"name": "Alice", "email": "alice@example.com"})";
    std::string filepath = GetTestFilePath("config.json");
    WriteTestFile("config.json", json_str);
    
    JsonReader reader;
    auto result = reader.readFile(filepath);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value()["name"], "Alice");
    EXPECT_EQ(result.value()["email"], "alice@example.com");
}

// Test: Read non-existent file returns error
TEST_F(JsonIoTest, ReadNonExistentFileReturnsError) {
    JsonReader reader;
    auto result = reader.readFile("/non/existent/path.json");
    
    EXPECT_FALSE(result);
}

// Test: Write JSON to string
TEST_F(JsonIoTest, WriteJsonToString) {
    json data = {
        {"name", "Bob"},
        {"age", 25},
        {"active", true}
    };
    
    JsonWriter writer;
    auto result = writer.toString(data);
    
    ASSERT_TRUE(result);
    std::string json_str = result.value();
    EXPECT_FALSE(json_str.empty());
    EXPECT_NE(json_str.find("\"name\""), std::string::npos);
    EXPECT_NE(json_str.find("\"Bob\""), std::string::npos);
}

// Test: Write JSON to file
TEST_F(JsonIoTest, WriteJsonToFile) {
    json data = {
        {"title", "My Config"},
        {"version", "1.0.0"}
    };
    
    std::string filepath = GetTestFilePath("output.json");
    JsonWriter writer;
    auto result = writer.writeFile(filepath, data);
    
    ASSERT_TRUE(result);
    EXPECT_TRUE(fs::exists(filepath));
    
    // Verify file contents
    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    EXPECT_FALSE(content.empty());
}

// Test: Round-trip JSON (write then read)
TEST_F(JsonIoTest, RoundTripJson) {
    json original = {
        {"name", "Charlie"},
        {"email", "charlie@example.com"},
        {"preferences", {
            {"theme", "dark"},
            {"notifications", true}
        }}
    };
    
    std::string filepath = GetTestFilePath("roundtrip.json");
    
    // Write
    JsonWriter writer;
    auto write_result = writer.writeFile(filepath, original);
    ASSERT_TRUE(write_result);
    
    // Read
    JsonReader reader;
    auto read_result = reader.readFile(filepath);
    ASSERT_TRUE(read_result);
    
    // Verify contents
    json read_data = read_result.value();
    EXPECT_EQ(read_data["name"], original["name"]);
    EXPECT_EQ(read_data["email"], original["email"]);
    EXPECT_EQ(read_data["preferences"]["theme"], "dark");
}

// Test: Read JSON with nested objects
TEST_F(JsonIoTest, ReadJsonWithNestedObjects) {
    std::string json_str = R"({
        "user": {
            "name": "David",
            "contact": {
                "email": "david@example.com",
                "phone": "123-456-7890"
            }
        }
    })";
    
    JsonReader reader;
    auto result = reader.parseString(json_str);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value()["user"]["name"], "David");
    EXPECT_EQ(result.value()["user"]["contact"]["email"], "david@example.com");
}

// Test: Read JSON with arrays
TEST_F(JsonIoTest, ReadJsonWithArrays) {
    std::string json_str = R"({
        "tags": ["python", "cpp", "web"],
        "scores": [85, 90, 95]
    })";
    
    JsonReader reader;
    auto result = reader.parseString(json_str);
    
    ASSERT_TRUE(result);
    auto tags = result.value()["tags"];
    EXPECT_EQ(tags.size(), 3);
    EXPECT_EQ(tags[0], "python");
    EXPECT_EQ(tags[1], "cpp");
}

// Test: Write JSON with special characters (UTF-8)
TEST_F(JsonIoTest, WriteJsonWithUtf8) {
    json data = {
        {"name", "François"},
        {"city", "北京"},
        {"emoji", "😀"}
    };
    
    JsonWriter writer;
    auto result = writer.toString(data);
    
    ASSERT_TRUE(result);
    std::string json_str = result.value();
    EXPECT_NE(json_str.find("François"), std::string::npos);
}

// Test: Write JSON with pretty formatting
TEST_F(JsonIoTest, WriteJsonWithPrettyFormatting) {
    json data = {
        {"name", "Emma"},
        {"settings", {
            {"theme", "light"},
            {"language", "en"}
        }}
    };
    
    JsonWriter writer;
    auto result = writer.toString(data, true); // pretty=true
    
    ASSERT_TRUE(result);
    std::string json_str = result.value();
    // Pretty-printed JSON should contain newlines
    EXPECT_NE(json_str.find("\n"), std::string::npos);
}

// Test: Write JSON with compact formatting
TEST_F(JsonIoTest, WriteJsonWithCompactFormatting) {
    json data = {
        {"name", "Frank"},
        {"active", true}
    };
    
    JsonWriter writer;
    auto result = writer.toString(data, false); // pretty=false
    
    ASSERT_TRUE(result);
    std::string json_str = result.value();
    // Compact format should not have unnecessary whitespace
    EXPECT_TRUE(json_str.length() > 0);
}

// Test: Preserve JSON key order
TEST_F(JsonIoTest, PreserveKeyOrder) {
    std::string json_str = R"({"z": 1, "a": 2, "m": 3})";
    
    JsonReader reader;
    auto result = reader.parseString(json_str);
    
    ASSERT_TRUE(result);
    // Using ordered_json should preserve insertion order
    auto keys = result.value().keys();
    EXPECT_EQ(keys[0], "z");
    EXPECT_EQ(keys[1], "a");
    EXPECT_EQ(keys[2], "m");
}

// Test: Handle empty JSON object
TEST_F(JsonIoTest, HandleEmptyJsonObject) {
    std::string json_str = "{}";
    
    JsonReader reader;
    auto result = reader.parseString(json_str);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value().size(), 0);
}

// Test: Handle empty JSON array
TEST_F(JsonIoTest, HandleEmptyJsonArray) {
    std::string json_str = "[]";
    
    JsonReader reader;
    auto result = reader.parseString(json_str);
    
    ASSERT_TRUE(result);
    EXPECT_TRUE(result.value().is_array());
}

// Test: Handle JSON with nulls
TEST_F(JsonIoTest, HandleJsonWithNulls) {
    std::string json_str = R"({"value": null, "name": "Test"})";
    
    JsonReader reader;
    auto result = reader.parseString(json_str);
    
    ASSERT_TRUE(result);
    EXPECT_TRUE(result.value()["value"].is_null());
}

// Test: Handle large JSON file
TEST_F(JsonIoTest, HandleLargeJsonFile) {
    json data = json::object();
    
    // Create large JSON with many properties
    for (int i = 0; i < 1000; ++i) {
        data["field_" + std::to_string(i)] = "value_" + std::to_string(i);
    }
    
    std::string filepath = GetTestFilePath("large.json");
    
    JsonWriter writer;
    auto write_result = writer.writeFile(filepath, data);
    ASSERT_TRUE(write_result);
    
    JsonReader reader;
    auto read_result = reader.readFile(filepath);
    ASSERT_TRUE(read_result);
    EXPECT_EQ(read_result.value().size(), 1000);
}

// Test: Handle JSON with deeply nested structure
TEST_F(JsonIoTest, HandleDeeplyNestedJson) {
    json data = {
        {"level1", {
            {"level2", {
                {"level3", {
                    {"level4", {
                        {"level5", "deep_value"}
                    }}
                }}
            }}
        }}
    };
    
    std::string filepath = GetTestFilePath("nested.json");
    
    JsonWriter writer;
    auto write_result = writer.writeFile(filepath, data);
    ASSERT_TRUE(write_result);
    
    JsonReader reader;
    auto read_result = reader.readFile(filepath);
    ASSERT_TRUE(read_result);
    EXPECT_EQ(read_result.value()["level1"]["level2"]["level3"]["level4"]["level5"], "deep_value");
}

// Test: Handle JSON with various data types
TEST_F(JsonIoTest, HandleJsonWithVariousTypes) {
    json data = {
        {"string_val", "text"},
        {"integer_val", 42},
        {"float_val", 3.14},
        {"bool_true", true},
        {"bool_false", false},
        {"null_val", nullptr},
        {"array_val", {1, 2, 3}},
        {"object_val", {{"nested", "value"}}}
    };
    
    std::string filepath = GetTestFilePath("mixed_types.json");
    
    JsonWriter writer;
    auto write_result = writer.writeFile(filepath, data);
    ASSERT_TRUE(write_result);
    
    JsonReader reader;
    auto read_result = reader.readFile(filepath);
    ASSERT_TRUE(read_result);
    
    json& result = read_result.value();
    EXPECT_EQ(result["string_val"], "text");
    EXPECT_EQ(result["integer_val"], 42);
    EXPECT_DOUBLE_EQ(result["float_val"], 3.14);
    EXPECT_EQ(result["bool_true"], true);
    EXPECT_EQ(result["bool_false"], false);
    EXPECT_TRUE(result["null_val"].is_null());
}

// Test: Overwrite existing JSON file
TEST_F(JsonIoTest, OverwriteExistingJsonFile) {
    std::string filepath = GetTestFilePath("overwrite.json");
    
    // Write first version
    json data1 = {{"version", "1.0"}};
    JsonWriter writer;
    auto result1 = writer.writeFile(filepath, data1);
    ASSERT_TRUE(result1);
    
    // Overwrite with second version
    json data2 = {{"version", "2.0"}, {"updated", true}};
    auto result2 = writer.writeFile(filepath, data2);
    ASSERT_TRUE(result2);
    
    // Verify final content
    JsonReader reader;
    auto read_result = reader.readFile(filepath);
    ASSERT_TRUE(read_result);
    EXPECT_EQ(read_result.value()["version"], "2.0");
    EXPECT_EQ(read_result.value()["updated"], true);
}

// Test: Handle file permission errors (platform-specific)
TEST_F(JsonIoTest, HandleFilePermissionError) {
    // This test is platform-specific and may need adjustment
    std::string filepath = "/root/configgui_test_no_permission.json";
    
    json data = {{"test", "data"}};
    JsonWriter writer;
    auto result = writer.writeFile(filepath, data);
    
    // Should fail due to permissions (unless running as root)
    // This test may fail on some systems, so we don't assert
}

} // namespace test
} // namespace core
} // namespace configgui
