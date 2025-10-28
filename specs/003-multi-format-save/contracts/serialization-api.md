# Serialization API Contract

**Date**: October 27, 2025  
**Version**: 1.0  
**Status**: Design  

---

## Overview

This document defines the public API contracts for the format serialization subsystem in ConfigGUI's multi-format save feature.

---

## Core Serialization API

### Interface: FormatSerializer (Abstract Base)

**Responsibility**: Define contract for any format serializer implementation

**Location**: `src/core/serializers/format_serializer.h`

#### Method: serialize()

```cpp
virtual SerializationResult serialize(
    const SerializationContext& context) = 0;
```

**Contract**:
- **Preconditions**:
  - `context.config_data` is valid JSON
  - `context.config_data` passes schema validation
  - `context.target_format` is one of FormatType values
  
- **Postconditions**:
  - On success: Returns `Result<string>` containing serialized content
  - On error: Returns `Result<string>` with `SerializationError` describing failure
  
- **Error Cases**:
  - `INVALID_DATA`: Configuration fails validation
  - `ENCODING_ERROR`: Data cannot be represented in target format
  - `UNKNOWN`: Other serialization errors
  
- **Side Effects**: None (pure function, no I/O)

- **Idempotency**: Idempotent - same input always produces same output

**Usage**:
```cpp
auto result = serializer->serialize(context);
if (result.is_ok()) {
    std::string content = result.value();
} else {
    handle_serialization_error(result.error());
}
```

---

#### Method: deserialize()

```cpp
virtual Result<nlohmann::json> deserialize(
    const std::string& content) = 0;
```

**Contract**:
- **Preconditions**:
  - `content` is valid text in target format
  
- **Postconditions**:
  - On success: Returns `Result<json>` containing parsed JSON
  - On error: Returns `Result<json>` with error details
  
- **Error Cases**:
  - `INVALID_FORMAT`: Malformed content for format
  - `ENCODING_ERROR`: Character encoding issues
  - `UNKNOWN`: Other parsing errors
  
- **Side Effects**: None

- **Idempotency**: Idempotent

**Usage**:
```cpp
auto result = serializer->deserialize(content);
if (auto json = result.value_or(nullptr)) {
    // Use parsed JSON
}
```

---

#### Method: validate_pre_serialization()

```cpp
virtual std::optional<SerializationError> validate_pre_serialization(
    const SerializationContext& context) const = 0;
```

**Contract**:
- **Preconditions**:
  - `context` is populated with configuration data
  
- **Postconditions**:
  - Returns `std::nullopt` if validation passes
  - Returns `SerializationError` if validation fails
  
- **Validation Rules** (format-specific):
  - JSON: Ensures no circular references, all types valid
  - INI: Checks for special characters that need escaping, size limits
  
- **Side Effects**: None (read-only validation)

- **Idempotency**: Idempotent

**Usage**:
```cpp
if (auto error = serializer->validate_pre_serialization(context)) {
    return error;  // Validation failed
}
```

---

#### Method: get_mime_type()

```cpp
virtual std::string get_mime_type() const = 0;
```

**Contract**:
- **Returns**: MIME type string for the format
  - JSON: `"application/json"`
  - INI: `"text/plain"` or `"text/x-ini"`
  
- **Side Effects**: None

**Usage**:
```cpp
std::string mime = serializer->get_mime_type();
response.set_header("Content-Type", mime);
```

---

#### Method: get_format_name()

```cpp
virtual std::string get_format_name() const = 0;
```

**Contract**:
- **Returns**: Human-readable format name
  - JSON: `"JSON"`
  - INI: `"INI"`
  
- **Side Effects**: None

**Usage**:
```cpp
std::cout << "Saving as: " << serializer->get_format_name() << std::endl;
```

---

### Factory: SerializerFactory

**Responsibility**: Create appropriate serializer instances

**Location**: `src/core/serializers/serializer_factory.h`

#### Function: create_serializer()

```cpp
static FormatSerializerPtr create_serializer(FormatType format);
```

**Contract**:
- **Preconditions**:
  - `format` is valid (JSON or INI)
  
- **Postconditions**:
  - Returns `unique_ptr<FormatSerializer>` for the format
  - Pointer is valid and ready to use
  
- **Error Cases**:
  - Throws `std::invalid_argument` if format is invalid
  
- **Exceptions**: `std::invalid_argument`

**Usage**:
```cpp
auto serializer = SerializerFactory::create_serializer(FormatType::INI);
```

---

#### Function: create_serializer_from_path()

```cpp
static FormatSerializerPtr create_serializer_from_path(
    const std::string& file_path);
```

**Contract**:
- **Preconditions**:
  - `file_path` ends with `.json` or `.ini`
  
- **Postconditions**:
  - Returns appropriate serializer based on extension
  
- **Error Cases**:
  - Throws `std::invalid_argument` if extension not recognized
  
- **Exceptions**: `std::invalid_argument`

- **Behavior**:
  - Case-insensitive extension matching
  - Ignores path components, only checks final extension

**Usage**:
```cpp
auto serializer = SerializerFactory::create_serializer_from_path(
    "/path/to/config.ini"
);
```

---

## File I/O API

### Function: write_configuration_file()

**Location**: `src/io/configuration_writer.h`

```cpp
Result<void> write_configuration_file(
    const std::string& file_path,
    const std::string& content,
    bool overwrite = false);
```

**Contract**:
- **Preconditions**:
  - `file_path` is absolute or relative path
  - `content` is valid serialized format
  - `overwrite` flag indicates if existing file should be replaced
  
- **Postconditions**:
  - On success: File written to disk, returns `Result<void>` (ok)
  - On error: File unchanged, returns `Result<void>` with error
  
- **Error Cases**:
  - `PERMISSION_DENIED`: No write access to directory
  - `FILE_EXISTS`: File exists and overwrite=false
  - `DISK_FULL`: Insufficient space
  - `IO_ERROR`: Other I/O errors
  
- **Side Effects**:
  - Creates or overwrites file
  - Allocates disk space
  - Updates directory modification time

- **Atomicity**: Best-effort (write to temporary, then rename if supported by OS)

**Usage**:
```cpp
auto result = write_configuration_file(
    "/home/user/config.ini",
    serialized_content,
    true  // overwrite if exists
);

if (!result.is_ok()) {
    std::cerr << "Write failed: " << result.error().message << std::endl;
}
```

---

### Function: read_configuration_file()

**Location**: `src/io/configuration_reader.h`

```cpp
Result<std::string> read_configuration_file(
    const std::string& file_path);
```

**Contract**:
- **Preconditions**:
  - `file_path` is absolute or relative path
  - File exists at path
  
- **Postconditions**:
  - On success: Returns `Result<string>` with file contents
  - On error: Returns `Result<string>` with error details
  
- **Error Cases**:
  - `PERMISSION_DENIED`: No read access
  - `IO_ERROR`: Read errors
  - `UNKNOWN`: Other errors
  
- **Side Effects**: None (read-only)

**Usage**:
```cpp
auto result = read_configuration_file("/path/to/config.ini");
if (auto content = result.value_or(nullptr)) {
    auto parsed = serializer->deserialize(*content);
}
```

---

## Save Workflow API

### High-Level Function: save_configuration()

**Location**: `src/core/save_manager.h` (to be created)

```cpp
Result<void> save_configuration(
    const ConfigurationData& config,
    const std::string& file_path,
    FormatType format,
    bool overwrite = false);
```

**Contract**:
- **Preconditions**:
  - `config` is loaded and currently valid
  - `file_path` is writable
  - `format` is JSON or INI
  
- **Postconditions**:
  - On success: File written, configuration persisted
  - On error: File unchanged, error details provided
  
- **Operations** (in order):
  1. Validate configuration against schema
  2. Create SerializationContext
  3. Get serializer from factory
  4. Call serialize()
  5. Call write_configuration_file()
  
- **Error Cases**:
  - Any validation error
  - Any serialization error
  - Any I/O error
  
- **Atomicity**: Fail-fast - stops at first error

**Usage**:
```cpp
auto result = save_configuration(
    current_config,
    "/path/to/export.ini",
    FormatType::INI,
    true  // overwrite existing
);

if (!result.is_ok()) {
    show_error_dialog(result.error().message);
}
```

---

## Error Handling API

### Type: SerializationError

**Location**: `src/core/models/serialization_result.h`

```cpp
struct SerializationError {
    enum class Code {
        INVALID_FORMAT,           // 400
        INVALID_DATA,             // 422
        ENCODING_ERROR,           // 400
        IO_ERROR,                 // 500
        PERMISSION_DENIED,        // 403
        DISK_FULL,               // 507
        FILE_EXISTS,             // 409
        UNKNOWN                  // 500
    };
    
    Code code;
    std::string message;        // User-facing message
    std::string context;        // Implementation context
    int system_errno = 0;       // System error number if applicable
};
```

**Usage**:
```cpp
if (auto error = result.error_or(nullptr)) {
    switch (error->code) {
        case SerializationError::Code::PERMISSION_DENIED:
            show_dialog("You don't have permission to write to this location");
            break;
        case SerializationError::Code::DISK_FULL:
            show_dialog("Insufficient disk space");
            break;
        default:
            show_dialog(error->message);
    }
}
```

---

## HTTP API (HTML Server)

### Endpoint: POST /api/save

**Purpose**: Save configuration in specified format (HTML server)

**Request**:
```json
{
  "config": {
    "database": { "host": "localhost", "port": 5432 },
    "features": ["logging", "auth"]
  },
  "format": "json",
  "filename": "config.json"
}
```

**Response (200 OK)**:
```json
{
  "status": "success",
  "format": "json",
  "file_size": 256,
  "saved_at": "2025-10-27T15:30:00Z"
}
```

**Response (400 Bad Request)**:
```json
{
  "status": "error",
  "error_code": "INVALID_DATA",
  "message": "Configuration validation failed",
  "details": {
    "invalid_fields": ["database.port"]
  }
}
```

**Response (403 Forbidden)**:
```json
{
  "status": "error",
  "error_code": "PERMISSION_DENIED",
  "message": "No write permission for this location"
}
```

**Response (507 Insufficient Storage)**:
```json
{
  "status": "error",
  "error_code": "DISK_FULL",
  "message": "Insufficient disk space"
}
```

---

## Thread Safety

**Thread Safety Level**: THREAD-SAFE

- Serializers are stateless and can be called from multiple threads
- Factory creates independent instances (no shared state)
- File I/O operations are serialized at OS level (standard file locking)
- Result<T> type is immutable after construction

**Recommendation**: Use thread pool for multiple concurrent saves (each in separate thread)

---

## Performance Guarantees

| Operation | Target | Notes |
|-----------|--------|-------|
| JSON serialization | < 100ms | For typical configs (< 1MB) |
| INI serialization | < 200ms | Includes flattening overhead |
| File write | < 500ms | Depends on I/O performance |
| Round-trip (JSON→INI→JSON) | < 300ms | Combined serialization time |

---

## Version Compatibility

**Current Version**: 1.0

**Breaking Changes**: None (first release)

**Backward Compatibility**: N/A (new feature)

---

## Testing Contract

All implementations must satisfy:

1. **Unit Tests**: ≥90% code coverage
2. **Integration Tests**: All workflows tested end-to-end
3. **Error Injection**: All error paths tested (I/O errors, permission denied, etc.)
4. **Round-trip**: JSON→INI→JSON produces identical validated configuration
5. **MISRA Compliance**: All code passes clang-tidy static analysis

---

## See Also

- `spec.md` - Feature specification
- `research.md` - Technical research and decisions
- `data-model.md` - Data structure definitions
