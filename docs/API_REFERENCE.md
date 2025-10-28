# ConfigGUI API Reference

**Version**: 2.0  
**Date**: October 28, 2025  
**Audience**: C++ developers, API users

---

## Table of Contents

1. [Core Types](#core-types)
2. [Schema API](#schema-api)
3. [I/O API](#io-api)
4. [Validation API](#validation-api)
5. [Configuration Data API](#configuration-data-api)
6. [Error Handling](#error-handling)
7. [HTTP Server API](#http-server-api)

---

## Core Types

### Result<T, E> - Error Handling Type

```cpp
namespace configgui::core {

template<typename T, typename E = std::string>
class Result {
public:
    // Constructors
    Result(const T& value);
    Result(const E& error);
    static Result success(const T& value);
    static Result failure(const E& error);
    
    // Query methods
    bool is_success() const;
    bool is_failure() const;
    
    // Access methods
    T value() const;  // throws if failure
    E error() const;  // throws if success
    
    // Fluent interface
    template<typename F>
    Result<typename F::return_type> and_then(F&& f) const;
};

// Common type aliases
using JsonResult = Result<json>;
using StringResult = Result<std::string>;
using VoidResult = Result<void>;

}
```

**Usage Example**:
```cpp
auto result = INIReader::readFile("config.ini");
if (result.is_success()) {
    json config = result.value();
    std::cout << config.dump(2) << std::endl;
} else {
    std::cerr << "Error: " << result.error() << std::endl;
}
```

### ValidationError

```cpp
struct ValidationError {
    std::string message;
    std::string field;
    int line;
    int column;
    
    std::string to_string() const;
};
```

---

## Schema API

### Schema Class

```cpp
namespace configgui::core {

class Schema {
public:
    // Getters
    const std::string& id() const;
    const std::string& name() const;
    const std::string& description() const;
    const json& full_schema() const;
    
    // Schema operations
    Result<bool> validate(const json& data) const;
    const std::vector<ValidationError>& last_errors() const;
    
    // Property access
    Result<json> get_property(const std::string& name) const;
    bool has_property(const std::string& name) const;
    std::vector<std::string> required_properties() const;
};

}
```

**Usage Example**:
```cpp
using namespace configgui::core;

// Create schema
Schema schema("db-config", "Database Configuration");

// Validate configuration
auto result = schema.validate(config_json);
if (result.is_failure()) {
    auto errors = schema.last_errors();
    for (const auto& err : errors) {
        std::cout << "Error at " << err.field 
                  << ": " << err.message << std::endl;
    }
}
```

### SchemaLoader Class

```cpp
namespace configgui::core {

class SchemaLoader {
public:
    // Load from file
    static Result<Schema> load_from_file(
        const std::string& file_path
    );
    
    // Load from string
    static Result<Schema> load_from_string(
        const std::string& json_content
    );
    
    // Validate schema
    static Result<bool> validate_schema(
        const json& schema_data
    );
};

}
```

**Usage Example**:
```cpp
using namespace configgui::core;

// Load schema from file
auto schema_result = SchemaLoader::load_from_file(
    "config.schema.json"
);

if (schema_result.is_success()) {
    Schema schema = schema_result.value();
    std::cout << "Loaded schema: " << schema.name() << std::endl;
} else {
    std::cerr << "Failed: " << schema_result.error() << std::endl;
}
```

---

## I/O API

### Reader Classes

All reader classes follow the same interface pattern:

```cpp
namespace configgui::io {

// JSON Reader
class JsonReader {
public:
    static Result<json> read_file(const std::string& file_path);
    static Result<json> read_string(const std::string& json_string);
};

// YAML Reader
class YamlReader {
public:
    static Result<json> read_file(const std::string& file_path);
    static Result<json> read_string(const std::string& yaml_string);
};

// INI Reader
class INIReader {
public:
    static Result<json> read_file(const std::string& file_path);
    static Result<json> read_string(const std::string& ini_string);
};

}
```

**Usage Example**:
```cpp
using namespace configgui::io;

// Read JSON
auto json_result = JsonReader::read_file("config.json");

// Read YAML
auto yaml_result = YamlReader::read_file("config.yaml");

// Read INI
auto ini_result = INIReader::read_file("config.ini");

// Handle result
if (json_result.is_success()) {
    json data = json_result.value();
    // Use data...
}
```

### INI Parser (Low-Level)

```cpp
namespace configgui::io {

class INIParser {
public:
    // Parse INI content
    static Result<json> parse(const std::string& ini_content);
    
    // Parse INI file
    static Result<json> parse_file(const std::string& file_path);

private:
    struct ParseContext {
        std::string current_section;
        json result;
        int line_number;
    };
    
    // Helper methods
    static bool is_section_header(const std::string& line);
    static std::string extract_section_name(const std::string& line);
    static bool is_key_value_pair(const std::string& line);
    static std::pair<std::string, std::string> parse_key_value(
        const std::string& line
    );
    static json parse_value(const std::string& value);
};

}
```

**Example INI Content**:
```ini
[Database]
host=localhost
port=5432
enabled=true

[Database.Pool]
min_size=5
max_size=20
```

### Writer Classes

```cpp
namespace configgui::io {

// JSON Writer
class JsonWriter {
public:
    static Result<std::string> to_string(const json& data);
    static Result<void> write_file(
        const std::string& file_path,
        const json& data
    );
};

// YAML Writer
class YamlWriter {
public:
    static Result<std::string> to_string(const json& data);
    static Result<void> write_file(
        const std::string& file_path,
        const json& data
    );
};

// INI Writer
class INIWriter {
public:
    static Result<std::string> to_string(const json& data);
    static Result<void> write_file(
        const std::string& file_path,
        const json& data
    );
};

}
```

**Usage Example**:
```cpp
using namespace configgui::io;

json config = {
    {"database", {
        {"host", "localhost"},
        {"port", 5432}
    }}
};

// Convert to INI string
auto ini_result = INIWriter::to_string(config);
if (ini_result.is_success()) {
    std::cout << ini_result.value() << std::endl;
}

// Write INI file
auto write_result = INIWriter::write_file("config.ini", config);
if (write_result.is_failure()) {
    std::cerr << "Write failed: " << write_result.error() << std::endl;
}
```

---

## Validation API

### SchemaValidator Class

```cpp
namespace configgui::core {

class SchemaValidator {
public:
    // Constructor
    explicit SchemaValidator(const Schema& schema);
    
    // Validate data against schema
    Result<bool> validate(const json& data);
    
    // Get validation errors
    const std::vector<ValidationError>& get_errors() const;
    
    // Clear previous validation state
    void clear_errors();
    
    // Check specific field
    bool is_field_valid(const std::string& field_name) const;
    
    // Get errors for specific field
    std::vector<std::string> get_field_errors(
        const std::string& field_name
    ) const;
};

}
```

**Usage Example**:
```cpp
using namespace configgui::core;

// Load schema
auto schema_result = SchemaLoader::load_from_file(
    "config.schema.json"
);
if (schema_result.is_failure()) return;

Schema schema = schema_result.value();

// Create validator
SchemaValidator validator(schema);

// Validate configuration
auto result = validator.validate(config_json);
if (result.is_failure()) {
    auto errors = validator.get_errors();
    for (const auto& err : errors) {
        std::cout << "Field: " << err.field << std::endl;
        std::cout << "Error: " << err.message << std::endl;
    }
}
```

---

## Configuration Data API

### ConfigurationData Class

```cpp
namespace configgui::core {

class ConfigurationData {
public:
    // Constructor
    ConfigurationData(
        const std::string& schema_id,
        const json& initial_data = json::object()
    );
    
    // Data access
    json get_value(const std::string& key) const;
    json get_nested_value(const std::string& path) const;
    
    // Data modification
    void set_value(const std::string& key, const json& value);
    void set_nested_value(const std::string& path, const json& value);
    
    // Schema reference
    const std::string& schema_id() const;
    
    // Export
    json to_json() const;
    std::string to_string(int indent = 2) const;
    
    // State management
    FormState& form_state();
    const FormState& form_state() const;
};

}
```

**Usage Example**:
```cpp
using namespace configgui::core;

// Create configuration data
ConfigurationData config("db-schema");

// Set values
config.set_value("host", "localhost");
config.set_value("port", 5432);

// Set nested values
config.set_nested_value("credentials.username", "admin");

// Retrieve values
json host = config.get_value("host");
json port = config.get_nested_value("database.port");

// Export
json exported = config.to_json();
std::cout << config.to_string(2) << std::endl;
```

### FormState Class

```cpp
namespace configgui::core {

class FormState {
public:
    // Dirty state tracking
    bool is_dirty() const;
    void mark_dirty();
    void mark_clean();
    
    // Error management
    void add_error(
        const std::string& field,
        const ValidationError& error
    );
    void clear_errors();
    void clear_field_errors(const std::string& field);
    
    // Validation status
    bool is_valid() const;
    bool is_field_valid(const std::string& field) const;
    
    // Error retrieval
    const std::vector<ValidationError>& get_errors() const;
    std::vector<ValidationError> get_field_errors(
        const std::string& field
    ) const;
};

}
```

---

## Error Handling

### Error Types

```cpp
namespace configgui::core {

// Predefined error messages
class ErrorType {
public:
    static const std::string INVALID_SCHEMA;
    static const std::string PARSE_ERROR;
    static const std::string VALIDATION_ERROR;
    static const std::string IO_ERROR;
    static const std::string TYPE_ERROR;
};

}
```

### Exception-Free Error Handling

ConfigGUI uses `Result<T>` pattern instead of exceptions:

```cpp
// DON'T use try-catch
// try {
//     json data = json::parse(content);
// } catch (const std::exception& e) {
//     // ...
// }

// DO use Result<T>
auto result = JsonReader::read_string(content);
if (result.is_success()) {
    json data = result.value();
    // Process data
} else {
    std::string error = result.error();
    // Handle error
}
```

---

## HTTP Server API

### REST Endpoints

#### Schema Management

**GET /api/schemas**
```
Description: List all available schemas
Response: 200 OK
Body: 
{
    "schemas": [
        {
            "id": "db-config",
            "name": "Database Configuration",
            "description": "...",
            "fileFormat": "json"
        }
    ]
}
```

**POST /api/schemas**
```
Description: Upload new schema
Content-Type: application/json or multipart/form-data
Body: Schema JSON content
Response: 201 Created
```

**GET /api/schemas/{schemaId}**
```
Description: Retrieve specific schema
Response: 200 OK
Body: Full schema JSON
```

#### Configuration Management

**GET /api/configs**
```
Description: List all saved configurations
Response: 200 OK
Body:
{
    "configs": [
        {
            "filename": "production.ini",
            "schema": "db-config",
            "timestamp": "2025-10-28T12:34:56Z"
        }
    ]
}
```

**POST /api/config**
```
Description: Save configuration to server
Content-Type: application/json
Body:
{
    "filename": "app.ini",
    "format": "ini",
    "data": {...},
    "overwrite": false
}
Response: 201 Created
Body: 
{
    "status": "success",
    "filename": "app.ini",
    "path": "/configs/app.ini"
}
```

**GET /api/config/{filename}**
```
Description: Download saved configuration
Response: 200 OK
Content-Type: application/octet-stream
Body: File contents
```

**DELETE /api/config/{filename}**
```
Description: Delete saved configuration
Response: 204 No Content
```

**POST /api/config/{filename}?action=validate**
```
Description: Validate configuration against schema
Response: 200 OK
Body:
{
    "valid": true,
    "errors": []
}
```

### JavaScript Client API

#### Form Generation
```javascript
// Generate form from schema
const form = FormGenerator.renderForm(schema, container);

// Collect form data
const data = FormGenerator.collectFormData();
```

#### File Handling
```javascript
// Upload schema
const schema = await FileHandler.handleFileUpload(file);

// Upload configuration
const config = await FileHandler.handleConfigUpload(file);

// Detect file format
const format = FileHandler.detectFormat(filename);
```

#### Save Operations
```javascript
// Save configuration (format selection)
await SaveManager.saveConfiguration(formData, format);

// Save to local PC
await SaveManager.saveToLocalPC(data, filename, format);

// Save to server
await SaveManager.saveToServerBackup(data, filename, format);
```

#### Server Communication
```javascript
// Load schemas
const schemas = await HttpClient.loadSchemas();

// Load configurations
const configs = await HttpClient.loadConfigs();

// Save configuration
await HttpClient.saveConfig(data, filename);

// Delete configuration
await HttpClient.deleteConfig(filename);
```

---

## Example: Complete Workflow

```cpp
#include "core/schema_loader.h"
#include "core/schema_validator.h"
#include "core/configuration_data.h"
#include "io/ini_reader.h"
#include "io/ini_writer.h"

using namespace configgui::core;
using namespace configgui::io;

int main() {
    // 1. Load schema
    auto schema_result = SchemaLoader::load_from_file(
        "config.schema.json"
    );
    if (schema_result.is_failure()) {
        std::cerr << schema_result.error() << std::endl;
        return 1;
    }
    Schema schema = schema_result.value();
    
    // 2. Load INI configuration
    auto config_result = INIReader::read_file("config.ini");
    if (config_result.is_failure()) {
        std::cerr << config_result.error() << std::endl;
        return 1;
    }
    json config_json = config_result.value();
    
    // 3. Create configuration data
    ConfigurationData config("db-schema", config_json);
    
    // 4. Validate
    SchemaValidator validator(schema);
    auto valid_result = validator.validate(config.to_json());
    if (valid_result.is_failure()) {
        auto errors = validator.get_errors();
        for (const auto& err : errors) {
            std::cout << "Validation error in " << err.field 
                      << ": " << err.message << std::endl;
        }
        return 1;
    }
    
    // 5. Modify configuration
    config.set_value("port", 5433);
    config.set_nested_value("credentials.username", "newuser");
    
    // 6. Validate again
    valid_result = validator.validate(config.to_json());
    if (valid_result.is_failure()) {
        std::cerr << "Validation failed after changes" << std::endl;
        return 1;
    }
    
    // 7. Save in different format
    auto save_result = INIWriter::write_file(
        "config_updated.ini",
        config.to_json()
    );
    if (save_result.is_failure()) {
        std::cerr << save_result.error() << std::endl;
        return 1;
    }
    
    std::cout << "Configuration saved successfully" << std::endl;
    return 0;
}
```

---

## C++ Compiler Support

```cpp
// All code is C++17 compatible
// Features used:
// - std::optional<T>
// - std::string_view (where applicable)
// - Structured bindings
// - Fold expressions
// - Concepts (limited use)

// Compile with:
// g++ -std=c++17 ...
// clang++ -std=c++17 ...
// cl /std:c++17 ...
```

---

## Thread Safety

- ✅ All `Result<T>` operations are thread-safe
- ✅ Reader/Writer static methods are thread-safe
- ⚠️ Schema and ConfigurationData are NOT thread-safe (single thread)
- ⚠️ FormState modifications require external synchronization
- 💡 Recommended: Use thread-local copies or mutex protection

```cpp
// Example: Thread-safe usage
std::mutex schema_mutex;
Schema schema;  // shared across threads

std::lock_guard<std::mutex> lock(schema_mutex);
auto result = schema.validate(data);  // Safe now
```

