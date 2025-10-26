#include "schema_service.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <yaml-cpp/yaml.h>

// Initialize static members
bool SchemaService::initialize(const std::string& schemaDir) {
    std::filesystem::path path{schemaDir};
    
    // Check if directory exists and is readable
    std::error_code ec;
    if (!std::filesystem::is_directory(path, ec)) {
        initialized_ = false;
        return false;
    }
    
    // Convert to absolute path
    schemaDir_ = std::filesystem::absolute(path, ec).string();
    initialized_ = true;
    return true;
}

std::vector<SchemaService::SchemaMetadata> SchemaService::listSchemas() const {
    std::vector<SchemaMetadata> schemas;
    
    if (!initialized_) {
        return schemas;
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator{schemaDir_}) {
            if (entry.is_regular_file()) {
                const auto& path = entry.path();
                const auto filename = path.filename().string();
                
                if (isSupportedSchemaFile(filename)) {
                    // Extract ID (filename without extension)
                    auto id = path.stem().string();
                    
                    // Load schema to extract metadata
                    auto schema = getSchemaByFilename(filename);
                    if (isError(schema)) {
                        continue;  // Skip files that fail to parse
                    }
                    
                    SchemaMetadata metadata;
                    metadata.id = id;
                    metadata.name = extractSchemaName(schema, id);
                    metadata.description = extractSchemaDescription(schema);
                    metadata.filePath = path.string();
                    metadata.fileFormat = (path.extension() == ".json") ? "json" : "yaml";
                    metadata.fileSize = std::filesystem::file_size(path);
                    
                    schemas.push_back(metadata);
                }
            }
        }
        
        // Sort by ID for consistent ordering
        std::sort(schemas.begin(), schemas.end(),
                  [](const SchemaMetadata& a, const SchemaMetadata& b) {
                      return a.id < b.id;
                  });
    }
    catch (const std::exception& /*e*/) {
        // Return whatever we found so far
    }
    
    return schemas;
}

json SchemaService::getSchema(const std::string& schemaId) const {
    if (!initialized_) {
        return createError("Schema service not initialized");
    }
    
    // Try with .json extension first
    auto jsonPath = std::filesystem::path{schemaDir_} / (schemaId + ".json");
    if (std::filesystem::exists(jsonPath)) {
        return getSchemaByFilename(schemaId + ".json");
    }
    
    // Try with .yaml extension
    auto yamlPath = std::filesystem::path{schemaDir_} / (schemaId + ".yaml");
    if (std::filesystem::exists(yamlPath)) {
        return getSchemaByFilename(schemaId + ".yaml");
    }
    
    // Try with .yml extension
    auto ymlPath = std::filesystem::path{schemaDir_} / (schemaId + ".yml");
    if (std::filesystem::exists(ymlPath)) {
        return getSchemaByFilename(schemaId + ".yml");
    }
    
    return createError("Schema not found", {{"id", schemaId}});
}

json SchemaService::getSchemaByFilename(const std::string& filename) const {
    if (!initialized_) {
        return createError("Schema service not initialized");
    }
    
    // Prevent directory traversal attacks
    if (filename.find("..") != std::string::npos || 
        filename.find("/") != std::string::npos ||
        filename.find("\\") != std::string::npos) {
        return createError("Invalid filename", {{"filename", filename}});
    }
    
    auto fullPath = std::filesystem::path{schemaDir_} / filename;
    
    if (!std::filesystem::exists(fullPath)) {
        return createError("File not found", {{"filename", filename}});
    }
    
    // Check extension and parse accordingly
    auto extension = fullPath.extension().string();
    
    // Convert extension to lowercase for comparison
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    if (extension == ".json") {
        return parseJsonFile(fullPath.string());
    }
    else if (extension == ".yaml" || extension == ".yml") {
        return parseYamlFile(fullPath.string());
    }
    
    return createError("Unsupported file format", {{"filename", filename}, {"extension", extension}});
}

bool SchemaService::isInitialized() const {
    return initialized_;
}

std::string SchemaService::getSchemaDir() const {
    return schemaDir_;
}

std::size_t SchemaService::getSchemaCount() const {
    return listSchemas().size();
}

bool SchemaService::isError(const json& obj) {
    return obj.contains("error") && obj["error"].is_string();
}

json SchemaService::createError(const std::string& errorMsg, const json& details) {
    json error = {
        {"error", errorMsg}
    };
    
    if (!details.empty()) {
        error["details"] = details;
    }
    
    return error;
}

json SchemaService::parseJsonFile(const std::string& filePath) const {
    try {
        std::ifstream file{filePath};
        if (!file.is_open()) {
            return createError("Cannot open file", {{"path", filePath}});
        }
        
        json parsed;
        file >> parsed;
        return parsed;
    }
    catch (const json::parse_error& e) {
        return createError("Invalid JSON", 
                         {{"path", filePath}, {"error", e.what()}});
    }
    catch (const std::exception& e) {
        return createError("Error parsing JSON",
                         {{"path", filePath}, {"error", e.what()}});
    }
}

json SchemaService::parseYamlFile(const std::string& filePath) const {
    try {
        YAML::Node yamlNode = YAML::LoadFile(filePath);
        
        // Convert YAML to JSON
        std::string yamlStr = YAML::Dump(yamlNode);
        auto jsonObj = json::parse(yamlStr);
        
        return jsonObj;
    }
    catch (const YAML::Exception& e) {
        return createError("Invalid YAML",
                         {{"path", filePath}, {"error", e.what()}});
    }
    catch (const json::parse_error& e) {
        return createError("Error converting YAML to JSON",
                         {{"path", filePath}, {"error", e.what()}});
    }
    catch (const std::exception& e) {
        return createError("Error parsing YAML",
                         {{"path", filePath}, {"error", e.what()}});
    }
}

std::string SchemaService::extractSchemaName(const json& schemaJson, 
                                            const std::string& schemaId) const {
    if (schemaJson.contains("title") && schemaJson["title"].is_string()) {
        return schemaJson["title"].get<std::string>();
    }
    
    if (schemaJson.contains("$id") && schemaJson["$id"].is_string()) {
        auto idStr = schemaJson["$id"].get<std::string>();
        // Extract last component if it's a URI
        auto pos = idStr.find_last_of("/:");
        if (pos != std::string::npos) {
            return idStr.substr(pos + 1);
        }
        return idStr;
    }
    
    // Fallback to schema ID
    return schemaId;
}

std::string SchemaService::extractSchemaDescription(const json& schemaJson) const {
    if (schemaJson.contains("description") && schemaJson["description"].is_string()) {
        return schemaJson["description"].get<std::string>();
    }
    
    return "";
}

bool SchemaService::isSupportedSchemaFile(const std::string& filename) {
    std::string ext = filename;
    
    // Get extension
    auto pos = ext.find_last_of(".");
    if (pos == std::string::npos) {
        return false;
    }
    
    ext = ext.substr(pos);
    
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    return ext == ".json" || ext == ".yaml" || ext == ".yml";
}

json SchemaService::SchemaMetadata::toJson() const {
    return json{
        {"id", id},
        {"name", name},
        {"description", description},
        {"fileFormat", fileFormat},
        {"fileSize", fileSize}
    };
}
