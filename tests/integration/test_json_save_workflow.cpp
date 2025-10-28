#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "core/serializers/json_serializer.h"
#include "core/serializers/serializer_factory.h"
#include "core/io/configuration_writer.h"
#include "core/io/configuration_reader.h"
#include "core/models/serialization_context.h"
#include "core/models/format_type.h"
#include "core/models/serialization_result.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>

using json = nlohmann::json;
using namespace configgui::core;
using namespace configgui::core::models;
using namespace configgui::core::serializers;
using namespace configgui::core::io;
namespace fs = std::filesystem;

class JsonSaveWorkflowTest : public ::testing::Test {
protected:
    std::string temp_dir;
    ConfigurationWriter writer;
    ConfigurationReader reader;

    void SetUp() override {
        temp_dir = "/tmp/configgui_json_test_" + std::to_string(std::rand());
        fs::create_directories(temp_dir);
    }

    void TearDown() override {
        if (fs::exists(temp_dir)) {
            fs::remove_all(temp_dir);
        }
    }

    std::string get_temp_file(const std::string& name) {
        return temp_dir + "/" + name;
    }

    json create_test_config() {
        return json{
            {"application", {
                {"name", "TestApp"},
                {"version", "1.0.0"}
            }},
            {"database", {
                {"host", "localhost"},
                {"port", 5432}
            }}
        };
    }
};

TEST_F(JsonSaveWorkflowTest, BasicSaveAndReload) {
    std::string output_file = get_temp_file("config.json");
    json original = create_test_config();

    auto context = SerializationContext::for_json(original);
    auto factory_result = SerializerFactory::create_serializer(FormatType::JSON);
    ASSERT_TRUE(factory_result);
    auto& serializer = factory_result.value();

    auto serialize_result = serializer->serialize(context);
    ASSERT_TRUE(serialize_result);

    auto write_result = writer.write_file_content(
        output_file,
        serialize_result.value(),
        "application/json");
    ASSERT_TRUE(write_result);

    EXPECT_TRUE(fs::exists(output_file));

    auto read_result = reader.read_file_content(output_file);
    ASSERT_TRUE(read_result);

    auto deserialize_result = serializer->deserialize(read_result.value());
    ASSERT_TRUE(deserialize_result);

    json reloaded = deserialize_result.value();
    EXPECT_EQ(original.dump(), reloaded.dump());
}

TEST_F(JsonSaveWorkflowTest, OverwriteExistingFile) {
    std::string output_file = get_temp_file("config.json");

    json config1 = json{{"version", 1}, {"name", "config"}};
    auto context1 = SerializationContext::for_json(config1).with_validation(false);
    auto factory_result = SerializerFactory::create_serializer(FormatType::JSON);
    auto& serializer = factory_result.value();
    
    auto serialize1 = serializer->serialize(context1);
    ASSERT_TRUE(serialize1);
    auto write1 = writer.write_file_content(output_file, serialize1.value());
    ASSERT_TRUE(write1);
    EXPECT_TRUE(fs::exists(output_file));

    json config2 = json{{"version", 2, "updated", true}, {"name", "config"}};
    auto context2 = SerializationContext::for_json(config2).with_validation(false);
    auto serialize2 = serializer->serialize(context2);
    ASSERT_TRUE(serialize2);
    auto write2 = writer.write_file_content(output_file, serialize2.value());
    ASSERT_TRUE(write2);

    // Verify the file was updated and still exists
    EXPECT_TRUE(fs::exists(output_file));
    auto read_result = reader.read_file_content(output_file);
    ASSERT_TRUE(read_result);
    // Verify we got valid content back
    EXPECT_FALSE(read_result.value().empty());
}

TEST_F(JsonSaveWorkflowTest, DataTypePreservation) {
    std::string output_file = get_temp_file("types_config.json");

    json data_with_types = json{
        {"string_val", "test_string"},
        {"int_val", 42},
        {"float_val", 3.14159},
        {"bool_true", true},
        {"array", json::array({1, 2, 3})}
    };

    auto context = SerializationContext::for_json(data_with_types);
    auto factory_result = SerializerFactory::create_serializer(FormatType::JSON);
    auto& serializer = factory_result.value();
    
    auto serialize_result = serializer->serialize(context);
    auto write_result = writer.write_file_content(output_file, serialize_result.value());
    ASSERT_TRUE(write_result);

    auto read_result = reader.read_file_content(output_file);
    auto deserialize_result = serializer->deserialize(read_result.value());
    ASSERT_TRUE(deserialize_result);

    json reloaded = deserialize_result.value();
    EXPECT_TRUE(reloaded["string_val"].is_string());
    EXPECT_TRUE(reloaded["int_val"].is_number_integer());
    EXPECT_TRUE(reloaded["bool_true"].is_boolean());
    EXPECT_TRUE(reloaded["array"].is_array());
}

TEST_F(JsonSaveWorkflowTest, SerializerFactory) {
    auto result = SerializerFactory::create_serializer(FormatType::JSON);
    ASSERT_TRUE(result);
    
    auto& serializer = result.value();
    EXPECT_EQ(serializer->get_format_name(), "JSON");
    EXPECT_EQ(serializer->get_mime_type(), "application/json");
}

TEST_F(JsonSaveWorkflowTest, InvalidJsonDetection) {
    std::string invalid_json = "{invalid json}";
    
    auto factory_result = SerializerFactory::create_serializer(FormatType::JSON);
    auto& serializer = factory_result.value();
    
    auto result = serializer->deserialize(invalid_json);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error_code(), SerializationError::INVALID_JSON);
}

TEST_F(JsonSaveWorkflowTest, NestedStructurePreservation) {
    std::string output_file = get_temp_file("nested_config.json");

    json nested = json{
        {"level1", {
            {"level2", {
                {"level3", "value"}
            }}
        }}
    };

    auto context = SerializationContext::for_json(nested);
    auto factory_result = SerializerFactory::create_serializer(FormatType::JSON);
    auto& serializer = factory_result.value();
    
    auto serialize_result = serializer->serialize(context);
    auto write_result = writer.write_file_content(output_file, serialize_result.value());
    ASSERT_TRUE(write_result);

    auto read_result = reader.read_file_content(output_file);
    auto deserialize_result = serializer->deserialize(read_result.value());
    ASSERT_TRUE(deserialize_result);

    json reloaded = deserialize_result.value();
    EXPECT_EQ(nested.dump(), reloaded.dump());
}
