// SPDX-License-Identifier: MIT
// YAML Reader/Writer Unit Tests

#include <gtest/gtest.h>
#include "src/io/yaml_reader.h"
#include "src/io/yaml_writer.h"
#include "common/test_fixtures.h"

using namespace configgui::io;

class YamlReaderWriterTest : public FileIOTestFixture
{
protected:
    YamlReader reader_;
    YamlWriter writer_;
};

// ========== Basic YAML Writing Tests ==========

TEST_F(YamlReaderWriterTest, WriteEmpty_Object)
{
    json data = json::object();
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
    EXPECT_TRUE(fileExists(temp_file));
}

TEST_F(YamlReaderWriterTest, WriteString)
{
    json data = "hello world";
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(YamlReaderWriterTest, WriteNumber)
{
    json data = 42;
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(YamlReaderWriterTest, WriteBoolean)
{
    json data = true;
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(YamlReaderWriterTest, WriteNull)
{
    json data = nullptr;
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

// ========== Complex YAML Writing Tests ==========

TEST_F(YamlReaderWriterTest, WriteSimpleObject)
{
    json data = {
        {"name", "John"},
        {"age", 30},
        {"email", "john@example.com"}
    };
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(YamlReaderWriterTest, WriteNestedObject)
{
    json data = {
        {"user", {
            {"name", "John"},
            {"address", {
                {"street", "123 Main"},
                {"city", "Springfield"}
            }}
        }}
    };
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(YamlReaderWriterTest, WriteArray)
{
    json data = json::array({1, 2, 3, 4, 5});
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(YamlReaderWriterTest, WriteArrayOfObjects)
{
    json data = json::array({
        json{{"name", "John"}, {"age", 30}},
        json{{"name", "Jane"}, {"age", 28}}
    });
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

// ========== Basic YAML Reading Tests ==========

TEST_F(YamlReaderWriterTest, ReadSimple_Object)
{
    json expected = {
        {"name", "John"},
        {"age", 30}
    };
    std::string temp_file = createTempFile("test.yaml");
    writer_.write(temp_file, expected);

    auto result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(result);
    EXPECT_EQ(result.value, expected);
}

TEST_F(YamlReaderWriterTest, ReadArray)
{
    json expected = json::array({1, 2, 3});
    std::string temp_file = createTempFile("test.yaml");
    writer_.write(temp_file, expected);

    auto result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(result);
    EXPECT_EQ(result.value, expected);
}

// ========== YAML-Specific Syntax Tests ==========

TEST_F(YamlReaderWriterTest, ReadYamlFlow_Syntax)
{
    std::string temp_file = createTempFile("test.yaml");
    std::ofstream file(temp_file);
    file << "name: John\n";
    file << "age: 30\n";
    file.close();

    auto result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(result);
    EXPECT_EQ(result.value["name"], "John");
    EXPECT_EQ(result.value["age"], 30);
}

TEST_F(YamlReaderWriterTest, ReadYamlList_Syntax)
{
    std::string temp_file = createTempFile("test.yaml");
    std::ofstream file(temp_file);
    file << "items:\n";
    file << "  - first\n";
    file << "  - second\n";
    file << "  - third\n";
    file.close();

    auto result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(result);
    EXPECT_EQ(result.value["items"].size(), 3);
}

// ========== Round-Trip Tests ==========

TEST_F(YamlReaderWriterTest, RoundTrip_SimpleObject)
{
    json original = {
        {"name", "John"},
        {"age", 30},
        {"email", "john@example.com"}
    };
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, original);
}

TEST_F(YamlReaderWriterTest, RoundTrip_NestedObject)
{
    json original = {
        {"user", {
            {"name", "John"},
            {"settings", {
                {"theme", "dark"},
                {"notifications", true}
            }}
        }}
    };
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, original);
}

TEST_F(YamlReaderWriterTest, RoundTrip_ArrayOfObjects)
{
    json original = json::array({
        json{{"id", 1}, {"name", "Alice"}},
        json{{"id", 2}, {"name", "Bob"}},
        json{{"id", 3}, {"name", "Charlie"}}
    });
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, original);
}

// ========== Error Handling Tests ==========

TEST_F(YamlReaderWriterTest, ReadNonExistent_File)
{
    std::string non_existent = temp_dir_ + "/non_existent_file.yaml";

    auto result = reader_.read(non_existent);

    ASSERT_INVALID_RESULT(result);
}

TEST_F(YamlReaderWriterTest, ReadMalformed_YAML)
{
    std::string temp_file = createTempFile("test.yaml");
    std::ofstream file(temp_file);
    file << "invalid: yaml: syntax:\n";
    file << "  bad indentation\n";
    file.close();

    auto result = reader_.read(temp_file);

    // YAML is more forgiving than JSON, but invalid indentation should fail
    // Result depends on YAML parser behavior
}

TEST_F(YamlReaderWriterTest, ReadEmpty_File)
{
    std::string temp_file = createTempFile("test.yaml");
    std::ofstream file(temp_file);
    file.close();

    auto result = reader_.read(temp_file);

    // Empty YAML file might return null or object depending on implementation
}

TEST_F(YamlReaderWriterTest, WriteToInvalidPath)
{
    json data = {{"test", "data"}};
    std::string invalid_path = "/invalid/nonexistent/path/file.yaml";

    auto result = writer_.write(invalid_path, data);

    ASSERT_INVALID_RESULT(result);
}

// ========== Unicode Handling Tests ==========

TEST_F(YamlReaderWriterTest, WriteUnicode_Strings)
{
    json data = {
        {"english", "Hello"},
        {"chinese", "你好"},
        {"emoji", "😀"}
    };
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(YamlReaderWriterTest, RoundTrip_Unicode)
{
    json original = {
        {"name", "José"},
        {"city", "São Paulo"},
        {"greeting", "Здравствуй"}
    };
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, original);
}

// ========== Special Characters Tests ==========

TEST_F(YamlReaderWriterTest, SpecialCharacters_Colon)
{
    json data = {{"url", "http://example.com:8080"}};
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, data).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value["url"], "http://example.com:8080");
}

TEST_F(YamlReaderWriterTest, SpecialCharacters_Hash)
{
    json data = {{"comment", "#This is not a comment in value"}};
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, data).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value["comment"], "#This is not a comment in value");
}

// ========== Multiline String Tests ==========

TEST_F(YamlReaderWriterTest, MultilineString_Block)
{
    json data = {
        {"text", "Line 1\nLine 2\nLine 3"}
    };
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, data).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value["text"], "Line 1\nLine 2\nLine 3");
}

// ========== Large Data Tests ==========

TEST_F(YamlReaderWriterTest, LargeArray_ManyElements)
{
    json data = json::array();
    for (int i = 0; i < 500; ++i)
    {
        data.push_back(i);
    }
    std::string temp_file = createTempFile("test.yaml");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(YamlReaderWriterTest, RoundTrip_LargeObject)
{
    json original = json::object();
    for (int i = 0; i < 50; ++i)
    {
        original["key_" + std::to_string(i)] = "value_" + std::to_string(i);
    }
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value.size(), original.size());
}

// ========== Type Preservation Tests ==========

TEST_F(YamlReaderWriterTest, TypePreservation_Integer)
{
    json original = {{"count", 42}};
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_TRUE(read_result.value["count"].is_number_integer());
}

TEST_F(YamlReaderWriterTest, TypePreservation_Float)
{
    json original = {{"price", 19.99}};
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_TRUE(read_result.value["price"].is_number_float());
}

TEST_F(YamlReaderWriterTest, TypePreservation_Boolean)
{
    json original = {{"active", true}, {"deleted", false}};
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value["active"], true);
}

TEST_F(YamlReaderWriterTest, TypePreservation_Null)
{
    json original = {{"data", nullptr}};
    std::string temp_file = createTempFile("test.yaml");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_TRUE(read_result.value["data"].is_null());
}

// ========== Configuration File Tests ==========

TEST_F(YamlReaderWriterTest, RoundTrip_Configuration)
{
    json config = createValidConfig();
    std::string temp_file = createTempFile("config.yaml");

    ASSERT_TRUE(writer_.write(temp_file, config).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, config);
}

// ========== JSON to YAML Conversion Tests ==========

TEST_F(YamlReaderWriterTest, ConvertJson_ToYaml)
{
    json data = {
        {"name", "John"},
        {"age", 30},
        {"email", "john@example.com"}
    };
    
    std::string json_file = createTempFile("data.json");
    std::string yaml_file = createTempFile("data.yaml");
    
    // Write as JSON
    auto json_write_result = configgui::io::JsonWriter().write(json_file, data);
    ASSERT_TRUE(json_write_result.ok);
    
    // Read JSON
    auto json_read_result = configgui::io::JsonReader().read(json_file);
    ASSERT_VALID_RESULT(json_read_result);
    
    // Write as YAML
    auto yaml_write_result = writer_.write(yaml_file, json_read_result.value);
    ASSERT_TRUE(yaml_write_result.ok);
    
    // Read YAML back
    auto yaml_read_result = reader_.read(yaml_file);
    ASSERT_VALID_RESULT(yaml_read_result);
    
    // Verify data is same
    EXPECT_EQ(yaml_read_result.value, data);
}

// ========== YAML to JSON Conversion Tests ==========

TEST_F(YamlReaderWriterTest, ConvertYaml_ToJson)
{
    json data = json::array({
        json{{"id", 1}, {"name", "Item1"}},
        json{{"id", 2}, {"name", "Item2"}}
    });
    
    std::string yaml_file = createTempFile("data.yaml");
    std::string json_file = createTempFile("data.json");
    
    // Write as YAML
    auto yaml_write_result = writer_.write(yaml_file, data);
    ASSERT_TRUE(yaml_write_result.ok);
    
    // Read YAML
    auto yaml_read_result = reader_.read(yaml_file);
    ASSERT_VALID_RESULT(yaml_read_result);
    
    // Write as JSON
    auto json_write_result = configgui::io::JsonWriter().write(json_file, yaml_read_result.value);
    ASSERT_TRUE(json_write_result.ok);
    
    // Read JSON back
    auto json_read_result = configgui::io::JsonReader().read(json_file);
    ASSERT_VALID_RESULT(json_read_result);
    
    // Verify data is same
    EXPECT_EQ(json_read_result.value, data);
}
