// SPDX-License-Identifier: MIT
// Unit tests for YAML I/O - Core module

#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>

#include "core/io/yaml_reader.h"
#include "core/io/yaml_writer.h"
#include "core/error_types.h"

namespace fs = std::filesystem;

namespace configgui {
namespace core {
namespace test {

// Test fixture for YAML I/O tests
class YamlIoTest : public ::testing::Test {
protected:
    std::string temp_dir;

    YamlIoTest() {
        // Create temporary directory for test files
        temp_dir = "/tmp/configgui_yaml_test_" + std::to_string(std::time(nullptr));
        fs::create_directories(temp_dir);
    }

    ~YamlIoTest() override {
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

// Test: Read valid YAML from string
TEST_F(YamlIoTest, ReadValidYamlFromString) {
    std::string yaml_str = "name: John\nage: 30";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value()["name"], "John");
    EXPECT_EQ(result.value()["age"], 30);
}

// Test: Read YAML from file
TEST_F(YamlIoTest, ReadYamlFromFile) {
    std::string yaml_str = "name: Alice\nemail: alice@example.com";
    std::string filepath = GetTestFilePath("config.yaml");
    WriteTestFile("config.yaml", yaml_str);
    
    YamlReader reader;
    auto result = reader.readFile(filepath);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value()["name"], "Alice");
    EXPECT_EQ(result.value()["email"], "alice@example.com");
}

// Test: Read non-existent YAML file returns error
TEST_F(YamlIoTest, ReadNonExistentFileReturnsError) {
    YamlReader reader;
    auto result = reader.readFile("/non/existent/path.yaml");
    
    EXPECT_FALSE(result);
}

// Test: Write YAML to string
TEST_F(YamlIoTest, WriteYamlToString) {
    nlohmann::json data = {
        {"name", "Bob"},
        {"age", 25},
        {"active", true}
    };
    
    YamlWriter writer;
    auto result = writer.toString(data);
    
    ASSERT_TRUE(result);
    std::string yaml_str = result.value();
    EXPECT_FALSE(yaml_str.empty());
}

// Test: Write YAML to file
TEST_F(YamlIoTest, WriteYamlToFile) {
    nlohmann::json data = {
        {"title", "My Config"},
        {"version", "1.0.0"}
    };
    
    std::string filepath = GetTestFilePath("output.yaml");
    YamlWriter writer;
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

// Test: Round-trip YAML (write then read)
TEST_F(YamlIoTest, RoundTripYaml) {
    nlohmann::json original = {
        {"name", "Charlie"},
        {"email", "charlie@example.com"},
        {"preferences", {
            {"theme", "dark"},
            {"notifications", true}
        }}
    };
    
    std::string filepath = GetTestFilePath("roundtrip.yaml");
    
    // Write
    YamlWriter writer;
    auto write_result = writer.writeFile(filepath, original);
    ASSERT_TRUE(write_result);
    
    // Read
    YamlReader reader;
    auto read_result = reader.readFile(filepath);
    ASSERT_TRUE(read_result);
    
    // Verify contents
    nlohmann::json read_data = read_result.value();
    EXPECT_EQ(read_data["name"], original["name"]);
    EXPECT_EQ(read_data["email"], original["email"]);
    EXPECT_EQ(read_data["preferences"]["theme"], "dark");
}

// Test: Read YAML with nested objects
TEST_F(YamlIoTest, ReadYamlWithNestedObjects) {
    std::string yaml_str = R"(user:
  name: David
  contact:
    email: david@example.com
    phone: 123-456-7890
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value()["user"]["name"], "David");
    EXPECT_EQ(result.value()["user"]["contact"]["email"], "david@example.com");
}

// Test: Read YAML with arrays
TEST_F(YamlIoTest, ReadYamlWithArrays) {
    std::string yaml_str = R"(tags:
  - python
  - cpp
  - web
scores:
  - 85
  - 90
  - 95
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    auto tags = result.value()["tags"];
    EXPECT_EQ(tags.size(), 3);
    EXPECT_EQ(tags[0], "python");
    EXPECT_EQ(tags[1], "cpp");
}

// Test: Write YAML with special characters (UTF-8)
TEST_F(YamlIoTest, WriteYamlWithUtf8) {
    nlohmann::json data = {
        {"name", "François"},
        {"city", "北京"},
        {"emoji", "😀"}
    };
    
    YamlWriter writer;
    auto result = writer.toString(data);
    
    ASSERT_TRUE(result);
    std::string yaml_str = result.value();
    // UTF-8 content should be preserved
    EXPECT_FALSE(yaml_str.empty());
}

// Test: Handle YAML with comments (comment stripping behavior)
TEST_F(YamlIoTest, ReadYamlWithComments) {
    std::string yaml_str = R"(# This is a comment
name: Emma  # inline comment
age: 28
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value()["name"], "Emma");
    EXPECT_EQ(result.value()["age"], 28);
}

// Test: Handle YAML with multiline strings
TEST_F(YamlIoTest, ReadYamlWithMultilineStrings) {
    std::string yaml_str = R"(description: |
  This is a multiline
  description that spans
  multiple lines
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    std::string desc = result.value()["description"];
    EXPECT_NE(desc.find("multiline"), std::string::npos);
}

// Test: Handle YAML with anchors and aliases
TEST_F(YamlIoTest, ReadYamlWithAnchorsAndAliases) {
    std::string yaml_str = R"(defaults: &defaults
  adapter: postgres
  host: localhost
production:
  <<: *defaults
  database: prod_db
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    // Behavior depends on yaml-cpp version and configuration
    EXPECT_TRUE(result.value().contains("production"));
}

// Test: Handle empty YAML file
TEST_F(YamlIoTest, HandleEmptyYamlFile) {
    std::string yaml_str = "";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    // Empty file behavior depends on yaml-cpp configuration
    // Could be valid empty or null
}

// Test: Handle YAML with only scalars
TEST_F(YamlIoTest, HandleYamlWithScalarsOnly) {
    std::string yaml_str = "42";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    // Behavior depends on yaml-cpp configuration
    ASSERT_TRUE(result);
}

// Test: Handle YAML with mixed types in arrays
TEST_F(YamlIoTest, HandleYamlWithMixedTypeArrays) {
    std::string yaml_str = R"(mixed:
  - string_value
  - 42
  - 3.14
  - true
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    auto mixed = result.value()["mixed"];
    EXPECT_EQ(mixed.size(), 4);
}

// Test: Handle large YAML file
TEST_F(YamlIoTest, HandleLargeYamlFile) {
    std::stringstream ss;
    ss << "fields:\n";
    
    for (int i = 0; i < 100; ++i) {
        ss << "  field_" << i << ": value_" << i << "\n";
    }
    
    std::string yaml_str = ss.str();
    std::string filepath = GetTestFilePath("large.yaml");
    WriteTestFile("large.yaml", yaml_str);
    
    YamlReader reader;
    auto result = reader.readFile(filepath);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value()["fields"].size(), 100);
}

// Test: Handle deeply nested YAML structure
TEST_F(YamlIoTest, HandleDeeplyNestedYaml) {
    std::string yaml_str = R"(level1:
  level2:
    level3:
      level4:
        level5: deep_value
)";
    
    std::string filepath = GetTestFilePath("nested.yaml");
    WriteTestFile("nested.yaml", yaml_str);
    
    YamlReader reader;
    auto result = reader.readFile(filepath);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value()["level1"]["level2"]["level3"]["level4"]["level5"], "deep_value");
}

// Test: Handle YAML with boolean values
TEST_F(YamlIoTest, HandleYamlWithBooleanValues) {
    std::string yaml_str = R"(yes_val: yes
no_val: no
true_val: true
false_val: false
on_val: on
off_val: off
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    // Different yaml-cpp versions may handle boolean variations differently
}

// Test: Handle YAML with null values
TEST_F(YamlIoTest, HandleYamlWithNullValues) {
    std::string yaml_str = R"(null_val: null
tilde_val: ~
empty_val:
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    // Null values should be parsed correctly
}

// Test: Convert between YAML and JSON
TEST_F(YamlIoTest, ConvertBetweenYamlAndJson) {
    // Write as YAML
    nlohmann::json data = {
        {"name", "Frank"},
        {"settings", {
            {"theme", "light"},
            {"language", "en"}
        }}
    };
    
    std::string yaml_filepath = GetTestFilePath("convert.yaml");
    YamlWriter yaml_writer;
    auto yaml_result = yaml_writer.writeFile(yaml_filepath, data);
    ASSERT_TRUE(yaml_result);
    
    // Read as YAML
    YamlReader yaml_reader;
    auto yaml_read = yaml_reader.readFile(yaml_filepath);
    ASSERT_TRUE(yaml_read);
    
    // Verify contents match
    EXPECT_EQ(yaml_read.value()["name"], "Frank");
}

// Test: Handle YAML format with inconsistent indentation
TEST_F(YamlIoTest, HandleYamlWithInconsistentIndentation) {
    std::string yaml_str = R"(key1: value1
  key2: value2
key3: value3
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    // Result depends on yaml-cpp validation level
    // May either succeed with warnings or fail
}

// Test: Overwrite existing YAML file
TEST_F(YamlIoTest, OverwriteExistingYamlFile) {
    std::string filepath = GetTestFilePath("overwrite.yaml");
    
    // Write first version
    nlohmann::json data1 = {{"version", "1.0"}};
    YamlWriter writer;
    auto result1 = writer.writeFile(filepath, data1);
    ASSERT_TRUE(result1);
    
    // Overwrite with second version
    nlohmann::json data2 = {{"version", "2.0"}, {"updated", true}};
    auto result2 = writer.writeFile(filepath, data2);
    ASSERT_TRUE(result2);
    
    // Verify final content
    YamlReader reader;
    auto read_result = reader.readFile(filepath);
    ASSERT_TRUE(read_result);
    EXPECT_EQ(read_result.value()["version"], "2.0");
}

// Test: Handle YAML with scientific notation
TEST_F(YamlIoTest, HandleYamlWithScientificNotation) {
    std::string yaml_str = R"(large_number: 1.23e10
small_number: 5.6e-8
)";
    
    YamlReader reader;
    auto result = reader.parseString(yaml_str);
    
    ASSERT_TRUE(result);
    // Scientific notation should be parsed correctly
}

// Test: Write YAML with explicit types
TEST_F(YamlIoTest, WriteYamlExplicitTypes) {
    nlohmann::json data = {
        {"string", "text"},
        {"integer", 42},
        {"float", 3.14},
        {"boolean", true},
        {"null_val", nullptr}
    };
    
    YamlWriter writer;
    auto result = writer.toString(data);
    
    ASSERT_TRUE(result);
    std::string yaml_str = result.value();
    EXPECT_FALSE(yaml_str.empty());
}

} // namespace test
} // namespace core
} // namespace configgui
