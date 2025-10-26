#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "../../../src/html/handlers/schema_service.h"

using json = nlohmann::json;

class SchemaServiceTest : public ::testing::Test {
protected:
    std::string testDir;
    std::string schemaDir;

    void SetUp() override {
        // Create temporary test directory
        testDir = "/tmp/configgui_schema_test_" + std::to_string(std::time(nullptr));
        std::filesystem::create_directories(testDir);
        schemaDir = testDir + "/schemas";
        std::filesystem::create_directories(schemaDir);
    }

    void TearDown() override {
        // Clean up test directory
        std::filesystem::remove_all(testDir);
    }

    /**
     * Create a test JSON schema file
     */
    void createJsonSchema(const std::string& filename, const json& content) {
        std::ofstream file{schemaDir + "/" + filename};
        file << content.dump(2);
    }

    /**
     * Create a test YAML schema file
     */
    void createYamlSchema(const std::string& filename, const std::string& content) {
        std::ofstream file{schemaDir + "/" + filename};
        file << content;
    }
};

// Test 1: Initialization
TEST_F(SchemaServiceTest, InitializeWithValidDirectory) {
    SchemaService service;
    EXPECT_FALSE(service.isInitialized());
    
    EXPECT_TRUE(service.initialize(schemaDir));
    EXPECT_TRUE(service.isInitialized());
    EXPECT_EQ(service.getSchemaDir(), std::filesystem::absolute(schemaDir).string());
}

TEST_F(SchemaServiceTest, InitializeWithInvalidDirectory) {
    SchemaService service;
    EXPECT_FALSE(service.initialize("/nonexistent/path/to/schemas"));
    EXPECT_FALSE(service.isInitialized());
}

// Test 2: Empty directory
TEST_F(SchemaServiceTest, ListSchemasEmptyDirectory) {
    SchemaService service;
    service.initialize(schemaDir);
    
    auto schemas = service.listSchemas();
    EXPECT_EQ(schemas.size(), 0);
    EXPECT_EQ(service.getSchemaCount(), 0);
}

// Test 3: List schemas
TEST_F(SchemaServiceTest, ListSchemasWithJsonFiles) {
    SchemaService service;
    service.initialize(schemaDir);
    
    // Create test schemas
    json schema1{
        {"$schema", "http://json-schema.org/draft-07/schema#"},
        {"type", "object"},
        {"title", "Schema One"},
        {"description", "First test schema"}
    };
    
    json schema2{
        {"$schema", "http://json-schema.org/draft-07/schema#"},
        {"type", "object"},
        {"title", "Schema Two"},
        {"description", "Second test schema"}
    };
    
    createJsonSchema("schema1.json", schema1);
    createJsonSchema("schema2.json", schema2);
    
    auto schemas = service.listSchemas();
    EXPECT_EQ(schemas.size(), 2);
    EXPECT_EQ(service.getSchemaCount(), 2);
    
    // Check ordering
    EXPECT_EQ(schemas[0].id, "schema1");
    EXPECT_EQ(schemas[0].name, "Schema One");
    EXPECT_EQ(schemas[0].description, "First test schema");
    EXPECT_EQ(schemas[0].fileFormat, "json");
    
    EXPECT_EQ(schemas[1].id, "schema2");
    EXPECT_EQ(schemas[1].name, "Schema Two");
    EXPECT_EQ(schemas[1].description, "Second test schema");
}

// Test 4: Get schema by ID
TEST_F(SchemaServiceTest, GetSchemaById) {
    SchemaService service;
    service.initialize(schemaDir);
    
    json schema{
        {"$schema", "http://json-schema.org/draft-07/schema#"},
        {"type", "object"},
        {"title", "Test Schema"},
        {"properties", {
            {"name", {{"type", "string"}}}
        }}
    };
    
    createJsonSchema("test.json", schema);
    
    auto loaded = service.getSchema("test");
    EXPECT_FALSE(SchemaService::isError(loaded));
    EXPECT_EQ(loaded["title"].get<std::string>(), "Test Schema");
    EXPECT_TRUE(loaded.contains("properties"));
}

// Test 5: Get schema not found
TEST_F(SchemaServiceTest, GetSchemaNonexistent) {
    SchemaService service;
    service.initialize(schemaDir);
    
    auto result = service.getSchema("nonexistent");
    EXPECT_TRUE(SchemaService::isError(result));
    EXPECT_EQ(result["error"].get<std::string>(), "Schema not found");
}

// Test 6: Invalid JSON handling
TEST_F(SchemaServiceTest, InvalidJsonFile) {
    SchemaService service;
    service.initialize(schemaDir);
    
    std::ofstream file{schemaDir + "/invalid.json"};
    file << "{ invalid json content }";
    file.close();
    
    auto result = service.getSchemaByFilename("invalid.json");
    EXPECT_TRUE(SchemaService::isError(result));
    EXPECT_EQ(result["error"].get<std::string>(), "Invalid JSON");
}

// Test 7: File not found by filename
TEST_F(SchemaServiceTest, GetSchemaByFilenameNotFound) {
    SchemaService service;
    service.initialize(schemaDir);
    
    auto result = service.getSchemaByFilename("nonexistent.json");
    EXPECT_TRUE(SchemaService::isError(result));
    EXPECT_EQ(result["error"].get<std::string>(), "File not found");
}

// Test 8: Directory traversal attack prevention
TEST_F(SchemaServiceTest, DirectoryTraversalPrevention) {
    SchemaService service;
    service.initialize(schemaDir);
    
    // Try to access parent directory
    auto result = service.getSchemaByFilename("../../../etc/passwd");
    EXPECT_TRUE(SchemaService::isError(result));
    EXPECT_EQ(result["error"].get<std::string>(), "Invalid filename");
}

// Test 9: Metadata to JSON conversion
TEST_F(SchemaServiceTest, SchemaMetadataToJson) {
    SchemaService::SchemaMetadata metadata;
    metadata.id = "test.schema";
    metadata.name = "Test Schema";
    metadata.description = "A test schema";
    metadata.fileFormat = "json";
    metadata.fileSize = 1024;
    
    auto json_obj = metadata.toJson();
    EXPECT_EQ(json_obj["id"].get<std::string>(), "test.schema");
    EXPECT_EQ(json_obj["name"].get<std::string>(), "Test Schema");
    EXPECT_EQ(json_obj["description"].get<std::string>(), "A test schema");
    EXPECT_EQ(json_obj["fileFormat"].get<std::string>(), "json");
    EXPECT_EQ(json_obj["fileSize"].get<std::uint64_t>(), 1024);
}

// Test 10: Schema name extraction from title
TEST_F(SchemaServiceTest, ExtractSchemaNameFromTitle) {
    SchemaService service;
    service.initialize(schemaDir);
    
    json schema{
        {"title", "My Configuration Schema"},
        {"type", "object"}
    };
    
    createJsonSchema("config.json", schema);
    auto schemas = service.listSchemas();
    EXPECT_EQ(schemas[0].name, "My Configuration Schema");
}

// Test 11: Schema name extraction fallback to ID
TEST_F(SchemaServiceTest, ExtractSchemaNameFallbackToId) {
    SchemaService service;
    service.initialize(schemaDir);
    
    json schema{
        {"type", "object"},
        {"properties", {}}
    };
    
    createJsonSchema("myschema.json", schema);
    auto schemas = service.listSchemas();
    EXPECT_EQ(schemas[0].name, "myschema");  // Falls back to ID when no title
}

// Test 12: Mixed file extensions
TEST_F(SchemaServiceTest, SupportedFileExtensions) {
    SchemaService service;
    service.initialize(schemaDir);
    
    json schema{
        {"type", "object"},
        {"title", "Test"}
    };
    
    createJsonSchema("schema1.json", schema);
    
    // Note: YAML support requires yaml-cpp to be properly integrated
    // For now, just test JSON files
    auto schemas = service.listSchemas();
    EXPECT_EQ(schemas.size(), 1);
    
    // Check formats
    auto json_schema = std::find_if(schemas.begin(), schemas.end(),
                                    [](const auto& s) { return s.id == "schema1"; });
    EXPECT_NE(json_schema, schemas.end());
    EXPECT_EQ(json_schema->fileFormat, "json");
}

// Test 13: Ignore non-schema files
TEST_F(SchemaServiceTest, IgnoreNonSchemaFiles) {
    SchemaService service;
    service.initialize(schemaDir);
    
    json schema{
        {"type", "object"},
        {"title", "Test"}
    };
    
    createJsonSchema("schema.json", schema);
    
    // Create non-schema files
    std::ofstream txt{schemaDir + "/readme.txt"};
    txt << "This is not a schema";
    txt.close();
    
    auto schemas = service.listSchemas();
    EXPECT_EQ(schemas.size(), 1);  // Only the .json schema should be counted
}

// Test 14: Error JSON helper
TEST_F(SchemaServiceTest, CreateErrorHelper) {
    auto error = SchemaService::createError("Test error", {{"code", 42}});
    
    EXPECT_TRUE(SchemaService::isError(error));
    EXPECT_EQ(error["error"].get<std::string>(), "Test error");
    EXPECT_EQ(error["details"]["code"].get<int>(), 42);
}

// Test 15: Uninitialized service handling
TEST_F(SchemaServiceTest, UninitializedServiceMethods) {
    SchemaService service;
    
    // Should return empty or error when not initialized
    auto schemas = service.listSchemas();
    EXPECT_EQ(schemas.size(), 0);
    
    auto result = service.getSchema("test");
    EXPECT_TRUE(SchemaService::isError(result));
    
    EXPECT_EQ(service.getSchemaCount(), 0);
    EXPECT_EQ(service.getSchemaDir(), "");
}
