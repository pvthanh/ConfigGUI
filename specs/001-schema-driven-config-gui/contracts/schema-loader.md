# Component Contract: Schema Loader

**Scope**: SchemaLoader, SchemaValidator components  
**Responsibility**: Load JSON Schema files and provide validation capability

## Interface Definition

### Primary Component: SchemaLoader

**Class**: `SchemaLoader`  
**Location**: `src/core/schema_loader.h/cpp`

**Purpose**: Load and parse JSON Schema files from disk, validate schema format, instantiate validator

#### Public Methods

```cpp
class SchemaLoader {
public:
    // Load schema from file
    Result<JSONSchema, FileError> loadSchema(
        const QString& filePath);
    
    // Load schema from JSON string
    Result<JSONSchema, FileError> loadSchemaFromString(
        const QString& schemaJson);
    
    // Validate schema against JSON Schema Draft 7 meta-schema
    Result<void, SchemaError> validateSchemaFormat(
        const nlohmann::json& schema);
    
    // Get schema metadata
    QString getSchemaName(const nlohmann::json& schema) const;
    QString getSchemaVersion(const nlohmann::json& schema) const;
    QString getSchemaDescription(const nlohmann::json& schema) const;
};
```

#### Input Contract

| Parameter | Type | Constraints | Error Behavior |
|-----------|------|-------------|----------------|
| filePath | QString | Must exist, readable, extension .json | FileError::NotFound, FileError::PermissionDenied |
| schemaJson | QString | Valid JSON format | FileError::InvalidJson |
| schema | nlohmann::json | Object with "type": "object" | SchemaError::InvalidSchema |

#### Output Contract

**Successful Load Returns**:
```cpp
struct JSONSchema {
    QString name;                              // From schema $id or filename
    QString version;                           // From schema version field
    QString description;                       // From schema description
    nlohmann::json schemaJson;                 // Raw schema
    nlohmann::json_schema::json_validator validator;  // Pre-instantiated
    QMap<QString, FieldMetadata> fields;      // Extracted metadata
    QStringList requiredFields;                // Required field names
};
```

**Error Returns**:
```cpp
enum class FileError {
    NotFound,                  // File doesn't exist
    PermissionDenied,          // Can't read file
    InvalidJson,               // JSON parse failed
    InvalidSchema,             // Not valid JSON Schema Draft 7
    SchemaReferenceNotSupported  // Contains $ref (v1.0 limitation)
};

enum class SchemaError {
    InvalidSchema,             // Doesn't conform to Draft 7
    NoRootObject,              // Root is not "type": "object"
    ContainsReferences,        // Has $ref fields
    CircularReference,         // Recursive schema definition
    UnsupportedKeyword         // Uses unsupported keyword
};
```

#### Validation Rules

| Rule | Enforcement |
|------|-------------|
| Root must be object | Reject with SchemaError::NoRootObject |
| Draft 7 compatibility | Validate against Draft 7 meta-schema |
| No $ref fields | Reject with SchemaError::ContainsReferences |
| Required fields present | Extract and validate |
| Field constraints present | Parse and cache (minLength, maxLength, pattern, enum, etc.) |

#### Example Usage

```cpp
SchemaLoader loader;

// Load from file
auto result = loader.loadSchema("/etc/schema.json");

if (result.isOk()) {
    JSONSchema schema = result.unwrap();
    
    qDebug() << "Schema:" << schema.name 
             << "v" << schema.version;
    
    // Now ready to create forms from schema
} else {
    FileError error = result.unwrapErr();
    showErrorDialog(error.toString());
}
```

---

## Secondary Component: SchemaValidator

**Class**: `SchemaValidator`  
**Location**: `src/validators/schema_validator.h/cpp`

**Purpose**: Validate configuration data against a loaded schema, report validation errors

### Public Methods

```cpp
class SchemaValidator : public QObject {
    Q_OBJECT
    
public:
    explicit SchemaValidator(const JSONSchema& schema);
    
    // Validate entire config
    ValidationResult validate(
        const nlohmann::json& configData) const;
    
    // Validate single field (for real-time validation)
    ValidationResult validateField(
        const QString& fieldPath,
        const QVariant& value) const;
    
    // Get field constraint info
    ConstraintInfo getFieldConstraints(
        const QString& fieldPath) const;
    
    // Check if all required fields present
    ValidationResult validateRequired(
        const nlohmann::json& configData) const;

signals:
    void validationProgress(int processedFields, int totalFields);
    void validationComplete(const ValidationResult& result);
    
private:
    const JSONSchema& schema_;
    QMap<QString, ConstraintInfo> constraintCache_;
};
```

#### Validation Result

```cpp
struct ValidationResult {
    bool isValid;                              // No errors?
    QList<ValidationError> errors;             // List of violations
    int errorCount;                            // Number of errors
    int warningCount;                          // Number of warnings
    
    bool operator==(const ValidationResult& other) const {
        return isValid == other.isValid && 
               errors == other.errors;
    }
};
```

#### Validation Error Details

```cpp
struct ValidationError {
    QString fieldPath;                         // e.g., "database.port"
    QString fieldName;                         // User-readable name
    QString errorType;                         // required, type, minimum, etc.
    QString message;                           // User-friendly message
    QString constraintName;                    // Constraint that failed
    QVariant constraintValue;                  // Value of constraint
    QVariant attemptedValue;                   // User's invalid input
    QStringList suggestions;                   // How to fix it
};
```

#### Input Contract

| Parameter | Type | Constraints |
|-----------|------|-------------|
| configData | nlohmann::json | Any valid JSON |
| fieldPath | QString | Dot-notation (e.g., "db.port") |
| value | QVariant | Any Qt value type |

#### Output Contract

**Valid Config Returns**:
```cpp
ValidationResult {
    isValid: true,
    errors: [],
    errorCount: 0,
    warningCount: 0
}
```

**Invalid Config Returns**:
```cpp
ValidationResult {
    isValid: false,
    errors: [
        ValidationError {
            fieldPath: "port",
            errorType: "minimum",
            message: "Port must be at least 1024",
            suggestions: ["Choose a port >= 1024"]
        },
        // ... more errors
    ],
    errorCount: 1,
    warningCount: 0
}
```

#### Validation Rules

| Constraint | Implementation |
|-----------|-----------------|
| Required | Check presence in JSON |
| Type | Check typeof() matches schema type |
| Minimum/Maximum | For numbers, compare values |
| MinLength/MaxLength | For strings, check length |
| Pattern | Regex match against pattern |
| Enum | Check value in enum list |
| MultipleOf | Check value % constraint == 0 |

#### Real-Time Validation Pattern

```cpp
// Usage in UI layer
connect(portSpinBox, qOverload<int>(&QSpinBox::valueChanged),
        [this](int value) {
            ValidationResult result = validator_.validateField(
                "port", 
                QVariant(value)
            );
            
            if (result.isValid) {
                portSpinBox->setStyleSheet("");  // Clear error style
            } else {
                portSpinBox->setStyleSheet("border: 2px solid red;");
                portSpinBox->setToolTip(result.errors[0].message);
            }
        });
```

---

## Integration Requirements

### SchemaLoader → SchemaValidator Flow

```
1. User clicks "Load Schema"
   ↓
2. SchemaLoader.loadSchema("/path/to/schema.json")
   ├─> Read file → parse JSON
   ├─> Validate against Draft 7 meta-schema
   ├─> Extract field metadata
   ├─> Create JSONSchema object
   └─> Instantiate nlohmann json-schema validator
   ↓
3. Create SchemaValidator(loadedSchema)
   ├─> Cache constraint info for each field
   ├─> Ready for real-time field validation
   └─> Ready for full config validation
   ↓
4. UI layer uses SchemaValidator for:
   ├─> Real-time field validation on edit
   ├─> Full config validation before save
   └─> Display constraint hints to user
```

### Error Handling Strategy

```cpp
// Client code pattern
SchemaLoader loader;
auto schemaResult = loader.loadSchema(filePath);

if (!schemaResult.isOk()) {
    // Handle load error
    FileError err = schemaResult.unwrapErr();
    switch (err.type) {
        case FileError::NotFound:
            showDialog("Schema file not found");
            break;
        case FileError::InvalidJson:
            showDialog("Schema JSON is invalid");
            break;
        case FileError::InvalidSchema:
            showDialog("Schema doesn't conform to JSON Schema Draft 7");
            break;
        case FileError::SchemaReferenceNotSupported:
            showDialog("Schema references not supported in v1.0");
            break;
    }
    return;
}

JSONSchema schema = schemaResult.unwrap();
auto validator = SchemaValidator(schema);

// Now validate config data
auto configResult = validator.validate(configData);
if (!configResult.isValid) {
    displayValidationErrors(configResult.errors);
}
```

---

## Performance Considerations

### Optimization Strategies

1. **Validator Caching**
   - SchemaLoader pre-instantiates nlohmann validator
   - Avoid recreating validator for each validation

2. **Constraint Caching**
   - SchemaValidator caches constraint info
   - Quick field lookup without re-parsing schema

3. **Partial Validation**
   - validateField() for real-time (single field)
   - validate() for full config (pre-save check)

### Performance Targets

| Operation | Target | Notes |
|-----------|--------|-------|
| Load schema | <500ms | File I/O + parsing + validation |
| Validate config | <100ms | For typical configs (<10MB) |
| Single field validation | <10ms | Real-time feedback |

---

## Testing Contract

### Unit Tests

- ✅ Load valid schema from file → JSONSchema returned
- ✅ Load invalid JSON → FileError::InvalidJson
- ✅ Load schema with $ref → FileError::SchemaReferenceNotSupported
- ✅ Validate valid config → ValidationResult.isValid == true
- ✅ Validate invalid config → ValidationResult.errors populated
- ✅ Single field validation → Returns specific field error

### Integration Tests

- ✅ Load schema → Create validator → Validate config (full flow)
- ✅ Multiple schemas loaded in sequence
- ✅ Schema with complex nested objects
- ✅ Schema with array types

---

## Contract Version

**Version**: 1.0  
**Last Updated**: 2025-10-20  
**Status**: Ready for Implementation

