// SPDX-License-Identifier: MIT
// Integration Tests - Complete Workflows

#include <gtest/gtest.h>
#include "src/core/schema_loader.h"
#include "src/core/schema_validator.h"
#include "src/io/json_reader.h"
#include "src/io/json_writer.h"
#include "src/io/yaml_reader.h"
#include "src/io/yaml_writer.h"
#include "src/core/configuration_data.h"
#include "src/core/form_state.h"
#include "common/test_fixtures.h"

using namespace configgui::io;
using namespace configgui::core;

// ========== Schema Loading Integration Tests ==========

class SchemaLoadingIntegrationTest : public SchemaTestFixture
{
protected:
    SchemaLoader schema_loader_;
};

TEST_F(SchemaLoadingIntegrationTest, LoadSchema_FromJson)
{
    json schema_json = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"age", {{"type", "integer"}}}
        }},
        {"required", {"name"}}
    };

    std::string temp_file = createTempFile("schema.json");
    configgui::io::JsonWriter().write(temp_file, schema_json);

    auto result = schema_loader_.loadSchema(temp_file);

    EXPECT_TRUE(result.ok);
}

// ========== Configuration Load-Edit-Save Workflow ==========

class ConfigurationWorkflowTest : public FileIOTestFixture
{
};

TEST_F(ConfigurationWorkflowTest, Workflow_LoadConfigJson)
{
    json config = createValidConfig();
    std::string config_file = createTempFile("config.json");
    
    JsonWriter writer;
    auto write_result = writer.write(config_file, config);
    
    EXPECT_TRUE(write_result.ok);
}

TEST_F(ConfigurationWorkflowTest, Workflow_LoadConfigYaml)
{
    json config = createValidConfig();
    std::string config_file = createTempFile("config.yaml");
    
    YamlWriter writer;
    auto write_result = writer.write(config_file, config);
    
    EXPECT_TRUE(write_result.ok);
}

TEST_F(ConfigurationWorkflowTest, Workflow_LoadEditSave_Json)
{
    json original_config = createValidConfig();
    std::string config_file = createTempFile("config.json");
    
    // Load
    JsonWriter writer;
    auto write_result = writer.write(config_file, original_config);
    EXPECT_TRUE(write_result.ok);
    
    // Read back
    JsonReader reader;
    auto read_result = reader.read(config_file);
    EXPECT_TRUE(read_result.ok);
    
    // Modify
    read_result.value["name"] = "Modified";
    
    // Save
    auto save_result = writer.write(config_file, read_result.value);
    EXPECT_TRUE(save_result.ok);
    
    // Verify
    auto verify_result = reader.read(config_file);
    EXPECT_EQ(verify_result.value["name"], "Modified");
}

TEST_F(ConfigurationWorkflowTest, Workflow_LoadEditSave_Yaml)
{
    json original_config = createValidConfig();
    std::string config_file = createTempFile("config.yaml");
    
    // Load
    YamlWriter writer;
    auto write_result = writer.write(config_file, original_config);
    EXPECT_TRUE(write_result.ok);
    
    // Read back
    YamlReader reader;
    auto read_result = reader.read(config_file);
    EXPECT_TRUE(read_result.ok);
    
    // Modify
    read_result.value["name"] = "Modified";
    
    // Save
    auto save_result = writer.write(config_file, read_result.value);
    EXPECT_TRUE(save_result.ok);
    
    // Verify
    auto verify_result = reader.read(config_file);
    EXPECT_EQ(verify_result.value["name"], "Modified");
}

// ========== Format Conversion Workflow ==========

TEST_F(ConfigurationWorkflowTest, Workflow_JsonToYamlConversion)
{
    json config = createValidConfig();
    std::string json_file = createTempFile("config.json");
    std::string yaml_file = createTempFile("config.yaml");
    
    // Write as JSON
    JsonWriter json_writer;
    auto json_write_result = json_writer.write(json_file, config);
    EXPECT_TRUE(json_write_result.ok);
    
    // Read JSON
    JsonReader json_reader;
    auto json_read_result = json_reader.read(json_file);
    EXPECT_TRUE(json_read_result.ok);
    
    // Write as YAML
    YamlWriter yaml_writer;
    auto yaml_write_result = yaml_writer.write(yaml_file, json_read_result.value);
    EXPECT_TRUE(yaml_write_result.ok);
    
    // Read YAML
    YamlReader yaml_reader;
    auto yaml_read_result = yaml_reader.read(yaml_file);
    EXPECT_TRUE(yaml_read_result.ok);
    
    // Verify data is same
    EXPECT_EQ(yaml_read_result.value, json_read_result.value);
}

TEST_F(ConfigurationWorkflowTest, Workflow_YamlToJsonConversion)
{
    json config = createValidConfig();
    std::string yaml_file = createTempFile("config.yaml");
    std::string json_file = createTempFile("config.json");
    
    // Write as YAML
    YamlWriter yaml_writer;
    auto yaml_write_result = yaml_writer.write(yaml_file, config);
    EXPECT_TRUE(yaml_write_result.ok);
    
    // Read YAML
    YamlReader yaml_reader;
    auto yaml_read_result = yaml_reader.read(yaml_file);
    EXPECT_TRUE(yaml_read_result.ok);
    
    // Write as JSON
    JsonWriter json_writer;
    auto json_write_result = json_writer.write(json_file, yaml_read_result.value);
    EXPECT_TRUE(json_write_result.ok);
    
    // Read JSON
    JsonReader json_reader;
    auto json_read_result = json_reader.read(json_file);
    EXPECT_TRUE(json_read_result.ok);
    
    // Verify data is same
    EXPECT_EQ(json_read_result.value, yaml_read_result.value);
}

// ========== Configuration Data Management Workflow ==========

TEST_F(ConfigurationWorkflowTest, Workflow_ConfigurationDataTracking)
{
    ConfigurationData config;
    
    EXPECT_FALSE(config.isDirty());
    
    config.setValue("key1", "value1");
    EXPECT_TRUE(config.isDirty());
    
    config.markClean();
    EXPECT_FALSE(config.isDirty());
    
    config.setValue("key2", "value2");
    EXPECT_TRUE(config.isDirty());
}

TEST_F(ConfigurationWorkflowTest, Workflow_ConfigurationDataPersistence)
{
    ConfigurationData config;
    config.setValue("name", "Test");
    config.setValue("value", 42);
    
    json exported = config.data;
    
    ConfigurationData config2;
    for (auto& [key, value] : exported.items())
    {
        config2.setValue(key, value);
    }
    
    EXPECT_EQ(config2.getValue("name"), "Test");
    EXPECT_EQ(config2.getValue("value"), 42);
}

// ========== Form State Management Workflow ==========

TEST_F(ConfigurationWorkflowTest, Workflow_FormStateCreation)
{
    FormState form_state;
    
    form_state.addField("name", "string", true);
    form_state.addField("email", "string", true);
    form_state.addField("age", "integer", false);
    
    EXPECT_EQ(form_state.getFields().size(), 3);
}

TEST_F(ConfigurationWorkflowTest, Workflow_FormStateValidation)
{
    FormState form_state;
    form_state.addField("name", "string", true);
    
    EXPECT_TRUE(form_state.isValid());
    
    form_state.addValidationError("name", "Name is required");
    EXPECT_FALSE(form_state.isValid());
    
    form_state.clearValidationErrors("name");
    EXPECT_TRUE(form_state.isValid());
}

// ========== Error Handling Workflow ==========

TEST_F(ConfigurationWorkflowTest, Workflow_HandleInvalidJson)
{
    std::string temp_file = createTempFile("invalid.json");
    std::ofstream file(temp_file);
    file << "{ invalid json }";
    file.close();
    
    JsonReader reader;
    auto result = reader.read(temp_file);
    
    EXPECT_FALSE(result.ok);
    EXPECT_GT(result.errors.size(), 0);
}

TEST_F(ConfigurationWorkflowTest, Workflow_HandleMissingFile)
{
    std::string non_existent = temp_dir_ + "/non_existent.json";
    
    JsonReader reader;
    auto result = reader.read(non_existent);
    
    EXPECT_FALSE(result.ok);
}

TEST_F(ConfigurationWorkflowTest, Workflow_HandleWriteError)
{
    std::string invalid_path = "/invalid/path/that/doesnt/exist/config.json";
    
    JsonWriter writer;
    auto result = writer.write(invalid_path, json{{"test", "data"}});
    
    EXPECT_FALSE(result.ok);
}

// ========== Complex Configuration Workflow ==========

TEST_F(ConfigurationWorkflowTest, Workflow_ComplexConfig_NestedObjects)
{
    json config = {
        {"app", {
            {"name", "MyApp"},
            {"version", "1.0.0"},
            {"settings", {
                {"theme", "dark"},
                {"language", "en"}
            }}
        }},
        {"database", {
            {"host", "localhost"},
            {"port", 5432}
        }}
    };
    
    std::string json_file = createTempFile("complex.json");
    JsonWriter writer;
    auto write_result = writer.write(json_file, config);
    
    EXPECT_TRUE(write_result.ok);
    
    JsonReader reader;
    auto read_result = reader.read(json_file);
    
    EXPECT_TRUE(read_result.ok);
    EXPECT_EQ(read_result.value["app"]["settings"]["theme"], "dark");
}

TEST_F(ConfigurationWorkflowTest, Workflow_ComplexConfig_Arrays)
{
    json config = {
        {"servers", json::array({
            json{{"name", "server1"}, {"port", 8080}},
            json{{"name", "server2"}, {"port", 8081}}
        })},
        {"features", json::array({"feature1", "feature2", "feature3"})}
    };
    
    std::string yaml_file = createTempFile("complex.yaml");
    YamlWriter writer;
    auto write_result = writer.write(yaml_file, config);
    
    EXPECT_TRUE(write_result.ok);
    
    YamlReader reader;
    auto read_result = reader.read(yaml_file);
    
    EXPECT_TRUE(read_result.ok);
    EXPECT_EQ(read_result.value["servers"].size(), 2);
    EXPECT_EQ(read_result.value["features"].size(), 3);
}

// ========== Multi-Format Round-Trip Tests ==========

TEST_F(ConfigurationWorkflowTest, Workflow_RoundTrip_JsonOnly)
{
    json original = createValidConfig();
    std::string file1 = createTempFile("file1.json");
    std::string file2 = createTempFile("file2.json");
    
    JsonWriter writer;
    JsonReader reader;
    
    // First write
    auto write1 = writer.write(file1, original);
    EXPECT_TRUE(write1.ok);
    
    // First read
    auto read1 = reader.read(file1);
    EXPECT_TRUE(read1.ok);
    
    // Second write
    auto write2 = writer.write(file2, read1.value);
    EXPECT_TRUE(write2.ok);
    
    // Second read
    auto read2 = reader.read(file2);
    EXPECT_TRUE(read2.ok);
    
    // Verify data integrity
    EXPECT_EQ(read2.value, original);
}

TEST_F(ConfigurationWorkflowTest, Workflow_RoundTrip_YamlOnly)
{
    json original = createValidConfig();
    std::string file1 = createTempFile("file1.yaml");
    std::string file2 = createTempFile("file2.yaml");
    
    YamlWriter writer;
    YamlReader reader;
    
    // First write
    auto write1 = writer.write(file1, original);
    EXPECT_TRUE(write1.ok);
    
    // First read
    auto read1 = reader.read(file1);
    EXPECT_TRUE(read1.ok);
    
    // Second write
    auto write2 = writer.write(file2, read1.value);
    EXPECT_TRUE(write2.ok);
    
    // Second read
    auto read2 = reader.read(file2);
    EXPECT_TRUE(read2.ok);
    
    // Verify data integrity
    EXPECT_EQ(read2.value, original);
}

TEST_F(ConfigurationWorkflowTest, Workflow_RoundTrip_Alternating)
{
    json original = createValidConfig();
    std::string json_file = createTempFile("data.json");
    std::string yaml_file = createTempFile("data.yaml");
    
    JsonWriter json_writer;
    YamlWriter yaml_writer;
    JsonReader json_reader;
    YamlReader yaml_reader;
    
    // JSON -> YAML -> JSON
    json_writer.write(json_file, original);
    auto json_data = json_reader.read(json_file);
    
    yaml_writer.write(yaml_file, json_data.value);
    auto yaml_data = yaml_reader.read(yaml_file);
    
    json_writer.write(json_file, yaml_data.value);
    auto final_json_data = json_reader.read(json_file);
    
    EXPECT_EQ(final_json_data.value, original);
}

// ========== Performance-Like Tests ==========

TEST_F(ConfigurationWorkflowTest, Workflow_ManyOperations)
{
    std::string json_file = createTempFile("performance.json");
    JsonWriter writer;
    JsonReader reader;
    
    json data = json::object();
    
    // Create large configuration
    for (int i = 0; i < 100; ++i)
    {
        data["key_" + std::to_string(i)] = {
            {"value", i},
            {"enabled", i % 2 == 0}
        };
    }
    
    auto write_result = writer.write(json_file, data);
    EXPECT_TRUE(write_result.ok);
    
    auto read_result = reader.read(json_file);
    EXPECT_TRUE(read_result.ok);
    EXPECT_EQ(read_result.value.size(), 100);
}

// ========== Error Recovery Workflow ==========

TEST_F(ConfigurationWorkflowTest, Workflow_RecoveryFromInvalidFile)
{
    std::string good_file = createTempFile("good.json");
    std::string bad_file = createTempFile("bad.json");
    
    json good_data = createValidConfig();
    
    // Write good data
    JsonWriter writer;
    writer.write(good_file, good_data);
    
    // Write bad data
    std::ofstream bad_stream(bad_file);
    bad_stream << "{ bad json }";
    bad_stream.close();
    
    JsonReader reader;
    
    // Try to read bad file
    auto bad_result = reader.read(bad_file);
    EXPECT_FALSE(bad_result.ok);
    
    // Should still be able to read good file
    auto good_result = reader.read(good_file);
    EXPECT_TRUE(good_result.ok);
}

// ========== Schema Validation Integration ==========

TEST_F(SchemaLoadingIntegrationTest, Workflow_SchemaValidation_Integration)
{
    // Create a simple schema
    Schema schema;
    SchemaField name_field;
    name_field.name = "name";
    name_field.type = "string";
    name_field.required = true;
    schema.fields.push_back(name_field);
    schema.required_fields = {"name"};
    
    // Create validator
    SchemaValidator validator;
    
    // Valid configuration
    json valid_config = {{"name", "John"}};
    auto valid_result = validator.validate(valid_config, schema);
    EXPECT_TRUE(valid_result.ok);
    
    // Invalid configuration
    json invalid_config = json::object();
    auto invalid_result = validator.validate(invalid_config, schema);
    EXPECT_FALSE(invalid_result.ok);
}
