// SPDX-License-Identifier: MIT
// JSON Reader/Writer Unit Tests

#include <gtest/gtest.h>
#include "src/io/json_reader.h"
#include "src/io/json_writer.h"
#include "common/test_fixtures.h"

using namespace configgui::io;

class JsonReaderWriterTest : public FileIOTestFixture
{
protected:
    JsonReader reader_;
    JsonWriter writer_;
};

// ========== Basic JSON Writing Tests ==========

TEST_F(JsonReaderWriterTest, WriteEmpty_Object)
{
    json data = json::object();
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
    EXPECT_TRUE(fileExists(temp_file));
}

TEST_F(JsonReaderWriterTest, WriteEmpty_Array)
{
    json data = json::array();
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, WriteString)
{
    json data = "hello world";
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, WriteNumber)
{
    json data = 42;
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, WriteFloat)
{
    json data = 3.14159;
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, WriteBoolean)
{
    json data = true;
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, WriteNull)
{
    json data = nullptr;
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

// ========== Complex Object Writing Tests ==========

TEST_F(JsonReaderWriterTest, WriteSimpleObject)
{
    json data = {
        {"name", "John"},
        {"age", 30},
        {"email", "john@example.com"}
    };
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, WriteNestedObject)
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
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, WriteArray)
{
    json data = json::array({1, 2, 3, 4, 5});
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, WriteArrayOfObjects)
{
    json data = json::array({
        json{{"name", "John"}, {"age", 30}},
        json{{"name", "Jane"}, {"age", 28}}
    });
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

// ========== Basic JSON Reading Tests ==========

TEST_F(JsonReaderWriterTest, ReadEmpty_Object)
{
    json expected = json::object();
    std::string temp_file = createTempFile("test.json");
    writer_.write(temp_file, expected);

    auto result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(result);
    EXPECT_EQ(result.value, expected);
}

TEST_F(JsonReaderWriterTest, ReadSimple_Object)
{
    json expected = {
        {"name", "John"},
        {"age", 30}
    };
    std::string temp_file = createTempFile("test.json");
    writer_.write(temp_file, expected);

    auto result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(result);
    EXPECT_EQ(result.value, expected);
}

TEST_F(JsonReaderWriterTest, ReadArray)
{
    json expected = json::array({1, 2, 3});
    std::string temp_file = createTempFile("test.json");
    writer_.write(temp_file, expected);

    auto result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(result);
    EXPECT_EQ(result.value, expected);
}

// ========== Round-Trip Tests ==========

TEST_F(JsonReaderWriterTest, RoundTrip_SimpleObject)
{
    json original = {
        {"name", "John"},
        {"age", 30},
        {"email", "john@example.com"}
    };
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, original);
}

TEST_F(JsonReaderWriterTest, RoundTrip_NestedObject)
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
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, original);
}

TEST_F(JsonReaderWriterTest, RoundTrip_ArrayOfObjects)
{
    json original = json::array({
        json{{"id", 1}, {"name", "Alice"}},
        json{{"id", 2}, {"name", "Bob"}},
        json{{"id", 3}, {"name", "Charlie"}}
    });
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, original);
}

// ========== Pretty Printing Tests ==========

TEST_F(JsonReaderWriterTest, WritePretty_DefaultFormatting)
{
    json data = {{"name", "John"}, {"age", 30}};
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
    std::string content = readFile(temp_file);
    EXPECT_NE(content.find('\n'), std::string::npos);  // Has newlines
}

TEST_F(JsonReaderWriterTest, WritePretty_NestedStructure)
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
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
    std::string content = readFile(temp_file);
    EXPECT_GT(content.length(), 0);
}

// ========== Error Handling Tests ==========

TEST_F(JsonReaderWriterTest, ReadNonExistent_File)
{
    std::string non_existent = temp_dir_ + "/non_existent_file.json";

    auto result = reader_.read(non_existent);

    ASSERT_INVALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, ReadMalformed_JSON)
{
    std::string temp_file = createTempFile("test.json");
    std::ofstream file(temp_file);
    file << "{ invalid json }";
    file.close();

    auto result = reader_.read(temp_file);

    ASSERT_INVALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, ReadEmpty_File)
{
    std::string temp_file = createTempFile("test.json");
    std::ofstream file(temp_file);
    file.close();

    auto result = reader_.read(temp_file);

    ASSERT_INVALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, WriteToInvalidPath)
{
    json data = {{"test", "data"}};
    std::string invalid_path = "/invalid/nonexistent/path/file.json";

    auto result = writer_.write(invalid_path, data);

    ASSERT_INVALID_RESULT(result);
}

// ========== Unicode Handling Tests ==========

TEST_F(JsonReaderWriterTest, WriteUnicode_Strings)
{
    json data = {
        {"english", "Hello"},
        {"chinese", "你好"},
        {"emoji", "😀"}
    };
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, RoundTrip_Unicode)
{
    json original = {
        {"name", "José"},
        {"city", "São Paulo"},
        {"greeting", "Здравствуй"}
    };
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, original);
}

// ========== Special Characters Tests ==========

TEST_F(JsonReaderWriterTest, SpecialCharacters_Escaped)
{
    json data = {
        {"quotes", "He said \"Hello\""},
        {"backslash", "path\\to\\file"},
        {"newline", "line1\nline2"},
        {"tab", "col1\tcol2"}
    };
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, data).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value["quotes"], "He said \"Hello\"");
}

// ========== Large Data Tests ==========

TEST_F(JsonReaderWriterTest, LargeArray_ManyElements)
{
    json data = json::array();
    for (int i = 0; i < 1000; ++i)
    {
        data.push_back(i);
    }
    std::string temp_file = createTempFile("test.json");

    auto result = writer_.write(temp_file, data);

    ASSERT_VALID_RESULT(result);
}

TEST_F(JsonReaderWriterTest, RoundTrip_LargeArray)
{
    json original = json::array();
    for (int i = 0; i < 100; ++i)
    {
        original.push_back(json{
            {"index", i},
            {"name", "Item" + std::to_string(i)},
            {"value", i * 1.5}
        });
    }
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value.size(), original.size());
}

// ========== Type Preservation Tests ==========

TEST_F(JsonReaderWriterTest, TypePreservation_Integer)
{
    json original = {{"count", 42}};
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_TRUE(read_result.value["count"].is_number_integer());
}

TEST_F(JsonReaderWriterTest, TypePreservation_Float)
{
    json original = {{"price", 19.99}};
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_TRUE(read_result.value["price"].is_number_float());
}

TEST_F(JsonReaderWriterTest, TypePreservation_Boolean)
{
    json original = {{"active", true}, {"deleted", false}};
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_TRUE(read_result.value["active"].is_boolean());
    EXPECT_EQ(read_result.value["active"], true);
}

TEST_F(JsonReaderWriterTest, TypePreservation_Null)
{
    json original = {{"data", nullptr}};
    std::string temp_file = createTempFile("test.json");

    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_TRUE(read_result.value["data"].is_null());
}

// ========== Field Order Tests ==========

TEST_F(JsonReaderWriterTest, FieldOrder_Preserved)
{
    json original = json::object();
    original["z_field"] = "last";
    original["a_field"] = "first";
    original["m_field"] = "middle";

    std::string temp_file = createTempFile("test.json");
    ASSERT_TRUE(writer_.write(temp_file, original).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    // JSON objects are unordered, so we just verify all fields are present
    EXPECT_TRUE(read_result.value.contains("z_field"));
    EXPECT_TRUE(read_result.value.contains("a_field"));
    EXPECT_TRUE(read_result.value.contains("m_field"));
}

// ========== Configuration File Tests ==========

TEST_F(JsonReaderWriterTest, RoundTrip_Configuration)
{
    json config = createValidConfig();
    std::string temp_file = createTempFile("config.json");

    ASSERT_TRUE(writer_.write(temp_file, config).ok);
    auto read_result = reader_.read(temp_file);

    ASSERT_VALID_RESULT(read_result);
    EXPECT_EQ(read_result.value, config);
}
