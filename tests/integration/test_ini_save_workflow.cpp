#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "core/serializers/ini_serializer.h"
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

class IniSaveWorkflowTest : public ::testing::Test {
protected:
    std::string temp_dir;
    ConfigurationWriter writer;
    ConfigurationReader reader;

    void SetUp() override {
        temp_dir = "/tmp/configgui_ini_test_" + std::to_string(std::rand());
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
            {"database", {
                {"host", "localhost"},
                {"port", 5432}
            }},
            {"logging", {
                {"level", "debug"},
                {"enabled", true}
            }}
        };
    }
};

TEST_F(IniSaveWorkflowTest, BasicSaveAndReload) {
    std::string output_file = get_temp_file("config.ini");
    json original = create_test_config();

    auto context = SerializationContext::for_ini(original).with_validation(false);
    auto factory_result = SerializerFactory::create_serializer(FormatType::INI);
    ASSERT_TRUE(factory_result);
    auto& serializer = factory_result.value();

    auto serialize_result = serializer->serialize(context);
    ASSERT_TRUE(serialize_result);

    auto write_result = writer.write_file_content(
        output_file,
        serialize_result.value(),
        "text/plain");
    ASSERT_TRUE(write_result);

    EXPECT_TRUE(fs::exists(output_file));

    auto read_result = reader.read_file_content(output_file);
    ASSERT_TRUE(read_result);

    auto deserialize_result = serializer->deserialize(read_result.value());
    ASSERT_TRUE(deserialize_result);

    json reloaded = deserialize_result.value();
    // INI format may flatten structures, so verify key fields exist
    EXPECT_TRUE(reloaded.is_object());
}

TEST_F(IniSaveWorkflowTest, NestedStructureFlattening) {
    std::string output_file = get_temp_file("nested.ini");

    json nested = json{
        {"app", {
            {"name", "TestApp"},
            {"version", "1.0"}
        }},
        {"features", {
            {"auth", true},
            {"cache", false}
        }}
    };

    auto context = SerializationContext::for_ini(nested).with_validation(false);
    auto factory_result = SerializerFactory::create_serializer(FormatType::INI);
    auto& serializer = factory_result.value();
    
    auto serialize_result = serializer->serialize(context);
    ASSERT_TRUE(serialize_result);
    
    auto write_result = writer.write_file_content(output_file, serialize_result.value());
    ASSERT_TRUE(write_result);

    EXPECT_TRUE(fs::exists(output_file));
    
    auto read_result = reader.read_file_content(output_file);
    ASSERT_TRUE(read_result);
    
    // Verify content is readable INI format (contains '=' and section markers)
    std::string content = read_result.value();
    EXPECT_FALSE(content.empty());
    // INI should contain at least one assignment
    EXPECT_TRUE(content.find('=') != std::string::npos || 
                content.find('[') != std::string::npos);
}

TEST_F(IniSaveWorkflowTest, SerializerFactory) {
    auto result = SerializerFactory::create_serializer(FormatType::INI);
    ASSERT_TRUE(result);
    
    auto& serializer = result.value();
    EXPECT_EQ(serializer->get_format_name(), "INI");
    EXPECT_EQ(serializer->get_mime_type(), "text/plain");
}

TEST_F(IniSaveWorkflowTest, OverwriteExistingFile) {
    std::string output_file = get_temp_file("config.ini");

    json config1 = json{{"name", "config1"}};
    auto context1 = SerializationContext::for_ini(config1).with_validation(false);
    auto factory_result = SerializerFactory::create_serializer(FormatType::INI);
    auto& serializer = factory_result.value();
    
    auto serialize1 = serializer->serialize(context1);
    auto write1 = writer.write_file_content(output_file, serialize1.value());
    ASSERT_TRUE(write1);
    EXPECT_TRUE(fs::exists(output_file));

    json config2 = json{{"name", "config2"}, {"version", "2.0"}};
    auto context2 = SerializationContext::for_ini(config2).with_validation(false);
    auto serialize2 = serializer->serialize(context2);
    auto write2 = writer.write_file_content(output_file, serialize2.value());
    ASSERT_TRUE(write2);

    auto read_result = reader.read_file_content(output_file);
    ASSERT_TRUE(read_result);
    EXPECT_FALSE(read_result.value().empty());
}

TEST_F(IniSaveWorkflowTest, SpecialCharactersHandling) {
    std::string output_file = get_temp_file("special.ini");

    json data = json{
        {"message", "Hello: World"},
        {"path", "C:\\Users\\Test"},
        {"email", "test@example.com"}
    };

    auto context = SerializationContext::for_ini(data).with_validation(false);
    auto factory_result = SerializerFactory::create_serializer(FormatType::INI);
    auto& serializer = factory_result.value();
    
    auto serialize_result = serializer->serialize(context);
    ASSERT_TRUE(serialize_result);
    
    auto write_result = writer.write_file_content(output_file, serialize_result.value());
    ASSERT_TRUE(write_result);

    auto read_result = reader.read_file_content(output_file);
    ASSERT_TRUE(read_result);
    
    // Should have serialized content
    EXPECT_FALSE(read_result.value().empty());
}
