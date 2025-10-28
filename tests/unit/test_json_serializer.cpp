#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "core/serializers/json_serializer.h"
#include "core/models/serialization_context.h"
#include <fstream>
#include <sstream>

using json = nlohmann::json;
using namespace configgui::core;

class JsonSerializerTest : public ::testing::Test {
protected:
    JsonSerializer serializer;
    SerializationContext context;

    void SetUp() override {
        context = SerializationContext::for_json();
    }

    // Helper to load JSON from file
    json load_json_file(const std::string& path) {
        std::ifstream file(path);
        return json::parse(file);
    }

    // Helper to create test JSON
    json create_simple_json() {
        return json{
            {"application", {
                {"name", "TestApp"},
                {"version", "1.0.0"}
            }},
            {"settings", {
                {"debug_mode", true},
                {"port", 8080},
                {"timeout", 30.5}
            }}
        };
    }

    // Helper to create complex nested JSON
    json create_complex_json() {
        return json{
            {"database", {
                {"hosts", json::array({
                    {"name", "db1", {"ip", "192.168.1.1"}},
                    {"name", "db2", {"ip", "192.168.1.2"}}
                })},
                {"credentials", {
                    {"username", "admin"},
                    {"password", "secret"}
                }},
                {"options", {
                    {"timeout", 5000},
                    {"retry_count", 3},
                    {"enabled", true}
                }}
            }},
            {"logging", {
                {"level", "DEBUG"},
                {"file", "/var/log/app.log"},
                {"max_size", 10485760}
            }}
        };
    }
};

// Test serialization of simple types
TEST_F(JsonSerializerTest, SerializeSimpleTypes) {
    json simple = json{
        {"name", "test"},
        {"count", 42},
        {"ratio", 3.14},
        {"enabled", true}
    };

    auto result = serializer.serialize(simple, context);
    ASSERT_TRUE(result) << "Serialization failed";

    std::string serialized = result.value();
    EXPECT_FALSE(serialized.empty());
    EXPECT_NE(serialized.find("test"), std::string::npos);
    EXPECT_NE(serialized.find("42"), std::string::npos);
}

// Test serialization of nested objects
TEST_F(JsonSerializerTest, SerializeNestedObjects) {
    json nested = json{
        {"level1", {
            {"level2", {
                {"level3", "value"}
            }}
        }}
    };

    auto result = serializer.serialize(nested, context);
    ASSERT_TRUE(result);

    std::string serialized = result.value();
    json reparsed = json::parse(serialized);
    EXPECT_EQ(reparsed["level1"]["level2"]["level3"], "value");
}

// Test serialization of arrays
TEST_F(JsonSerializerTest, SerializeArrays) {
    json with_arrays = json{
        {"items", json::array({
            {"id", 1, {"name", "item1"}},
            {"id", 2, {"name", "item2"}},
            {"id", 3, {"name", "item3"}}
        })},
        {"numbers", json::array({1, 2, 3, 4, 5})}
    };

    auto result = serializer.serialize(with_arrays, context);
    ASSERT_TRUE(result);

    std::string serialized = result.value();
    json reparsed = json::parse(serialized);
    EXPECT_EQ(reparsed["items"].size(), 3);
    EXPECT_EQ(reparsed["numbers"][0], 1);
}

// Test deserialization of valid JSON
TEST_F(JsonSerializerTest, DeserializeValidJson) {
    std::string valid_json = R"({
        "key1": "value1",
        "key2": 42,
        "nested": {
            "key3": "value3"
        }
    })";

    auto result = serializer.deserialize(valid_json, context);
    ASSERT_TRUE(result) << "Deserialization failed";

    json deserialized = result.value();
    EXPECT_EQ(deserialized["key1"], "value1");
    EXPECT_EQ(deserialized["key2"], 42);
    EXPECT_EQ(deserialized["nested"]["key3"], "value3");
}

// Test deserialization of complex JSON
TEST_F(JsonSerializerTest, DeserializeComplexJson) {
    std::string complex_json = R"({
        "database": {
            "hosts": [
                {"name": "db1", "ip": "192.168.1.1"},
                {"name": "db2", "ip": "192.168.1.2"}
            ],
            "credentials": {
                "username": "admin",
                "password": "secret"
            }
        }
    })";

    auto result = serializer.deserialize(complex_json, context);
    ASSERT_TRUE(result);

    json deserialized = result.value();
    EXPECT_EQ(deserialized["database"]["hosts"].size(), 2);
    EXPECT_EQ(deserialized["database"]["credentials"]["username"], "admin");
}

// Test detection of invalid JSON
TEST_F(JsonSerializerTest, DetectInvalidJson) {
    std::string invalid_json = R"({
        "key": "value"
        "missing_comma": true
    })";

    auto result = serializer.deserialize(invalid_json, context);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error_code(), SerializationError::InvalidJson);
}

// Test detection of malformed JSON
TEST_F(JsonSerializerTest, DetectMalformedJson) {
    std::string malformed = "{incomplete";

    auto result = serializer.deserialize(malformed, context);
    EXPECT_FALSE(result);
}

// Test pretty-printing functionality
TEST_F(JsonSerializerTest, PrettyPrintingFormatting) {
    json data = create_simple_json();
    auto pretty_context = SerializationContext::for_json().with_pretty_print(true);

    auto result = serializer.serialize(data, pretty_context);
    ASSERT_TRUE(result);

    std::string serialized = result.value();
    // Check for newlines and indentation in pretty-printed output
    EXPECT_NE(serialized.find('\n'), std::string::npos);
}

// Test compact formatting (without pretty-printing)
TEST_F(JsonSerializerTest, CompactFormatting) {
    json data = create_simple_json();
    auto compact_context = SerializationContext::for_json().with_pretty_print(false);

    auto result = serializer.serialize(data, compact_context);
    ASSERT_TRUE(result);

    std::string serialized = result.value();
    // Should be compact with minimal whitespace
    size_t newline_count = std::count(serialized.begin(), serialized.end(), '\n');
    EXPECT_LE(newline_count, 1);  // At most one newline at end
}

// Test round-trip preservation
TEST_F(JsonSerializerTest, RoundTripPreservation) {
    json original = create_complex_json();

    // Serialize to string
    auto serialize_result = serializer.serialize(original, context);
    ASSERT_TRUE(serialize_result);

    // Deserialize back
    auto deserialize_result = serializer.deserialize(serialize_result.value(), context);
    ASSERT_TRUE(deserialize_result);

    json recovered = deserialize_result.value();
    EXPECT_EQ(original.dump(), recovered.dump());
}

// Test error case: empty string
TEST_F(JsonSerializerTest, ErrorEmptyString) {
    std::string empty;
    auto result = serializer.deserialize(empty, context);
    EXPECT_FALSE(result);
}

// Test error case: null pointer/invalid input
TEST_F(JsonSerializerTest, ErrorInvalidInput) {
    std::string invalid = "null";
    auto result = serializer.deserialize(invalid, context);
    // JSON null is technically valid JSON, should be handled gracefully
    EXPECT_TRUE(result);
}

// Test MIME type
TEST_F(JsonSerializerTest, MimeType) {
    std::string mime = serializer.get_mime_type();
    EXPECT_EQ(mime, "application/json");
}

// Test format name
TEST_F(JsonSerializerTest, FormatName) {
    std::string name = serializer.get_format_name();
    EXPECT_EQ(name, "JSON");
}

// Test pre-serialization validation
TEST_F(JsonSerializerTest, PreSerializationValidation) {
    json data = create_simple_json();
    context = SerializationContext::for_json().with_validation(true);

    auto validation_result = serializer.validate_pre_serialization(data, context);
    // Should either return no error or return a validation error if data is invalid
    // Valid data should pass
    if (validation_result) {
        EXPECT_TRUE(validation_result.value().empty() || 
                   validation_result.value() == "");
    }
}

// Test serialization with special characters
TEST_F(JsonSerializerTest, SpecialCharactersHandling) {
    json data = json{
        {"text", "String with \"quotes\" and \\ backslashes"},
        {"unicode", "Unicode: ñ, é, 中文"},
        {"newlines", "Line1\nLine2\nLine3"}
    };

    auto serialize_result = serializer.serialize(data, context);
    ASSERT_TRUE(serialize_result);

    auto deserialize_result = serializer.deserialize(serialize_result.value(), context);
    ASSERT_TRUE(deserialize_result);

    json recovered = deserialize_result.value();
    EXPECT_EQ(recovered["text"], data["text"]);
    EXPECT_EQ(recovered["unicode"], data["unicode"]);
}

// Test serialization of empty objects
TEST_F(JsonSerializerTest, EmptyObjectSerialization) {
    json empty = json::object();
    auto result = serializer.serialize(empty, context);
    ASSERT_TRUE(result);
    
    auto deserialized = serializer.deserialize(result.value(), context);
    ASSERT_TRUE(deserialized);
    EXPECT_TRUE(deserialized.value().is_object());
}

// Test serialization of empty arrays
TEST_F(JsonSerializerTest, EmptyArraySerialization) {
    json empty_array = json::array();
    auto result = serializer.serialize(empty_array, context);
    ASSERT_TRUE(result);
    
    auto deserialized = serializer.deserialize(result.value(), context);
    ASSERT_TRUE(deserialized);
    EXPECT_TRUE(deserialized.value().is_array());
    EXPECT_EQ(deserialized.value().size(), 0);
}

// Test large JSON handling
TEST_F(JsonSerializerTest, LargeJsonHandling) {
    json large = json::object();
    for (int i = 0; i < 1000; ++i) {
        large[std::string("key_") + std::to_string(i)] = json::object({
            {"value", i},
            {"name", "item_" + std::to_string(i)}
        });
    }

    auto serialize_result = serializer.serialize(large, context);
    ASSERT_TRUE(serialize_result);
    EXPECT_GT(serialize_result.value().length(), 10000);

    auto deserialize_result = serializer.deserialize(serialize_result.value(), context);
    ASSERT_TRUE(deserialize_result);
    EXPECT_EQ(deserialize_result.value().size(), 1000);
}

// Test numeric precision
TEST_F(JsonSerializerTest, NumericPrecision) {
    json numbers = json{
        {"pi", 3.14159265359},
        {"large_int", 9007199254740991LL},
        {"small_decimal", 0.0000001}
    };

    auto serialize_result = serializer.serialize(numbers, context);
    ASSERT_TRUE(serialize_result);

    auto deserialize_result = serializer.deserialize(serialize_result.value(), context);
    ASSERT_TRUE(deserialize_result);

    json recovered = deserialize_result.value();
    EXPECT_DOUBLE_EQ(recovered["pi"], 3.14159265359);
}

// Test boolean value handling
TEST_F(JsonSerializerTest, BooleanHandling) {
    json bools = json{
        {"true_value", true},
        {"false_value", false},
        {"array_bools", json::array({true, false, true})}
    };

    auto serialize_result = serializer.serialize(bools, context);
    ASSERT_TRUE(serialize_result);

    auto deserialize_result = serializer.deserialize(serialize_result.value(), context);
    ASSERT_TRUE(deserialize_result);

    json recovered = deserialize_result.value();
    EXPECT_TRUE(recovered["true_value"].is_boolean());
    EXPECT_TRUE(recovered["true_value"].get<bool>());
    EXPECT_FALSE(recovered["false_value"].get<bool>());
}
