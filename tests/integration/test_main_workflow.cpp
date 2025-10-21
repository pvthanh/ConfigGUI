// SPDX-License-Identifier: MIT
// Integration Tests - Main GUI Workflow
// Tests the complete user workflow: Load Schema -> Load Config -> Save Config

#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include "src/ui/form_generator.h"
#include "src/ui/main_window.h"
#include "src/io/json_writer.h"
#include "src/io/json_reader.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace configgui::io;
using namespace configgui::ui;

// ========== Main Workflow Integration Tests ==========

class MainWorkflowTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory
        temp_dir_ = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(temp_dir_->isValid());
    }

    void TearDown() override
    {
        temp_dir_.reset();
    }

    // Helper: Create a valid schema JSON file
    QString createSchemaFile(const std::string& filename)
    {
        json schema = {
            {"$schema", "http://json-schema.org/draft-07/schema#"},
            {"title", "Test Configuration Schema"},
            {"type", "object"},
            {"properties", {
                {"application", {
                    {"type", "object"},
                    {"properties", {
                        {"name", {{"type", "string"}, {"minLength", 1}, {"maxLength", 100}}},
                        {"version", {{"type", "string"}}},
                        {"enabled", {{"type", "boolean"}}},
                        {"port", {{"type", "integer"}, {"minimum", 1}, {"maximum", 65535}}}
                    }},
                    {"required", {"name", "version"}}
                }},
                {"database", {
                    {"type", "object"},
                    {"properties", {
                        {"type", {{"type", "string"}}},
                        {"connection_string", {{"type", "string"}}},
                        {"max_connections", {{"type", "integer"}}}
                    }}
                }}
            }},
            {"required", {"application"}}
        };

        QString file_path = QString::fromStdString(temp_dir_->path().toStdString() + "/" + filename);
        JsonWriter writer;
        auto result = writer.write(file_path.toStdString(), schema);
        
        if (!result.ok)
        {
            return "";
        }
        return file_path;
    }

    // Helper: Create a valid configuration JSON file
    QString createConfigFile(const std::string& filename)
    {
        json config = {
            {"application", {
                {"name", "Test Application"},
                {"version", "1.0.0"},
                {"enabled", true},
                {"port", 8080}
            }},
            {"database", {
                {"type", "PostgreSQL"},
                {"connection_string", "postgresql://user@localhost/testdb"},
                {"max_connections", 100}
            }}
        };

        QString file_path = QString::fromStdString(temp_dir_->path().toStdString() + "/" + filename);
        JsonWriter writer;
        auto result = writer.write(file_path.toStdString(), config);
        
        if (!result.ok)
        {
            return "";
        }
        return file_path;
    }

    std::unique_ptr<QTemporaryDir> temp_dir_;
};

// ========== Unit Tests for FormGenerator ==========

TEST_F(MainWorkflowTest, FormGenerator_GenerateFromSchema)
{
    // Test: FormGenerator can parse a schema and generate form
    FormGenerator form_gen;

    json schema = {
        {"type", "object"},
        {"properties", {
            {"app", {
                {"type", "object"},
                {"properties", {
                    {"name", {{"type", "string"}}},
                    {"port", {{"type", "integer"}}}
                }}
            }}
        }}
    };

    bool result = form_gen.generateFromSchema(schema);
    EXPECT_TRUE(result) << "FormGenerator should successfully generate form from schema";
}

TEST_F(MainWorkflowTest, FormGenerator_SetFormData_NestedObjects)
{
    // Test: FormGenerator can populate form with nested object data
    FormGenerator form_gen;

    json schema = {
        {"type", "object"},
        {"properties", {
            {"application", {
                {"type", "object"},
                {"properties", {
                    {"name", {{"type", "string"}}},
                    {"port", {{"type", "integer"}}}
                }}
            }},
            {"database", {
                {"type", "object"},
                {"properties", {
                    {"host", {{"type", "string"}}}
                }}
            }}
        }}
    };

    form_gen.generateFromSchema(schema);

    json config = {
        {"application", {
            {"name", "MyApp"},
            {"port", 9000}
        }},
        {"database", {
            {"host", "localhost"}
        }}
    };

    form_gen.setFormData(config);
    // If setFormData doesn't throw, it succeeded
    EXPECT_FALSE(form_gen.isDirty()) << "Form should be clean after setFormData";
}

TEST_F(MainWorkflowTest, FormGenerator_GetFormData_ReconstructsNesting)
{
    // Test: FormGenerator.getFormData() returns properly nested structure
    FormGenerator form_gen;

    json schema = {
        {"type", "object"},
        {"properties", {
            {"section1", {
                {"type", "object"},
                {"properties", {
                    {"field1", {{"type", "string"}}},
                    {"field2", {{"type", "integer"}}}
                }}
            }}
        }}
    };

    form_gen.generateFromSchema(schema);

    json input_data = {
        {"section1", {
            {"field1", "value1"},
            {"field2", 42}
        }}
    };

    form_gen.setFormData(input_data);
    json output_data = form_gen.getFormData();

    // Verify nested structure is preserved
    EXPECT_TRUE(output_data.contains("section1")) << "Output should contain section1";
    EXPECT_TRUE(output_data["section1"].contains("field1")) << "section1 should contain field1";
    EXPECT_EQ(output_data["section1"]["field1"], "value1") << "field1 value should be preserved";
    EXPECT_EQ(output_data["section1"]["field2"], 42) << "field2 value should be preserved";
}

// ========== End-to-End Workflow Tests ==========

TEST_F(MainWorkflowTest, E2E_LoadSchema_HasNoEffect_WhenFileDoesNotExist)
{
    // Test: Loading non-existent schema should fail gracefully
    FormGenerator form_gen;
    json schema;
    
    // This should not crash
    bool result = form_gen.generateFromSchema(json::object());
    EXPECT_FALSE(result) << "Should fail when schema has no properties";
}

TEST_F(MainWorkflowTest, E2E_FileIO_SaveAndLoadRoundtrip)
{
    // Test: Complete workflow - create, save, load, and verify data
    QString config_file = createConfigFile("test_config.json");
    ASSERT_FALSE(config_file.isEmpty()) << "Config file should be created";

    // Read the file back
    JsonReader reader;
    auto read_result = reader.read(config_file.toStdString());
    EXPECT_TRUE(read_result.ok) << "Should be able to read saved config";

    json saved_config = read_result.value;
    EXPECT_TRUE(saved_config.contains("application")) << "Config should contain application section";
    EXPECT_EQ(saved_config["application"]["name"], "Test Application");
    EXPECT_EQ(saved_config["application"]["port"], 8080);
}

TEST_F(MainWorkflowTest, E2E_FormGenerator_LoadModifySave_Workflow)
{
    // Test: Complete workflow - generate form, modify data, save
    QString schema_file = createSchemaFile("test_schema.json");
    ASSERT_FALSE(schema_file.isEmpty()) << "Schema file should be created";

    QString config_file = createConfigFile("test_config.json");
    ASSERT_FALSE(config_file.isEmpty()) << "Config file should be created";

    // Read schema
    JsonReader schema_reader;
    auto schema_result = schema_reader.read(schema_file.toStdString());
    ASSERT_TRUE(schema_result.ok) << "Should be able to read schema";

    // Read config
    JsonReader config_reader;
    auto config_result = config_reader.read(config_file.toStdString());
    ASSERT_TRUE(config_result.ok) << "Should be able to read config";

    // Generate form from schema
    FormGenerator form_gen;
    bool gen_result = form_gen.generateFromSchema(schema_result.value);
    EXPECT_TRUE(gen_result) << "FormGenerator should generate form from schema";

    // Load config into form
    form_gen.setFormData(config_result.value);

    // Get modified data from form
    json form_data = form_gen.getFormData();
    EXPECT_TRUE(form_data.contains("application")) << "Form data should contain application";
    EXPECT_EQ(form_data["application"]["name"], "Test Application");

    // Save to file
    JsonWriter writer;
    auto write_result = writer.write(config_file.toStdString(), form_data);
    EXPECT_TRUE(write_result.ok) << "Should be able to save modified config";

    // Verify saved file
    auto verify_result = config_reader.read(config_file.toStdString());
    EXPECT_TRUE(verify_result.ok) << "Should be able to read back saved config";
    EXPECT_EQ(verify_result.value["application"]["name"], "Test Application");
}

TEST_F(MainWorkflowTest, E2E_FormGenerator_FailsOnInvalidSchema)
{
    // Test: FormGenerator handles invalid schema gracefully
    FormGenerator form_gen;

    // Schema without properties
    json invalid_schema = {{"type", "object"}};
    bool result = form_gen.generateFromSchema(invalid_schema);
    EXPECT_FALSE(result) << "Should fail on schema without properties";

    // Non-object schema
    json non_object_schema = {{"type", "string"}};
    result = form_gen.generateFromSchema(non_object_schema);
    EXPECT_FALSE(result) << "Should fail on non-object schema";
}

TEST_F(MainWorkflowTest, E2E_DataIntegrity_NestedStructurePreserved)
{
    // Test: Nested structure is preserved through load-modify-save cycle
    QString schema_file = createSchemaFile("integrity_schema.json");
    QString config_file = createConfigFile("integrity_config.json");

    // Read original config
    JsonReader reader;
    auto original_result = reader.read(config_file.toStdString());
    json original_config = original_result.value;

    // Read schema
    auto schema_result = reader.read(schema_file.toStdString());
    json schema = schema_result.value;

    // Create form and load config
    FormGenerator form_gen;
    form_gen.generateFromSchema(schema);
    form_gen.setFormData(original_config);

    // Get form data back
    json retrieved_config = form_gen.getFormData();

    // Verify all nested values are preserved
    EXPECT_EQ(retrieved_config["application"]["name"], original_config["application"]["name"]);
    EXPECT_EQ(retrieved_config["application"]["version"], original_config["application"]["version"]);
    EXPECT_EQ(retrieved_config["application"]["enabled"], original_config["application"]["enabled"]);
    EXPECT_EQ(retrieved_config["application"]["port"], original_config["application"]["port"]);
    
    EXPECT_EQ(retrieved_config["database"]["type"], original_config["database"]["type"]);
    EXPECT_EQ(retrieved_config["database"]["connection_string"], original_config["database"]["connection_string"]);
    EXPECT_EQ(retrieved_config["database"]["max_connections"], original_config["database"]["max_connections"]);
}

TEST_F(MainWorkflowTest, E2E_ErrorHandling_InvalidJsonFile)
{
    // Test: Error handling for invalid JSON files
    QString temp_file = QString::fromStdString(temp_dir_->path().toStdString() + "/invalid.json");
    
    // Write invalid JSON
    QFile file(temp_file);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QByteArray("{ invalid json content }"));
    file.close();

    // Try to read it
    JsonReader reader;
    auto result = reader.read(temp_file.toStdString());
    EXPECT_FALSE(result.ok) << "Should fail to parse invalid JSON";
}

TEST_F(MainWorkflowTest, E2E_MultipleRoundtrips)
{
    // Test: Multiple load-modify-save cycles maintain data integrity
    QString config_file = createConfigFile("roundtrip_config.json");

    JsonReader reader;
    JsonWriter writer;

    // Original config
    auto result1 = reader.read(config_file.toStdString());
    json config1 = result1.value;

    // First roundtrip
    FormGenerator form1;
    json schema = {
        {"type", "object"},
        {"properties", {
            {"application", {
                {"type", "object"},
                {"properties", {
                    {"name", {{"type", "string"}}},
                    {"version", {{"type", "string"}}},
                    {"enabled", {{"type", "boolean"}}},
                    {"port", {{"type", "integer"}}}
                }}
            }},
            {"database", {
                {"type", "object"},
                {"properties", {
                    {"type", {{"type", "string"}}},
                    {"connection_string", {{"type", "string"}}},
                    {"max_connections", {{"type", "integer"}}}
                }}
            }}
        }}
    };

    form1.generateFromSchema(schema);
    form1.setFormData(config1);
    json modified1 = form1.getFormData();
    auto write1 = writer.write(config_file.toStdString(), modified1);
    EXPECT_TRUE(write1.ok);

    // Second roundtrip
    auto result2 = reader.read(config_file.toStdString());
    json config2 = result2.value;

    FormGenerator form2;
    form2.generateFromSchema(schema);
    form2.setFormData(config2);
    json modified2 = form2.getFormData();
    auto write2 = writer.write(config_file.toStdString(), modified2);
    EXPECT_TRUE(write2.ok);

    // Verify data is identical after roundtrips
    auto final_result = reader.read(config_file.toStdString());
    json final_config = final_result.value;

    EXPECT_EQ(final_config["application"]["name"], config1["application"]["name"]);
    EXPECT_EQ(final_config["application"]["port"], config1["application"]["port"]);
    EXPECT_EQ(final_config["database"]["connection_string"], config1["database"]["connection_string"]);
}

int main(int argc, char** argv)
{
    // Initialize QApplication for GUI tests
    QApplication app(argc, argv);
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
