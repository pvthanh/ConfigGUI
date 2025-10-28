// SPDX-License-Identifier: MIT
// INI Parser Unit Tests

#include <gtest/gtest.h>
#include "../../../src/core/io/ini_parser.h"
#include "../../../src/core/io/ini_reader.h"
#include "../../../src/core/io/ini_writer.h"

using json = nlohmann::ordered_json;

/**
 * Test: Simple key-value pairs without sections
 */
TEST(INIParserTest, SimpleFlatKeyValues)
{
    std::string ini_content = R"(
key1=value1
key2=value2
key3=value3
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success()) << "Parse failed: " << result.error();

    json parsed = result.value();
    EXPECT_EQ(parsed["key1"], "value1");
    EXPECT_EQ(parsed["key2"], "value2");
    EXPECT_EQ(parsed["key3"], "value3");
}

/**
 * Test: Section headers with nested key-value pairs
 */
TEST(INIParserTest, SectionHeaders)
{
    std::string ini_content = R"(
[Database]
host=localhost
port=5432

[Server]
timeout=30
debug=false
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    ASSERT_TRUE(parsed.contains("Database"));
    ASSERT_TRUE(parsed.contains("Server"));

    EXPECT_EQ(parsed["Database"]["host"], "localhost");
    EXPECT_EQ(parsed["Database"]["port"], 5432);
    EXPECT_EQ(parsed["Server"]["timeout"], 30);
    EXPECT_FALSE(parsed["Server"]["debug"]);
}

/**
 * Test: Nested section headers (dotted notation)
 */
TEST(INIParserTest, NestedSections)
{
    std::string ini_content = R"(
[Server]
name=MainServer

[Server.Http]
port=8080
enabled=true

[Server.Https]
port=8443
enabled=true
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_EQ(parsed["Server"]["name"], "MainServer");
    EXPECT_EQ(parsed["Server"]["Http"]["port"], 8080);
    EXPECT_TRUE(parsed["Server"]["Http"]["enabled"]);
    EXPECT_EQ(parsed["Server"]["Https"]["port"], 8443);
}

/**
 * Test: Type inference - booleans
 */
TEST(INIParserTest, BooleanTypeInference)
{
    std::string ini_content = R"(
[Config]
enabled=true
disabled=false
maybe_true=True
maybe_false=FALSE
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_TRUE(parsed["Config"]["enabled"].is_boolean());
    EXPECT_TRUE(parsed["Config"]["enabled"]);
    EXPECT_TRUE(parsed["Config"]["disabled"].is_boolean());
    EXPECT_FALSE(parsed["Config"]["disabled"]);
    EXPECT_TRUE(parsed["Config"]["maybe_true"].is_boolean());
    EXPECT_TRUE(parsed["Config"]["maybe_false"].is_boolean());
}

/**
 * Test: Type inference - integers
 */
TEST(INIParserTest, IntegerTypeInference)
{
    std::string ini_content = R"(
[Numbers]
positive=123
negative=-456
zero=0
large=999999
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_TRUE(parsed["Numbers"]["positive"].is_number_integer());
    EXPECT_EQ(parsed["Numbers"]["positive"], 123);
    EXPECT_EQ(parsed["Numbers"]["negative"], -456);
    EXPECT_EQ(parsed["Numbers"]["zero"], 0);
    EXPECT_EQ(parsed["Numbers"]["large"], 999999);
}

/**
 * Test: Type inference - floats
 */
TEST(INIParserTest, FloatTypeInference)
{
    std::string ini_content = R"(
[Floats]
pi=3.14159
negative=-2.5
scientific=1.23e-4
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_TRUE(parsed["Floats"]["pi"].is_number_float());
    EXPECT_NEAR(parsed["Floats"]["pi"].get<double>(), 3.14159, 0.00001);
    EXPECT_NEAR(parsed["Floats"]["negative"].get<double>(), -2.5, 0.00001);
}

/**
 * Test: Type inference - strings
 */
TEST(INIParserTest, StringTypeInference)
{
    std::string ini_content = R"(
[Strings]
text=Hello World
path=/home/user/config
url=https://example.com
mixed=Value with 123 and true
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_EQ(parsed["Strings"]["text"], "Hello World");
    EXPECT_EQ(parsed["Strings"]["path"], "/home/user/config");
    EXPECT_EQ(parsed["Strings"]["url"], "https://example.com");
    EXPECT_EQ(parsed["Strings"]["mixed"], "Value with 123 and true");
}

/**
 * Test: Array notation - simple arrays
 */
TEST(INIParserTest, SimpleArrayNotation)
{
    std::string ini_content = R"(
[Arrays]
items[0]=first
items[1]=second
items[2]=third

numbers[0]=10
numbers[1]=20
numbers[2]=30
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    ASSERT_TRUE(parsed["Arrays"]["items"].is_array());
    EXPECT_EQ(parsed["Arrays"]["items"][0], "first");
    EXPECT_EQ(parsed["Arrays"]["items"][1], "second");
    EXPECT_EQ(parsed["Arrays"]["items"][2], "third");

    ASSERT_TRUE(parsed["Arrays"]["numbers"].is_array());
    EXPECT_EQ(parsed["Arrays"]["numbers"][0], 10);
    EXPECT_EQ(parsed["Arrays"]["numbers"][1], 20);
    EXPECT_EQ(parsed["Arrays"]["numbers"][2], 30);
}

/**
 * Test: Escape sequences - newlines, tabs, etc.
 */
TEST(INIParserTest, EscapeSequences)
{
    std::string ini_content = R"(
[Escapes]
newline=Line1\nLine2
tab=Col1\tCol2
carriage=Text\rEnd
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_EQ(parsed["Escapes"]["newline"], "Line1\nLine2");
    EXPECT_EQ(parsed["Escapes"]["tab"], "Col1\tCol2");
    EXPECT_EQ(parsed["Escapes"]["carriage"], "Text\rEnd");
}

/**
 * Test: Comments - semicolon and hash prefixes
 */
TEST(INIParserTest, CommentHandling)
{
    std::string ini_content = R"(
; This is a comment
[Config]
; Another comment
key1=value1
# Hash comment
key2=value2
# Multiple comment lines
# should be ignored
key3=value3
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_EQ(parsed["Config"]["key1"], "value1");
    EXPECT_EQ(parsed["Config"]["key2"], "value2");
    EXPECT_EQ(parsed["Config"]["key3"], "value3");
    EXPECT_EQ(parsed["Config"].size(), 3);
}

/**
 * Test: Empty lines are skipped
 */
TEST(INIParserTest, EmptyLinesSkipped)
{
    std::string ini_content = R"(
[Section1]
key1=value1

key2=value2


key3=value3

[Section2]

key4=value4
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_EQ(parsed["Section1"].size(), 3);
    EXPECT_EQ(parsed["Section2"].size(), 1);
}

/**
 * Test: INIReader wrapper - read from string
 */
TEST(INIReaderTest, ReadString)
{
    std::string ini_content = R"(
[Database]
host=localhost
port=5432
)";

    auto result = configgui::io::INIReader::readString(ini_content);
    ASSERT_TRUE(result.is_success()) << "Read failed: " << result.error();

    json parsed = result.value();
    EXPECT_EQ(parsed["Database"]["host"], "localhost");
    EXPECT_EQ(parsed["Database"]["port"], 5432);
}

/**
 * Test: INIWriter - convert JSON to INI string
 */
TEST(INIWriterTest, ToStringSimple)
{
    json config = {
        {"Database", {
            {"host", "localhost"},
            {"port", 5432},
            {"enabled", true}
        }},
        {"Server", {
            {"timeout", 30}
        }}
    };

    auto result = configgui::io::INIWriter::toString(config);
    ASSERT_TRUE(result.is_success()) << "Conversion failed: " << result.error().message;

    std::string ini_output = result.value();

    // Verify sections exist
    EXPECT_NE(ini_output.find("[Database]"), std::string::npos);
    EXPECT_NE(ini_output.find("[Server]"), std::string::npos);

    // Verify values exist
    EXPECT_NE(ini_output.find("host=localhost"), std::string::npos);
    EXPECT_NE(ini_output.find("port=5432"), std::string::npos);
    EXPECT_NE(ini_output.find("enabled=true"), std::string::npos);
    EXPECT_NE(ini_output.find("timeout=30"), std::string::npos);
}

/**
 * Test: Round-trip - Parse INI to JSON, then convert back to INI
 */
TEST(INIRoundTripTest, SimpleRoundTrip)
{
    std::string original_ini = R"(
[Database]
host=localhost
port=5432
enabled=true

[Cache]
ttl=3600
)";

    // Parse INI to JSON
    auto parse_result = configgui::io::INIParser::parse(original_ini);
    ASSERT_TRUE(parse_result.is_success());
    json parsed = parse_result.value();

    // Convert back to INI
    auto write_result = configgui::io::INIWriter::toString(parsed);
    ASSERT_TRUE(write_result.is_success());
    std::string regenerated_ini = write_result.value();

    // Parse again to verify structure is preserved
    auto parse_result2 = configgui::io::INIParser::parse(regenerated_ini);
    ASSERT_TRUE(parse_result2.is_success());
    json parsed2 = parse_result2.value();

    // Verify both parsings are identical
    EXPECT_EQ(parsed, parsed2);
}

/**
 * Test: Complex nested structure
 */
TEST(INIParserTest, ComplexNested)
{
    std::string ini_content = R"(
[Application]
name=MyApp
version=1.0

[Application.Database]
host=localhost
port=5432

[Application.Database.Options]
ssl=true
timeout=30

[Application.Cache]
enabled=true
ttl=3600
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_EQ(parsed["Application"]["name"], "MyApp");
    EXPECT_EQ(parsed["Application"]["Database"]["host"], "localhost");
    EXPECT_EQ(parsed["Application"]["Database"]["Options"]["ssl"], true);
    EXPECT_EQ(parsed["Application"]["Database"]["Options"]["timeout"], 30);
    EXPECT_EQ(parsed["Application"]["Cache"]["ttl"], 3600);
}

/**
 * Test: Whitespace handling - trimming around equals
 */
TEST(INIParserTest, WhitespaceHandling)
{
    std::string ini_content = R"(
[Config]
  key1  =  value1  
key2    =value2
key3=    value3
  key4=value4
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_EQ(parsed["Config"]["key1"], "value1");
    EXPECT_EQ(parsed["Config"]["key2"], "value2");
    EXPECT_EQ(parsed["Config"]["key3"], "value3");
    EXPECT_EQ(parsed["Config"]["key4"], "value4");
}

/**
 * Test: Mixed array and object properties
 */
TEST(INIParserTest, MixedArrayAndObject)
{
    std::string ini_content = R"(
[Database]
host=localhost
port=5432

[Servers]
servers[0]=db1.local
servers[1]=db2.local
servers[2]=db3.local
ttl=60
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_EQ(parsed["Database"]["host"], "localhost");
    EXPECT_EQ(parsed["Servers"]["servers"].size(), 3);
    EXPECT_EQ(parsed["Servers"]["ttl"], 60);
}

/**
 * Test: Error handling - invalid JSON input for writer
 */
TEST(INIWriterTest, InvalidInput)
{
    json invalid_input = json::array();  // Arrays cannot be converted to INI
    auto result = configgui::io::INIWriter::toString(invalid_input);
    EXPECT_TRUE(result.is_failure());
}

/**
 * Test: Empty INI content
 */
TEST(INIParserTest, EmptyContent)
{
    std::string ini_content = "";
    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_TRUE(parsed.is_object());
    EXPECT_TRUE(parsed.empty());
}

/**
 * Test: Only comments and empty lines
 */
TEST(INIParserTest, OnlyCommentsAndEmptyLines)
{
    std::string ini_content = R"(
; This is a comment
# Another comment
; Yet another comment

# More comments
)";

    auto result = configgui::io::INIParser::parse(ini_content);
    ASSERT_TRUE(result.is_success());

    json parsed = result.value();
    EXPECT_TRUE(parsed.empty());
}
