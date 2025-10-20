# Phase 1: Data Model & Domain Entities

**Date**: 2025-10-20  
**Feature**: Schema-Driven Configuration GUI  
**Status**: Complete

## Overview

This document defines the core domain entities and data structures for ConfigGUI. These entities represent the fundamental business concepts manipulated by the application.

---

## Core Entities

### 1. JSONSchema

**Purpose**: Represents a parsed JSON Schema that defines configuration structure

**Definition**:
```cpp
class JSONSchema {
public:
    // Identification
    QString name;                          // Schema name/ID
    QString version;                       // Schema version (e.g., "1.0.0")
    QString description;                   // Human-readable description
    
    // Structure
    nlohmann::json schemaJson;             // Raw schema JSON
    nlohmann::json_schema::json_validator  // Validator instance
        validator;
    
    // Metadata
    QMap<QString, FieldMetadata> fields;   // Field descriptions, examples
    QStringList requiredFields;            // Fields that must be set
    QMap<QString, ConstraintInfo> constraints;  // Field constraints
    
    // Methods
    Result<void, ValidationError> validate(
        const nlohmann::json& data) const;
    
    QStringList getTopLevelFields() const;
    FieldType getFieldType(const QString& fieldPath) const;
    QString getFieldDescription(const QString& fieldPath) const;
};
```

**Key Fields**:
- `schemaJson`: The raw JSON Schema as parsed from file (Draft 7)
- `validator`: Pre-instantiated nlohmann json-schema validator for performance
- `fields`: Metadata extracted from schema (descriptions, examples, constraints)
- `requiredFields`: Cached list of required field names for UI rendering

**Relationships**:
- Used by: FormGenerator (to create UI), SchemaValidator (to validate)
- Related to: FieldType, ConstraintInfo, FieldMetadata

**Validation Rules**:
- Schema must be valid JSON Schema Draft 7
- Must have "type": "object" at root level
- Cannot contain `$ref` to external files (v1.0 limitation)

**Examples**:

```json
{
  "name": "app_config",
  "version": "1.0",
  "description": "Application configuration schema",
  "type": "object",
  "required": ["appName", "port"],
  "properties": {
    "appName": {
      "type": "string",
      "minLength": 1,
      "maxLength": 100,
      "description": "Application name"
    },
    "port": {
      "type": "integer",
      "minimum": 1024,
      "maximum": 65535,
      "description": "Server port number"
    }
  }
}
```

---

### 2. ConfigurationData

**Purpose**: Represents user-provided configuration values bound to a schema

**Definition**:
```cpp
class ConfigurationData {
public:
    // Core data
    QString schemaName;                   // Associated schema name
    nlohmann::json values;                // Current configuration values
    
    // State tracking
    QMap<QString, bool> fieldTouched;     // Which fields have been edited
    QMap<QString, QString> fieldErrors;   // Validation errors per field
    
    // Metadata
    QDateTime lastModified;               // Timestamp of last change
    QString sourceFile;                   // Path to loaded config file (if any)
    FileFormat sourceFormat;              // Format it was loaded from (JSON/YAML)
    
    // Methods
    void setValue(const QString& fieldPath, const QVariant& value);
    QVariant getValue(const QString& fieldPath) const;
    
    bool isValid() const;
    bool hasErrors() const;
    QStringList getErrorFields() const;
    
    Result<void, FileError> loadFromFile(const QString& filePath);
    Result<void, FileError> saveToFile(
        const QString& filePath,
        FileFormat format) const;
};
```

**Key Fields**:
- `values`: nlohmann::json object containing all user-provided values
- `fieldTouched`: Tracks which fields user has edited (for validation UI)
- `fieldErrors`: Maps field paths to error messages
- `sourceFormat`: Tracks whether loaded from JSON or YAML (for consistency in save)

**Relationships**:
- Contains: Multiple ValidationError objects
- References: JSONSchema (for validation)
- Used by: FormGenerator (to populate fields), FileHandler (to save)

**State Transitions**:

```
┌─────────────────────────────────┐
│  ConfigurationData Created      │
│  (empty values, no errors)      │
└────────────┬────────────────────┘
             │
             ├─> User edits field
             │   └─> setValue() → Validation triggered
             │       ├─> VALID: fieldErrors cleared
             │       └─> INVALID: fieldErrors[field] = message
             │
             ├─> Load from file
             │   └─> loadFromFile() → Populate values
             │       └─> Trigger validation for all fields
             │
             └─> Save to file (if valid)
                 └─> saveToFile() → Write values as JSON/YAML
```

**Validation Rules**:
- Cannot save if `hasErrors()` returns true
- Must have all required fields populated (from schema)
- All values must satisfy field constraints (type, range, pattern, enum)

**Example**:

```cpp
ConfigurationData config;
config.schemaName = "app_config";
config.setValue("appName", "MyApp");
config.setValue("port", 8080);

if (config.isValid()) {
    config.saveToFile("/etc/myapp.json", FileFormat::JSON);
}
```

---

### 3. ValidationError

**Purpose**: Represents a single constraint violation when data doesn't match schema

**Definition**:
```cpp
class ValidationError {
public:
    // Error identification
    QString fieldPath;                    // Path to problematic field (e.g., "database.port")
    QString fieldName;                    // Human-readable field name
    QString errorType;                    // Type of error (required, type, range, pattern, enum)
    QString message;                      // User-friendly error message
    
    // Context
    QString constraintName;               // Which constraint was violated
    QVariant constraintValue;             // Value of the constraint (e.g., max value)
    QVariant attemptedValue;              // User's invalid input
    
    // Suggestions
    QStringList suggestions;              // Suggestions to fix (e.g., allowed enum values)
    
    // Severity
    enum Severity { Warning, Error, Critical };
    Severity severity;
    
    // Methods
    QString toDisplayString() const;      // Format for UI display
};
```

**Key Fields**:
- `fieldPath`: JSON path (e.g., "database.credentials.username")
- `errorType`: One of: "required", "type", "minimum", "maximum", "pattern", "enum", "minLength", "maxLength"
- `suggestions`: For enums, contains list of allowed values; for patterns, explains format

**Relationships**:
- Belongs to: ConfigurationData (one Config has many Errors)
- Created by: SchemaValidator
- Displayed by: FormValidatorDisplay

**Error Type Mapping**:

| JSON Schema Constraint | ErrorType | Example Message |
|----------------------|-----------|-----------------|
| required | "required" | "Field 'port' is required" |
| type | "type" | "Expected integer, got string" |
| minimum | "minimum" | "Value must be at least 1024" |
| maximum | "maximum" | "Value cannot exceed 65535" |
| minLength | "minLength" | "Must be at least 3 characters" |
| maxLength | "maxLength" | "Cannot exceed 100 characters" |
| pattern | "pattern" | "Must match format: xxx-xxx-xxxx" |
| enum | "enum" | "Must be one of: [option1, option2, option3]" |
| multipleOf | "multipleOf" | "Must be a multiple of 5" |

**Example**:

```cpp
ValidationError error;
error.fieldPath = "database.port";
error.fieldName = "Port";
error.errorType = "minimum";
error.message = "Port number must be at least 1024";
error.constraintName = "minimum";
error.constraintValue = 1024;
error.attemptedValue = 80;
error.severity = ValidationError::Error;

// Display: "Port: Value must be at least 1024 (constraint: minimum 1024)"
```

---

### 4. FormState

**Purpose**: Tracks current UI form state and user interactions

**Definition**:
```cpp
class FormState {
public:
    // Field states
    QMap<QString, WidgetState> fieldStates;  // State per field
    QMap<QString, QWidget*> fieldWidgets;    // Widget references
    
    // Focus and interaction
    QString focusedField;                 // Currently focused field
    QSet<QString> expandedGroups;         // Which nested groups are expanded
    QMap<QString, int> scrollPositions;   // Scroll state for large forms
    
    // Modification tracking
    bool isDirty;                         // Has data been modified?
    QDateTime modificationTime;           // When was it last modified?
    
    // Methods
    void markFieldTouched(const QString& fieldPath);
    void markFieldDirty();
    void updateFieldWidget(
        const QString& fieldPath,
        const QVariant& value);
    
    void showValidationError(
        const QString& fieldPath,
        const QString& message);
    void clearValidationError(const QString& fieldPath);
    
    bool hasUnsavedChanges() const;
};
```

**Widget State Sub-Entity**:
```cpp
struct WidgetState {
    QVariant currentValue;              // Widget's current value
    bool isValid;                       // Passes validation?
    QString validationMessage;          // Error message if invalid
    bool isTouched;                     // User has interacted with this field?
    bool isVisible;                     // Is widget visible in UI?
    bool isEnabled;                     // Is widget enabled for editing?
};
```

**Key Fields**:
- `fieldStates`: Maps field path → WidgetState for each form field
- `focusedField`: Used to set focus on first error field
- `expandedGroups`: For nested objects, tracks which QGroupBox sections are expanded
- `isDirty`: Used to prompt "Save changes?" on exit

**Relationships**:
- Belongs to: FormGenerator (UI creates/maintains FormState)
- References: ConfigurationData (synced with data changes)
- Observes: Qt signals from widgets (textChanged, valueChanged, etc.)

**State Diagram**:

```
┌──────────────────────┐
│  Form Displayed      │
│  (isDirty = false)   │
└──────────┬───────────┘
           │
           ├─> User edits field
           │   └─> fieldStates[field].currentValue updated
           │   └─> isDirty = true
           │   └─> Validation triggered
           │   ├─> VALID: fieldStates[field].isValid = true
           │   └─> INVALID: Show error UI
           │
           ├─> User saves
           │   └─> isDirty = false
           │   └─> ConfigurationData.values synced
           │
           └─> User closes (isDirty = true)
               └─> Prompt "Save changes?"
```

**Example**:

```cpp
FormState formState;
formState.markFieldTouched("port");
formState.updateFieldWidget("port", 8080);
formState.showValidationError("port", "Port must be 1024-65535");
formState.markFieldDirty();

if (formState.hasUnsavedChanges()) {
    // Prompt user before closing
}
```

---

## Supporting Types & Enums

### FileFormat Enum
```cpp
enum class FileFormat {
    JSON,
    YAML
};
```

### FieldType Enum
```cpp
enum class FieldType {
    String,
    Integer,
    Number,
    Boolean,
    Array,
    Object,
    Null
};
```

### ConstraintInfo Structure
```cpp
struct ConstraintInfo {
    FieldType fieldType;
    QVariant minValue;                  // For numbers
    QVariant maxValue;                  // For numbers
    int minLength = -1;                 // For strings
    int maxLength = -1;                 // For strings
    QStringList enumValues;             // For enum constraints
    QString pattern;                    // For pattern constraints
    bool isRequired = false;
    QString description;
    QString example;
};
```

### FieldMetadata Structure
```cpp
struct FieldMetadata {
    QString description;
    QString example;
    QStringList enumValues;
    QString defaultValue;
    bool isRequired = false;
    QString helpText;
    QMap<QString, QString> customAttributes;  // x-* schema extensions
};
```

---

## Entity Relationships Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      JSONSchema                             │
│  (Schema structure, constraints, metadata)                  │
└────────────────────────┬────────────────────────────────────┘
                         │
                         │ defines structure for
                         ↓
┌─────────────────────────────────────────────────────────────┐
│                  ConfigurationData                          │
│  (User-provided values, state, validation status)           │
└────────────────────────┬────────────────────────────────────┘
                         │
         ┌───────────────┼───────────────┐
         │               │               │
         ↓               ↓               ↓
    ┌────────────┐  ┌──────────────┐  ┌───────────┐
    │ Validated  │  │FormState     │  │   File    │
    │    by      │  │(UI tracking) │  │ I/O Ops   │
    │            │  │              │  │           │
    └────┬───────┘  └──────┬───────┘  └─────┬─────┘
         │                  │                │
         ↓                  ↓                ↓
    ┌─────────────────────────────────────────────────┐
    │ ValidationError                                 │
    │ (Constraint violations per field)               │
    │ - displayed in UI via FormState                 │
    │ - stored in ConfigurationData.fieldErrors       │
    └─────────────────────────────────────────────────┘
```

---

## Data Flow Example

### Scenario: User edits a field value

```
1. User types in QLineEdit (port field)
   │
   ├─> Qt signal: QLineEdit::textChanged("8080")
   │
   ├─> Slot: FormGenerator::onFieldChanged("port", "8080")
   │   │
   │   ├─> FormState.updateFieldWidget("port", 8080)
   │   │
   │   ├─> ConfigurationData.setValue("port", 8080)
   │   │   └─> ConfigurationData.values["port"] = 8080
   │   │
   │   └─> Trigger validation:
   │       │
   │       ├─> JSONSchema.validate(ConfigurationData.values)
   │       │   └─> nlohmann_json_schema validates
   │       │
   │       └─> Result:
   │           ├─> VALID:
   │           │   └─> FormState.clearValidationError("port")
   │           │   └─> Update UI: remove error styling
   │           │
   │           └─> INVALID:
   │               └─> Create ValidationError
   │               └─> FormState.showValidationError("port", "...")
   │               └─> ConfigurationData.fieldErrors["port"] = message
   │               └─> Update UI: show red border, tooltip
   │
   └─> FormState.markFieldDirty()
```

---

## Migration Considerations

### From v1.0 to v1.1

**Future: Schema References ($ref)**
- JSONSchema will support `$ref` to local files
- ConfigurationData unchanged
- ValidationError unchanged
- FormState unchanged

**Future: Additional Formats (TOML, etc.)**
- FileFormat enum extended with TOML
- ConfigurationData.sourceFormat updated
- FileHandler gains new reader/writer types
- No changes to core entity structure

---

## Summary

These four core entities form the backbone of ConfigGUI:

1. **JSONSchema**: What the config should look like
2. **ConfigurationData**: What the user provided
3. **ValidationError**: What's wrong with the data
4. **FormState**: How the UI is presenting it to the user

Together, they enable:
- ✅ Dynamic form generation from schemas
- ✅ Real-time validation with immediate user feedback
- ✅ State preservation during user interactions
- ✅ Conversion between JSON/YAML formats
- ✅ Save/load config files with validation

