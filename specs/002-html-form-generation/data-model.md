# Data Model: HTML Form Generation

**Date**: October 24, 2025  
**Feature Branch**: `002-html-form-generation`  
**Status**: Phase 1 Design Complete

---

## Overview

This document defines the data structures, entities, and their relationships for the HTML Form Generation feature. All entities are shared between Qt and HTML implementations through a common C++ core.

---

## Core Entity Models

### 1. Schema Entity

**Purpose**: Represents a JSON or YAML schema file defining configuration structure, types, and validation rules

**Definition**:
```cpp
// src/core/schema/schema.h
struct Schema {
    std::string id;                          // Unique identifier (filename without ext)
    std::string title;                       // Schema title (from $schema metadata)
    std::string description;                 // Schema description
    nlohmann::json schema_json;              // Raw JSON schema document
    std::vector<Property> properties;        // Top-level properties
    std::vector<std::string> required_fields; // Required property names
    std::string source_path;                 // File path to schema
    std::string version;                     // Schema version (if specified)
};
```

**Fields**:
- `id`: Used for schema references in form generation; derived from filename
- `schema_json`: Stored for serialization and reference
- `properties`: Extracted and structured for UI generation
- `source_path`: Enables schema reloading/refresh

**Relationships**:
- Schema → Many Properties (1:N)
- Schema → Many ValidationRules (through Properties)

**Example**:
```json
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Application Configuration",
    "description": "Main application settings",
    "type": "object",
    "properties": {
        "app_name": { "type": "string" },
        "port": { "type": "integer", "minimum": 1024 }
    },
    "required": ["app_name"]
}
```

---

### 2. Property Entity

**Purpose**: Represents a single field in a schema (string, number, object, array, etc.)

**Definition**:
```cpp
// src/core/schema/property.h
struct Property {
    std::string name;                           // Property name (from schema key)
    std::string title;                          // Display title (from schema title or generated)
    std::string description;                    // Field description
    std::string type;                           // JSON type: string, number, boolean, object, array
    std::optional<nlohmann::json> default_value; // Default value if provided
    bool required;                              // Is this field required?
    std::vector<ValidationRule> rules;          // Validation constraints
    std::optional<std::vector<Property>> nested_properties; // If type=object
    std::optional<Property> array_items;        // If type=array
    std::map<std::string, std::string> ui_hints; // x-* custom UI hints from schema
};
```

**Fields**:
- `name`: Maps directly to JSON key
- `type`: Determines input field type in HTML
- `rules`: Contains pattern, min/max, enum, etc.
- `nested_properties`: For nested objects (recursive)
- `ui_hints`: Schema extensions (x-component, x-placeholder, etc.)

**Type Mappings to HTML**:
| JSON Type | HTML Input | Example |
|-----------|-----------|---------|
| string | text, textarea, or select | app_name → `<input type="text">` |
| number/integer | number input | port → `<input type="number">` |
| boolean | checkbox | enabled → `<input type="checkbox">` |
| object | fieldset | config → nested form |
| array | repeatable section | servers → list of objects |

**Example**:
```json
{
    "name": "database_url",
    "title": "Database URL",
    "description": "Connection string for PostgreSQL",
    "type": "string",
    "required": true,
    "rules": [
        { "type": "pattern", "value": "^postgresql://.*" }
    ],
    "ui_hints": {
        "x-placeholder": "postgresql://user:pass@localhost/db"
    }
}
```

---

### 3. ValidationRule Entity

**Purpose**: Represents constraints and validation requirements for a property

**Definition**:
```cpp
// src/core/schema/validation_rule.h
struct ValidationRule {
    std::string type;                    // Rule type: required, pattern, minLength, maxLength, 
                                        // minimum, maximum, enum, uniqueItems, etc.
    nlohmann::json constraint;           // Rule value/constraint (pattern regex, min value, etc.)
    std::string error_message;           // Custom error message for this rule
    
    // Evaluation
    Result<bool> evaluate(const nlohmann::json& value) const;
    std::string getErrorMessage(const std::string& field_name) const;
};
```

**Standard Rule Types**:
| Rule Type | Value | Example |
|-----------|-------|---------|
| required | boolean | Makes field mandatory |
| pattern | regex string | `/^[a-z]+$/` for lowercase only |
| minLength | integer | Min string length |
| maxLength | integer | Max string length |
| minimum | number | Min numeric value |
| maximum | number | Max numeric value |
| enum | array | ["dev", "staging", "prod"] |
| minItems | integer | Min array length |
| maxItems | integer | Max array length |
| const | value | Field must equal this value |

**Examples**:
```cpp
ValidationRule port_rule = {
    type: "minimum",
    constraint: 1024,
    error_message: "Port must be >= 1024"
};

ValidationRule pattern_rule = {
    type: "pattern",
    constraint: "^[a-zA-Z0-9_-]+$",
    error_message: "Only alphanumeric, dash, underscore allowed"
};
```

---

### 4. Configuration Entity

**Purpose**: Represents user-filled form data conforming to a schema

**Definition**:
```cpp
// src/core/data/configuration_data.h
class ConfigurationData {
    std::string schema_id;                    // Reference to Schema
    nlohmann::json data;                      // User-filled values
    std::map<std::string, ValidationError> validation_errors; // Field validation state
    std::string source_path;                  // File path (if loaded from file)
    
public:
    // Core operations
    Result<bool> validateAgainstSchema(const Schema& schema);
    Result<void> saveToFile(const std::string& format); // json or yaml
    Result<void> loadFromFile(const std::string& path);
    nlohmann::json toJson() const;
    std::string toYaml() const;
    
    // Field operations
    Result<void> setFieldValue(const std::string& path, const nlohmann::json& value);
    nlohmann::json getFieldValue(const std::string& path) const;
    ValidationError getFieldError(const std::string& path) const;
};
```

**State Transitions**:
```
[Empty]
   ↓ loadSchema()
[Schema Loaded]
   ↓ loadFromFile() or setFieldValue()
[Partially Filled]
   ↓ validateAgainstSchema()
[Valid] or [Invalid with Errors]
   ↓ saveToFile() (only if Valid)
[Saved]
```

**Example**:
```json
{
    "schema_id": "app_config",
    "data": {
        "app_name": "MyApp",
        "port": 8080,
        "database_url": "postgresql://localhost/mydb"
    },
    "validation_errors": {},
    "is_valid": true
}
```

---

### 5. ValidationError Entity

**Purpose**: Represents a validation failure for a specific field

**Definition**:
```cpp
// src/core/schema/validation_error.h
struct ValidationError {
    std::string field_name;              // Path to field (e.g., "config.database.host")
    std::string rule_type;               // Which rule failed (pattern, required, etc.)
    std::string message;                 // User-friendly error message
    nlohmann::json attempted_value;      // Value that failed validation
    
    static ValidationError required(const std::string& field);
    static ValidationError pattern(const std::string& field, const std::string& pattern);
    static ValidationError rangeError(const std::string& field, double min, double max);
};
```

**HTTP Representation**:
```json
{
    "field_name": "port",
    "rule_type": "minimum",
    "message": "Port must be >= 1024",
    "attempted_value": 500
}
```

---

### 6. FormTemplate Entity (HTML specific)

**Purpose**: Represents generated HTML form structure

**Definition**:
```javascript
// src/html/assets/js/form-template.js
class FormTemplate {
    constructor(schema, properties) {
        this.schema = schema;
        this.properties = properties;
        this.formId = `form_${schema.id}`;
    }
    
    // Generate HTML form structure
    toHtml() { /*...*/ }
    
    // Generate validation feedback elements
    getValidationFeedback(fieldName, errors) { /*...*/ }
    
    // Get form data as JSON
    getFormData() { /*...*/ }
}
```

**HTML Structure Template**:
```html
<form id="form_app_config" class="configgui-form">
    <!-- Schema selection -->
    <div class="schema-info">
        <h2>Application Configuration</h2>
        <p>Main application settings</p>
    </div>
    
    <!-- Form fields -->
    <div class="form-field">
        <label for="app_name">App Name *</label>
        <input type="text" id="app_name" name="app_name" required>
        <div class="field-error" style="display:none;"></div>
    </div>
    
    <!-- Nested objects -->
    <fieldset class="nested-object">
        <legend>Database Settings</legend>
        <!-- nested fields -->
    </fieldset>
    
    <!-- Arrays/repeatable sections -->
    <div class="array-section" data-property="servers">
        <button class="add-item">+ Add Server</button>
        <div class="array-items">
            <!-- dynamically added items -->
        </div>
    </div>
    
    <!-- Submit -->
    <button type="submit" class="btn-primary">Save Configuration</button>
    <button type="button" class="btn-secondary">Cancel</button>
</form>
```

---

## Validation Rules Definition

### Shared Validator (Core)

```cpp
// src/core/schema/schema_validator.h
class SchemaValidator {
public:
    // Validate single field
    Result<bool> validateField(
        const std::string& field_name,
        const Property& property,
        const nlohmann::json& value
    ) const;
    
    // Validate entire configuration
    std::vector<ValidationError> validateConfiguration(
        const ConfigurationData& config,
        const Schema& schema
    ) const;
    
    // Get validation rules for property
    std::vector<ValidationRule> getRulesForProperty(
        const Property& property
    ) const;
};
```

### Validation Flow

**Qt Application**:
```
User edits field → Qt widget change signal → Core validator
→ Validation result → UI feedback (error tooltip, red highlight)
```

**HTML Application**:
```
User edits field → JavaScript change event → HTTP POST /api/validate
→ Core validator (backend) → JSON error response
→ JavaScript updates DOM (error message, red highlight)
```

**Parity**: Both use same SchemaValidator from core; UI feedback differs (Qt: native, HTML: CSS) but validation logic identical

---

## Data Flow Diagrams

### Form Generation Flow

```
[User selects schema in HTML main form]
         ↓
[HTTP POST /api/schema/load {schema_id: "app_config"}]
         ↓
[Core: SchemaLoader → loads schema file]
         ↓
[Core: SchemaValidator → extracts properties and rules]
         ↓
[HTML Server: converts to JSON response]
         ↓
[JavaScript FormGenerator.generateForm() → creates DOM elements]
         ↓
[Form displayed in browser with validation listeners]
```

### Validation Flow (Real-time)

```
[User types in field]
         ↓
[JavaScript change event]
         ↓
[HTTP POST /api/validate {field: "port", value: 500}]
         ↓
[Core: SchemaValidator.validateField()]
         ↓
[JSON response with error or success]
         ↓
[JavaScript updates field styling and error message]
```

### Configuration Save Flow

```
[User fills form and clicks Save]
         ↓
[JavaScript collects form data → {app_name: "...", port: 8080, ...}]
         ↓
[HTTP POST /api/config/save {schema_id: "app_config", data: {...}}]
         ↓
[Core: ConfigurationData.validateAgainstSchema()]
         ↓
[If Valid: ConfigurationData.saveToFile()]
         ↓
[HTTP response with success or validation errors]
         ↓
[JavaScript displays success message or error feedback]
```

---

## Key Constraints & Considerations

### Type Safety
- All entities use strong C++ types (std::string, std::vector, etc.)
- No void* or weak typing
- Result<T> pattern for fallible operations

### MISRA C++ Compliance
- Smart pointers for all dynamic allocations (std::unique_ptr, std::shared_ptr)
- No raw new/delete
- Const-correctness throughout
- No variable-length arrays

### Validation Consistency
- Single SchemaValidator instance shared between Qt and HTML
- Same validation rules produce identical results
- Deterministic error messages

### Performance
- Lazy property extraction (don't parse entire schema until needed)
- Cached validation rule results
- Efficient nested property traversal

---

## Persistence Format

### JSON Format (preferred)
```json
{
    "schema_id": "app_config",
    "app_name": "MyApplication",
    "port": 8080,
    "database_url": "postgresql://localhost/db",
    "features": {
        "auth": true,
        "ssl": false
    }
}
```

### YAML Format (optional)
```yaml
schema_id: app_config
app_name: MyApplication
port: 8080
database_url: postgresql://localhost/db
features:
  auth: true
  ssl: false
```

Both formats supported equally; determined by file extension or Content-Type header

---

## Next: Phase 1 Contracts & Quickstart

With data model defined, proceed to:
- `contracts/openapi.yaml` - HTTP API specification
- `contracts/schema-load-contract.md` - Schema loading guarantees
- `quickstart.md` - Developer setup guide

---

**Data Model Complete**: All entities, relationships, and validation flows defined.
