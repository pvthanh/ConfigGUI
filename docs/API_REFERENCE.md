# ConfigGUI API Reference

**Version**: 1.0.0  
**Date**: October 20, 2025  
**Language**: C++17  
**Namespace**: `configgui`

## Table of Contents

1. [Core Data Model API](#core-data-model-api)
2. [Validator Framework API](#validator-framework-api)
3. [File I/O API](#file-io-api)
4. [UI Components API](#ui-components-api)
5. [Utility Functions API](#utility-functions-api)
6. [Error Handling API](#error-handling-api)

---

## Core Data Model API

### `class JSONSchema`

Represents a JSON Schema (Draft 7) for configuration validation.

#### Header
```cpp
#include "core/schema.h"
```

#### Constructor
```cpp
JSONSchema(const json& schema);
```
**Parameters**:
- `schema`: nlohmann/json object representing the schema

**Throws**: std::invalid_argument if schema is invalid

#### Public Methods

##### `const json& raw_schema() const`
Returns the raw schema JSON object.

**Returns**: Reference to internal schema JSON

**Example**:
```cpp
JSONSchema schema(json::parse("{\"type\": \"object\"}"));
const auto& raw = schema.raw_schema();
```

---

##### `QString title() const`
Returns the schema title.

**Returns**: Title string (empty if not specified)

**Example**:
```cpp
auto title = schema.title();
```

---

##### `QString description() const`
Returns the schema description.

**Returns**: Description string (empty if not specified)

**Example**:
```cpp
auto desc = schema.description();
```

---

##### `std::vector<QString> required_fields() const`
Returns list of required field names.

**Returns**: Vector of required field names

**Example**:
```cpp
auto required = schema.required_fields();
for (const auto& field : required) {
    qDebug() << "Required: " << field;
}
```

---

##### `const json& properties() const`
Returns schema properties object.

**Returns**: JSON object containing all property definitions

**Example**:
```cpp
const auto& props = schema.properties();
for (auto it = props.begin(); it != props.end(); ++it) {
    std::cout << "Property: " << it.key() << std::endl;
}
```

---

##### `bool hasProperty(const std::string& name) const` ⚡ **OPTIMIZED**
**O(1) property lookup** using indexed fields.

**Parameters**:
- `name`: Property name to check

**Returns**: true if property exists, false otherwise

**Complexity**: O(1) average case (vs O(n) linear search)

**Example**:
```cpp
if (schema.hasProperty("username")) {
    // Field exists
}
```

---

##### `const json* getProperty(const std::string& name) const` ⚡ **OPTIMIZED**
**O(1) property retrieval** using indexed fields.

**Parameters**:
- `name`: Property name to retrieve

**Returns**: Pointer to property JSON object, nullptr if not found

**Complexity**: O(1) average case

**Example**:
```cpp
if (auto prop = schema.getProperty("username")) {
    std::cout << "Type: " << prop->at("type") << std::endl;
} else {
    std::cerr << "Property not found" << std::endl;
}
```

---

### `class ConfigurationData`

Stores configuration values with type-safe access.

#### Header
```cpp
#include "core/configuration_data.h"
```

#### Constructor
```cpp
ConfigurationData();
ConfigurationData(const json& data);
```

#### Public Methods

##### `Result<json> get(const std::string& path) const`
Retrieve configuration value by path.

**Parameters**:
- `path`: JSON path to value (e.g., "database.host")

**Returns**: `Result<json>` containing value or error

**Example**:
```cpp
auto result = config.get("database.host");
if (result.is_success()) {
    auto value = result.value();
    std::cout << "Host: " << value << std::endl;
} else {
    auto error = result.error();
    std::cerr << "Error: " << error.message() << std::endl;
}
```

---

##### `Result<void> set(const std::string& path, const json& value)`
Set configuration value by path.

**Parameters**:
- `path`: JSON path to set
- `value`: Value to set

**Returns**: `Result<void>` indicating success or failure

**Example**:
```cpp
auto result = config.set("database.port", 5432);
if (!result.is_success()) {
    std::cerr << "Failed to set port: " << result.error().message() << std::endl;
}
```

---

##### `const json& raw() const`
Get raw configuration JSON.

**Returns**: Reference to internal JSON object

**Example**:
```cpp
const auto& raw_config = config.raw();
```

---

##### `std::string toJson() const`
Export configuration as JSON string.

**Returns**: JSON string representation

**Example**:
```cpp
std::string json_str = config.toJson();
std::cout << json_str << std::endl;
```

---

##### `std::string toYaml() const`
Export configuration as YAML string.

**Returns**: YAML string representation

**Example**:
```cpp
std::string yaml_str = config.toYaml();
std::cout << yaml_str << std::endl;
```

---

### `class FormState`

Tracks form widget states and changes.

#### Header
```cpp
#include "core/form_state.h"
```

#### Constructor
```cpp
FormState();
```

#### Public Methods

##### `void setValue(const std::string& path, const std::any& value)`
Set widget state value.

**Parameters**:
- `path`: Field path
- `value`: Value to store

**Example**:
```cpp
form_state.setValue("username", std::any(std::string("admin")));
form_state.setValue("port", std::any(5432));
```

---

##### `std::any getValue(const std::string& path) const`
Get widget state value.

**Parameters**:
- `path`: Field path

**Returns**: Stored value, or empty std::any if not found

**Example**:
```cpp
auto value = form_state.getValue("username");
if (value.has_value()) {
    auto str = std::any_cast<std::string>(value);
    std::cout << "Username: " << str << std::endl;
}
```

---

##### `void setValidationState(const std::string& path, bool valid)`
Set validation state for field.

**Parameters**:
- `path`: Field path
- `valid`: Whether field is valid

**Example**:
```cpp
form_state.setValidationState("email", true);  // Valid
form_state.setValidationState("port", false);  // Invalid
```

---

##### `bool getValidationState(const std::string& path) const`
Get validation state for field.

**Parameters**:
- `path`: Field path

**Returns**: true if valid, false if invalid or not checked

**Example**:
```cpp
if (form_state.getValidationState("email")) {
    std::cout << "Email is valid" << std::endl;
}
```

---

##### `bool hasChanges() const`
Check if form has unsaved changes.

**Returns**: true if any changes detected

**Example**:
```cpp
if (form_state.hasChanges()) {
    // Warn user about unsaved changes
}
```

---

##### `std::map<std::string, std::any> getChanges() const`
Get all changed fields.

**Returns**: Map of changed field paths to values

**Example**:
```cpp
auto changes = form_state.getChanges();
for (const auto& [path, value] : changes) {
    std::cout << "Changed: " << path << std::endl;
}
```

---

##### `void reset()`
Reset form to initial state.

**Example**:
```cpp
form_state.reset();  // Clear all changes
```

---

### `class ValidationError`

Represents a validation error with detailed information.

#### Header
```cpp
#include "core/validation_error.h"
```

#### Constructors
```cpp
ValidationError(const std::string& path,
                const std::string& message,
                const std::string& code = "");
```

#### Public Methods

##### `const std::string& path() const`
Get field path that failed validation.

**Returns**: Field path

**Example**:
```cpp
std::cout << "Error in: " << error.path() << std::endl;
```

---

##### `const std::string& message() const`
Get error message.

**Returns**: Human-readable error message

**Example**:
```cpp
std::cout << "Problem: " << error.message() << std::endl;
```

---

##### `const std::string& code() const`
Get error code.

**Returns**: Error code (e.g., "INVALID_TYPE", "OUT_OF_RANGE")

**Example**:
```cpp
if (error.code() == "INVALID_TYPE") {
    // Handle type error
}
```

---

##### `std::string toString() const`
Format error as string.

**Returns**: Formatted error string

**Example**:
```cpp
std::cerr << error.toString() << std::endl;
// Output: "username: Invalid email format [INVALID_FORMAT]"
```

---

### `template <typename T, typename E> class Result<T, E>`

Type-safe error handling without exceptions. ⚡ **OPTIMIZED**

#### Header
```cpp
#include "core/result.h"
```

#### Constructors
```cpp
Result(const T& value);  // Success
Result(const E& error);  // Failure
```

#### Static Factory Methods

##### `static Result<T, E> Ok(const T& value)`
Create success result.

**Example**:
```cpp
auto result = Result<json>::Ok(json::parse("{\"status\": \"ok\"}"));
```

---

##### `static Result<T, E> Error(const E& error)`
Create error result.

**Example**:
```cpp
auto result = Result<json>::Error(ValidationError("field", "Invalid"));
```

---

#### Public Methods

##### `bool is_success() const`
Check if result represents success.

**Returns**: true if successful

**Example**:
```cpp
if (result.is_success()) {
    // Success path
}
```

---

##### `bool is_failure() const`
Check if result represents failure.

**Returns**: true if failed

**Example**:
```cpp
if (result.is_failure()) {
    // Error path
}
```

---

##### `T& value() &`
Access value (lvalue reference).

**Returns**: Reference to value

**Throws**: std::bad_variant_access if result is failure

**Example**:
```cpp
auto& config = result.value();  // lvalue context
```

---

##### `const T& value() const&`
Access value (const lvalue reference).

**Example**:
```cpp
const auto& config = result.value();  // const context
```

---

##### `T&& value() &&` ⚡ **OPTIMIZED**
Access value (rvalue reference). Enables **move semantics**.

**Returns**: Rvalue reference to value

**Example**:
```cpp
auto config = std::move(result).value();  // Efficient move
```

---

##### `E& error() &`
Access error (lvalue reference).

**Example**:
```cpp
auto& err = result.error();
```

---

##### `const E& error() const&`
Access error (const lvalue reference).

**Example**:
```cpp
const auto& err = result.error();
```

---

##### `E&& error() &&` ⚡ **OPTIMIZED**
Access error (rvalue reference). Enables **move semantics**.

**Example**:
```cpp
auto err = std::move(result).error();  // Efficient move
```

---

##### `T value_or(const T& default_value) const`
Get value or default if error.

**Parameters**:
- `default_value`: Default to return if failed

**Returns**: Value if success, default_value if failure

**Example**:
```cpp
int port = config_result.value_or(json(5432)).get<int>();
```

---

---

## Validator Framework API

### `class IValidator`

Abstract base class for validation strategies.

#### Header
```cpp
#include "validators/ivalidator.h"
```

#### Virtual Methods

##### `virtual ValidationResult validate(const json& value, const json& schema) = 0`
Validate a value against schema constraints.

**Parameters**:
- `value`: Value to validate
- `schema`: Schema definition for field

**Returns**: `ValidationResult` with success/failure info

**Example**:
```cpp
TypeValidator validator;
auto result = validator.validate(json("test"), json{{"type", "string"}});
```

---

##### `virtual std::string getName() const = 0`
Get validator name.

**Returns**: Validator name (e.g., "TypeValidator")

**Example**:
```cpp
std::cout << "Using validator: " << validator.getName() << std::endl;
```

---

### `class TypeValidator`

Validates JSON type constraints.

#### Header
```cpp
#include "validators/type_validator.h"
```

#### Supported Types
- string
- number
- integer
- boolean
- array
- object
- null

#### Example
```cpp
TypeValidator validator;
auto result = validator.validate(json(123), json{{"type", "integer"}});
```

---

### `class RangeValidator`

Validates numeric range constraints. ⚡ **OPTIMIZED with early exit**

#### Header
```cpp
#include "validators/range_validator.h"
```

#### Supported Constraints
- minimum
- maximum
- exclusiveMinimum
- exclusiveMaximum
- multipleOf

#### Example
```cpp
RangeValidator validator;
json schema = {
    {"type", "number"},
    {"minimum", 0},
    {"maximum", 100}
};
auto result = validator.validate(json(50), schema);
```

---

### `class PatternValidator`

Validates regex patterns. ⚡ **OPTIMIZED with thread-safe regex caching**

#### Header
```cpp
#include "validators/pattern_validator.h"
```

#### Features
- Thread-safe pattern caching (10-50x faster)
- Compiled regex reuse
- Automatic cache management

#### Example
```cpp
PatternValidator validator;
json schema = {
    {"type", "string"},
    {"pattern", "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"}
};
auto result = validator.validate(json("user@example.com"), schema);
```

**Performance**: First call: ~100μs, Subsequent: ~2μs (50x improvement)

---

### `class EnumValidator`

Validates against enumerated values.

#### Header
```cpp
#include "validators/enum_validator.h"
```

#### Example
```cpp
EnumValidator validator;
json schema = {
    {"type", "string"},
    {"enum", json::array({"red", "green", "blue"})}
};
auto result = validator.validate(json("red"), schema);
```

---

### `class RequiredValidator`

Validates required fields are present.

#### Header
```cpp
#include "validators/required_validator.h"
```

#### Example
```cpp
RequiredValidator validator;
json config = {{"username", "admin"}};
json schema = {{"required", json::array({"username", "password"})}};
auto result = validator.validate(config, schema);  // Fails: missing password
```

---

### `class SchemaValidator`

Orchestrates all validators for complete schema validation.

#### Header
```cpp
#include "core/schema_validator.h"
```

#### Public Methods

##### `std::vector<ValidationError> validate(const json& config, const JSONSchema& schema)`
Validate entire configuration against schema.

**Parameters**:
- `config`: Configuration to validate
- `schema`: Schema to validate against

**Returns**: Vector of validation errors (empty if valid)

**Example**:
```cpp
SchemaValidator validator;
auto errors = validator.validate(config, schema);
if (errors.empty()) {
    std::cout << "Configuration is valid" << std::endl;
} else {
    for (const auto& error : errors) {
        std::cerr << error.toString() << std::endl;
    }
}
```

---

---

## File I/O API

### `class JsonReader`

Reads JSON configuration files.

#### Header
```cpp
#include "io/json_reader.h"
```

#### Public Methods

##### `Result<json> readFile(const std::string& path)`
Read JSON from file.

**Parameters**:
- `path`: File path

**Returns**: `Result<json>` containing parsed JSON or error

**Example**:
```cpp
JsonReader reader;
auto result = reader.readFile("config.json");
if (result.is_success()) {
    auto config = result.value();
    std::cout << config << std::endl;
} else {
    std::cerr << result.error().message() << std::endl;
}
```

---

##### `Result<json> fromString(const std::string& content)`
Parse JSON from string.

**Parameters**:
- `content`: JSON string

**Returns**: `Result<json>` containing parsed JSON or error

**Example**:
```cpp
auto result = reader.fromString(R"({"status": "ok"})");
```

---

### `class JsonWriter` ⚡ **OPTIMIZED with move semantics**

Writes JSON configuration files.

#### Header
```cpp
#include "io/json_writer.h"
```

#### Public Methods

##### `Result<void> writeFile(const std::string& path, const json& data)`
Write JSON to file.

**Parameters**:
- `path`: Target file path
- `data`: JSON data to write

**Returns**: `Result<void>` indicating success or failure

**Example**:
```cpp
JsonWriter writer;
json config = {{"host", "localhost"}, {"port", 5432}};
auto result = writer.writeFile("config.json", config);
```

---

##### `Result<std::string> toString(const json& data)` ⚡ **OPTIMIZED**
Convert JSON to string with **move semantics**.

**Parameters**:
- `data`: JSON data

**Returns**: `Result<string>` containing formatted JSON

**Example**:
```cpp
auto result = writer.toString(config);
if (result.is_success()) {
    std::string json_str = std::move(result).value();  // Efficient move
    std::cout << json_str << std::endl;
}
```

---

### `class YamlReader`

Reads YAML configuration files.

#### Header
```cpp
#include "io/yaml_reader.h"
```

#### Public Methods

##### `Result<json> readFile(const std::string& path)`
Read YAML from file.

**Parameters**:
- `path`: File path

**Returns**: `Result<json>` containing parsed config or error

**Example**:
```cpp
YamlReader reader;
auto result = reader.readFile("config.yaml");
```

---

##### `Result<json> fromString(const std::string& content)`
Parse YAML from string.

**Parameters**:
- `content`: YAML content

**Returns**: `Result<json>` containing parsed config

**Example**:
```cpp
std::string yaml = R"(
host: localhost
port: 5432
)";
auto result = reader.fromString(yaml);
```

---

### `class YamlWriter`

Writes YAML configuration files.

#### Header
```cpp
#include "io/yaml_writer.h"
```

#### Public Methods

##### `Result<void> writeFile(const std::string& path, const json& data)`
Write YAML to file.

**Parameters**:
- `path`: Target file path
- `data`: Configuration data

**Returns**: `Result<void>` indicating success or failure

**Example**:
```cpp
YamlWriter writer;
auto result = writer.writeFile("config.yaml", config);
```

---

##### `Result<std::string> toString(const json& data)`
Convert config to YAML string.

**Parameters**:
- `data`: Configuration data

**Returns**: `Result<string>` containing formatted YAML

**Example**:
```cpp
auto result = writer.toString(config);
```

---

---

## UI Components API

### `class MainWindow : public QMainWindow`

Main application window.

#### Header
```cpp
#include "ui/main_window.h"
```

#### Public Methods

##### `MainWindow(QWidget* parent = nullptr)`
Constructor.

**Example**:
```cpp
auto window = std::make_unique<MainWindow>();
window->show();
```

---

##### `void loadSchema(const JSONSchema& schema)`
Load schema for configuration.

**Parameters**:
- `schema`: Schema to use for form generation

**Example**:
```cpp
JSONSchema schema(json::parse(schema_json));
window->loadSchema(schema);
```

---

##### `Result<ConfigurationData> getConfiguration() const`
Get current configuration from form.

**Returns**: `Result<ConfigurationData>` with current values

**Example**:
```cpp
auto result = window->getConfiguration();
if (result.is_success()) {
    auto config = result.value();
    std::cout << "Config: " << config.toJson() << std::endl;
}
```

---

##### `void setConfiguration(const ConfigurationData& config)`
Populate form with configuration data.

**Parameters**:
- `config`: Configuration to display

**Example**:
```cpp
ConfigurationData config(json::parse(config_str));
window->setConfiguration(config);
```

---

##### `void displayValidationErrors(const std::vector<ValidationError>& errors)`
Show validation errors in UI.

**Parameters**:
- `errors`: Validation errors to display

**Example**:
```cpp
auto errors = validator.validate(config, schema);
window->displayValidationErrors(errors);
```

---

### `class FormGenerator`

Generates forms from JSON schemas.

#### Header
```cpp
#include "ui/form_generator.h"
```

#### Public Methods

##### `QWidget* generateForm(const JSONSchema& schema)`
Generate form widget from schema.

**Parameters**:
- `schema`: Schema defining form structure

**Returns**: Pointer to generated form widget

**Example**:
```cpp
FormGenerator generator;
auto form = generator.generateForm(schema);
main_window->setCentralWidget(form);
```

---

### `class WidgetFactory`

Creates widgets for different field types.

#### Header
```cpp
#include "ui/widget_factory.h"
```

#### Static Methods

##### `static QWidget* createWidget(const std::string& field_type, const json& field_schema)`
Create widget for field type.

**Parameters**:
- `field_type`: Field type (string, number, boolean, etc.)
- `field_schema`: Field schema definition

**Returns**: Pointer to created widget

**Supported Types**:
- string → QLineEdit
- number → QDoubleSpinBox
- integer → QSpinBox
- boolean → QCheckBox
- array → QListWidget
- object → QGroupBox

**Example**:
```cpp
auto widget = WidgetFactory::createWidget("string", json{{"type", "string"}});
```

---

### `class ValidationFeedbackWidget : public QWidget`

Displays validation feedback for fields.

#### Header
```cpp
#include "ui/validation_feedback_widget.h"
```

#### Public Methods

##### `void setErrors(const std::vector<ValidationError>& errors)`
Display validation errors.

**Parameters**:
- `errors`: Errors to display

**Example**:
```cpp
ValidationFeedbackWidget feedback;
std::vector<ValidationError> errors{
    ValidationError("email", "Invalid email format")
};
feedback.setErrors(errors);
```

---

##### `void clearErrors()`
Clear validation feedback.

**Example**:
```cpp
feedback.clearErrors();
```

---

---

## Utility Functions API

### `namespace stringutils`

String manipulation utilities. ⚡ **OPTIMIZED with pre-allocation**

#### Header
```cpp
#include "utils/string_utils.h"
```

#### Functions

##### `std::string trim(const std::string& str)`
Remove leading/trailing whitespace.

**Parameters**:
- `str`: String to trim

**Returns**: Trimmed string

**Example**:
```cpp
auto trimmed = stringutils::trim("  hello world  ");
// Result: "hello world"
```

---

##### `std::string to_lower(const std::string& str)`
Convert string to lowercase.

**Parameters**:
- `str`: String to convert

**Returns**: Lowercase string

**Example**:
```cpp
auto lower = stringutils::to_lower("Hello World");
// Result: "hello world"
```

---

##### `std::string to_upper(const std::string& str)`
Convert string to uppercase.

**Parameters**:
- `str`: String to convert

**Returns**: Uppercase string

**Example**:
```cpp
auto upper = stringutils::to_upper("Hello World");
// Result: "HELLO WORLD"
```

---

##### `std::vector<std::string> split(const std::string& str, char delimiter)` ⚡ **OPTIMIZED**
Split string by delimiter with **pre-allocation**.

**Parameters**:
- `str`: String to split
- `delimiter`: Delimiter character

**Returns**: Vector of substrings

**Optimization**: Pre-allocates vector based on delimiter count

**Example**:
```cpp
auto parts = stringutils::split("one,two,three", ',');
// Result: ["one", "two", "three"]
```

---

##### `std::string replace(const std::string& str, const std::string& from, const std::string& to)`
Replace all occurrences of substring.

**Parameters**:
- `str`: Source string
- `from`: Substring to find
- `to`: Replacement substring

**Returns**: String with replacements

**Example**:
```cpp
auto result = stringutils::replace("hello world", "world", "ConfigGUI");
// Result: "hello ConfigGUI"
```

---

### `namespace fileutils`

File system utilities.

#### Header
```cpp
#include "utils/file_utils.h"
```

#### Functions

##### `bool exists(const std::string& path)`
Check if file exists.

**Parameters**:
- `path`: File path

**Returns**: true if exists, false otherwise

**Example**:
```cpp
if (fileutils::exists("config.json")) {
    // File exists
}
```

---

##### `Result<std::string> read_file(const std::string& path)`
Read file contents as string.

**Parameters**:
- `path`: File path

**Returns**: `Result<string>` with file contents or error

**Example**:
```cpp
auto result = fileutils::read_file("config.json");
if (result.is_success()) {
    std::string content = result.value();
}
```

---

##### `Result<void> write_file(const std::string& path, const std::string& content)`
Write string to file.

**Parameters**:
- `path`: File path
- `content`: Content to write

**Returns**: `Result<void>` indicating success or failure

**Example**:
```cpp
auto result = fileutils::write_file("output.json", json_string);
```

---

##### `bool is_directory(const std::string& path)`
Check if path is directory.

**Parameters**:
- `path`: Path to check

**Returns**: true if directory, false otherwise

**Example**:
```cpp
if (fileutils::is_directory("schemas")) {
    // Process directory
}
```

---

##### `std::vector<std::string> list_directory(const std::string& path)`
List files in directory.

**Parameters**:
- `path`: Directory path

**Returns**: Vector of filenames

**Example**:
```cpp
auto files = fileutils::list_directory("configs/");
for (const auto& file : files) {
    std::cout << file << std::endl;
}
```

---

### `class Logger`

Centralized logging system (Singleton).

#### Header
```cpp
#include "utils/logger.h"
```

#### Static Methods

##### `static Logger& getInstance()`
Get logger instance (Singleton).

**Returns**: Reference to logger instance

**Example**:
```cpp
auto& logger = Logger::getInstance();
```

---

##### `void debug(const std::string& message)`
Log debug message.

**Parameters**:
- `message`: Message to log

**Example**:
```cpp
Logger::getInstance().debug("Loading configuration...");
```

---

##### `void info(const std::string& message)`
Log info message.

**Parameters**:
- `message`: Message to log

**Example**:
```cpp
Logger::getInstance().info("Configuration loaded successfully");
```

---

##### `void warn(const std::string& message)`
Log warning message.

**Parameters**:
- `message`: Message to log

**Example**:
```cpp
Logger::getInstance().warn("Using default configuration");
```

---

##### `void error(const std::string& message)`
Log error message.

**Parameters**:
- `message`: Message to log

**Example**:
```cpp
Logger::getInstance().error("Failed to load schema");
```

---

---

## Error Handling API

### Error Codes

Common error codes returned in `ValidationError::code()`:

| Code | Meaning | Context |
|------|---------|---------|
| `INVALID_TYPE` | Wrong data type | TypeValidator |
| `OUT_OF_RANGE` | Value outside valid range | RangeValidator |
| `INVALID_FORMAT` | Value doesn't match pattern | PatternValidator |
| `NOT_IN_ENUM` | Value not in allowed list | EnumValidator |
| `FIELD_REQUIRED` | Required field missing | RequiredValidator |
| `INVALID_JSON` | JSON parsing error | JsonReader |
| `INVALID_YAML` | YAML parsing error | YamlReader |
| `FILE_NOT_FOUND` | File doesn't exist | File I/O |
| `IO_ERROR` | File read/write error | File I/O |

---

### Exception Policy

**ConfigGUI uses no exceptions** (MISRA C++ compliant).

All error conditions are handled via `Result<T, E>` return types:

```cpp
// ✓ Correct usage
auto result = reader.readFile("config.json");
if (result.is_success()) {
    auto config = result.value();
} else {
    auto error = result.error();
    std::cerr << error.message() << std::endl;
}

// ✗ Never throws
// try { ... } catch (...) { ... }  // Not used
```

---

### Error Handling Pattern

**Recommended pattern for error handling**:

```cpp
Result<ConfigurationData> loadConfiguration(const std::string& file_path) {
    // Step 1: Load file
    JsonReader reader;
    auto file_result = reader.readFile(file_path);
    if (file_result.is_failure()) {
        return Result<ConfigurationData>::Error(
            ValidationError(file_path, file_result.error().message(), "FILE_ERROR")
        );
    }
    
    // Step 2: Validate against schema
    SchemaValidator validator;
    auto json_data = file_result.value();
    auto errors = validator.validate(json_data, schema);
    if (!errors.empty()) {
        return Result<ConfigurationData>::Error(errors[0]);  // Return first error
    }
    
    // Step 3: Create configuration
    ConfigurationData config(json_data);
    return Result<ConfigurationData>::Ok(config);
}

// Usage
auto config_result = loadConfiguration("config.json");
if (config_result.is_success()) {
    auto config = std::move(config_result).value();  // Move semantics
    // Process configuration
} else {
    std::cerr << "Load failed: " << config_result.error().message() << std::endl;
}
```

---

## Common Usage Examples

### Loading and Validating Configuration

```cpp
#include "core/schema_loader.h"
#include "io/json_reader.h"
#include "core/schema_validator.h"

// 1. Load schema
SchemaLoader schema_loader;
auto schema_result = schema_loader.fromFile("schema.json");
auto schema = schema_result.value();

// 2. Load configuration
JsonReader reader;
auto config_result = reader.readFile("config.json");
auto config = config_result.value();

// 3. Validate
SchemaValidator validator;
auto errors = validator.validate(config, schema);

if (errors.empty()) {
    std::cout << "Configuration is valid!" << std::endl;
} else {
    for (const auto& error : errors) {
        std::cerr << error.toString() << std::endl;
    }
}
```

---

### Creating and Saving Configuration

```cpp
#include "core/configuration_data.h"
#include "io/json_writer.h"

// Create configuration
ConfigurationData config;
config.set("database.host", json("localhost"));
config.set("database.port", json(5432));

// Save to JSON
JsonWriter writer;
auto write_result = writer.writeFile("output.json", config.raw());

if (write_result.is_success()) {
    std::cout << "Configuration saved!" << std::endl;
} else {
    std::cerr << "Save failed: " << write_result.error().message() << std::endl;
}
```

---

## Performance Notes ⚡

- **Pattern Caching**: First regex: ~100μs, subsequent: ~2μs (**50x improvement**)
- **Vector Pre-allocation**: Reduces allocations by **20-30%**
- **Move Semantics**: Efficient value transfer in Result<T>
- **Schema Indexing**: O(n) → O(1) property lookups for large schemas (**50x improvement**)
- **String Operations**: Optimized with pre-allocation (**20% faster**)

See [PERFORMANCE_METRICS_REPORT.md](PERFORMANCE_METRICS_REPORT.md) for detailed benchmarks.

---

**ConfigGUI API Reference v1.0.0**  
**Date**: October 20, 2025  
**Status**: Production Ready
