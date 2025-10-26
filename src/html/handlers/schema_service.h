#ifndef CONFIGGUI_HTML_HANDLERS_SCHEMA_SERVICE_H
#define CONFIGGUI_HTML_HANDLERS_SCHEMA_SERVICE_H

#include <string>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief Schema Service - Discovers and loads JSON/YAML schemas from filesystem
 * 
 * Provides interface for scanning schema directories, listing available schemas,
 * and loading individual schema files. Used by HTML server to generate dynamic
 * schema listing endpoint.
 * 
 * @section Thread Safety
 * Not thread-safe. Should be protected by caller if accessed from multiple threads.
 * 
 * @section Error Handling
 * Methods return json objects with error information. Use isError() helper to check.
 * Error format: {"error": "message", "details": {...}}
 */
class SchemaService {
public:
    /**
     * @brief Schema metadata entry
     * 
     * Represents a discovered schema file with key information.
     */
    struct SchemaMetadata {
        std::string id;              ///< Schema identifier (basename without extension)
        std::string name;            ///< Human-readable name (from schema title or id)
        std::string description;     ///< Description (from schema description property)
        std::string filePath;        ///< Absolute path to schema file
        std::string fileFormat;      ///< Format (json, yaml)
        std::uint64_t fileSize;      ///< File size in bytes
        
        /**
         * Convert to JSON representation for API responses
         * @return JSON object with metadata
         */
        json toJson() const;
    };

    /**
     * @brief Initialize schema service with schema directory
     * 
     * @param schemaDir Path to directory containing JSON/YAML schema files
     * @return true if directory exists and is readable, false otherwise
     * 
     * @example
     * SchemaService service;
     * if (service.initialize("resources/schemas")) {
     *     auto schemas = service.listSchemas();
     * }
     */
    bool initialize(const std::string& schemaDir);

    /**
     * @brief List all discovered schemas in directory
     * 
     * Scans schema directory and returns metadata for all JSON/YAML files found.
     * Includes both .json and .yaml files (including .yml extension).
     * 
     * @return Vector of SchemaMetadata for all discovered schemas
     * 
     * @throws std::runtime_error if schema directory is not initialized
     * 
     * @example
     * auto schemas = service.listSchemas();
     * for (const auto& schema : schemas) {
     *     std::cout << schema.id << ": " << schema.name << std::endl;
     * }
     */
    std::vector<SchemaMetadata> listSchemas() const;

    /**
     * @brief Load a specific schema by ID
     * 
     * Loads and parses the schema file, returning parsed JSON.
     * ID is the filename without extension (e.g., "config.schema.json" → "config.schema").
     * 
     * @param schemaId Schema identifier (basename without extension)
     * @return JSON object if found and valid, error JSON if not found or invalid
     * 
     * @error {"error": "Schema not found", "id": "schemaId"}
     * @error {"error": "Invalid JSON/YAML", "id": "schemaId", "details": "parse error message"}
     * 
     * @example
     * auto schema = service.getSchema("config.schema");
     * if (!isError(schema)) {
     *     // Use schema
     * }
     */
    json getSchema(const std::string& schemaId) const;

    /**
     * @brief Load schema by full filename
     * 
     * Similar to getSchema but uses full filename instead of ID.
     * Useful for direct file access.
     * 
     * @param filename Schema filename (e.g., "config.schema.json")
     * @return JSON object if found and valid, error JSON if not found or invalid
     * 
     * @error {"error": "File not found", "filename": "filename"}
     * @error {"error": "Invalid JSON/YAML", "filename": "filename", "details": "error message"}
     */
    json getSchemaByFilename(const std::string& filename) const;

    /**
     * @brief Check if schema directory is initialized
     * 
     * @return true if initialize() was called with a valid directory, false otherwise
     */
    bool isInitialized() const;

    /**
     * @brief Get current schema directory path
     * 
     * @return Absolute path to schema directory, empty string if not initialized
     */
    std::string getSchemaDir() const;

    /**
     * @brief Get number of discovered schemas
     * 
     * Performs directory scan to count schemas.
     * 
     * @return Number of schema files found
     */
    std::size_t getSchemaCount() const;

    /**
     * @brief Check if JSON object represents an error
     * 
     * Helper function to determine if a returned JSON contains error information.
     * Useful for checking results from getSchema() and similar methods.
     * 
     * @param json JSON object to check
     * @return true if contains "error" field, false otherwise
     * 
     * @example
     * auto result = service.getSchema("test");
     * if (SchemaService::isError(result)) {
     *     auto error = result["error"].get<std::string>();
     *     // Handle error
     * }
     */
    static bool isError(const json& obj);

    /**
     * @brief Create error JSON response
     * 
     * Utility for creating standardized error JSON responses.
     * 
     * @param errorMsg Error message
     * @param details Optional additional details object
     * @return Error JSON: {"error": errorMsg, "details": details}
     */
    static json createError(const std::string& errorMsg, const json& details = {});

private:
    std::string schemaDir_;  ///< Schema directory path (absolute)
    bool initialized_;       ///< Whether initialize() was called successfully

    /**
     * @brief Parse JSON file
     * 
     * Reads and parses a JSON file with error handling.
     * 
     * @param filePath Absolute path to JSON file
     * @return Parsed JSON on success, error JSON on failure
     */
    json parseJsonFile(const std::string& filePath) const;

    /**
     * @brief Parse YAML file
     * 
     * Reads and parses a YAML file with error handling.
     * 
     * @param filePath Absolute path to YAML file
     * @return Parsed JSON (converted from YAML) on success, error JSON on failure
     */
    json parseYamlFile(const std::string& filePath) const;

    /**
     * @brief Extract metadata from schema JSON
     * 
     * Gets title, description, and other metadata from schema object.
     * Returns sensible defaults if fields are missing.
     * 
     * @param schemaJson Parsed schema JSON
     * @param schemaId Schema identifier for defaults
     * @return Metadata object extracted from schema
     */
    std::string extractSchemaName(const json& schemaJson, const std::string& schemaId) const;
    std::string extractSchemaDescription(const json& schemaJson) const;

    /**
     * @brief Check if file has supported schema extension
     * 
     * Supported: .json, .yaml, .yml
     * 
     * @param filename Filename to check
     * @return true if extension is supported
     */
    static bool isSupportedSchemaFile(const std::string& filename);
};

#endif // CONFIGGUI_HTML_HANDLERS_SCHEMA_SERVICE_H
