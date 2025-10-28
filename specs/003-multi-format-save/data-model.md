# Phase 1 Design: Data Models & Serialization Contracts

**Date**: October 27, 2025  
**Status**: Design Phase 1  
**Input**: Specification (spec.md) + Research (research.md)  

---

## Core Data Models

### 1. Format Type Enumeration

**File**: `src/core/models/format_type.h`

```cpp
#ifndef CONFIGGUI_FORMAT_TYPE_H
#define CONFIGGUI_FORMAT_TYPE_H

namespace configgui {

enum class FormatType {
    JSON,  // JSON format (RFC 7159)
    INI    // Windows INI format (dot-notation flattening)
};

// Conversion utilities
std::string format_type_to_string(FormatType format);
FormatType string_to_format_type(const std::string& str);

// File extension helpers
std::string get_file_extension(FormatType format);  // Returns ".json" or ".ini"
FormatType detect_format_from_path(const std::string& file_path);

} // namespace configgui

#endif // CONFIGGUI_FORMAT_TYPE_H
```

**Invariants**:
- Valid formats: JSON, INI only
- Extension mapping: JSON → .json, INI → .ini
- Case-insensitive path detection

---

### 2. Serialization Context

**File**: `src/core/models/serialization_context.h`

```cpp
#ifndef CONFIGGUI_SERIALIZATION_CONTEXT_H
#define CONFIGGUI_SERIALIZATION_CONTEXT_H

#include "format_type.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>

namespace configgui {

struct SerializationContext {
    // Input data
    nlohmann::json config_data;      // Current configuration (JSON format)
    FormatType target_format;         // JSON or INI
    
    // Metadata
    std::string file_path;           // Full path for save operation
    std::string schema_uri;          // Schema identifier (optional, for validation)
    
    // Output container
    std::string serialized_content;  // Populated after successful serialization
    
    // Validation
    bool validate_before_save = true;  // Always validate before serialization
    
    // Factory methods
    static SerializationContext for_json_save(
        const nlohmann::json& data,
        const std::string& path);
    
    static SerializationContext for_ini_save(
        const nlohmann::json& data,
        const std::string& path);
};

} // namespace configgui

#endif // CONFIGGUI_SERIALIZATION_CONTEXT_H
```

**Invariants**:
- `config_data` must be valid JSON (pre-validation)
- `target_format` must be JSON or INI
- `file_path` must be non-empty for actual save operations
- `serialized_content` populated only after successful serialization

---

### 3. Serialization Result Type

**File**: `src/core/models/serialization_result.h` (extends existing Result<T>)

```cpp
#ifndef CONFIGGUI_SERIALIZATION_RESULT_H
#define CONFIGGUI_SERIALIZATION_RESULT_H

#include "../result.h"  // Existing ConfigGUI Result<T> type
#include <string>

namespace configgui {

// Error type for serialization operations
struct SerializationError {
    enum class Code {
        INVALID_FORMAT,           // Unsupported format
        INVALID_DATA,             // Data doesn't match schema
        ENCODING_ERROR,           // Character encoding issue
        IO_ERROR,                 // File system error
        PERMISSION_DENIED,        // Write access denied
        DISK_FULL,               // Insufficient space
        FILE_EXISTS,             // File exists (overwrite needed)
        UNKNOWN                  // Other errors
    };
    
    Code code;
    std::string message;        // Human-readable error description
    std::string context;        // Additional context (file path, line number, etc.)
};

using SerializationResult = Result<std::string, SerializationError>;

} // namespace configgui

#endif // CONFIGGUI_SERIALIZATION_RESULT_H
```

**Error Mapping**:
- `INVALID_FORMAT`: Unsupported format requested
- `INVALID_DATA`: Configuration fails schema validation
- `ENCODING_ERROR`: Special characters not representable in target format
- `IO_ERROR`: File system read/write errors
- `PERMISSION_DENIED`: Insufficient write access
- `DISK_FULL`: No space available
- `FILE_EXISTS`: File exists without overwrite flag

---

### 4. Format Serializer Base Class

**File**: `src/core/serializers/format_serializer.h`

```cpp
#ifndef CONFIGGUI_FORMAT_SERIALIZER_H
#define CONFIGGUI_FORMAT_SERIALIZER_H

#include "../models/serialization_context.h"
#include "../models/serialization_result.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>

namespace configgui {

class FormatSerializer {
public:
    virtual ~FormatSerializer() = default;
    
    /**
     * Serialize configuration to target format string
     * @param context Serialization context with config data and metadata
     * @return Result<string> containing serialized content or error
     */
    virtual SerializationResult serialize(
        const SerializationContext& context) = 0;
    
    /**
     * Deserialize from format string back to JSON
     * @param content Serialized content in target format
     * @return Result<nlohmann::json> containing parsed JSON or error
     */
    virtual Result<nlohmann::json> deserialize(
        const std::string& content) = 0;
    
    /**
     * Get MIME type for this format
     * @return MIME type string (e.g., "application/json", "text/plain")
     */
    virtual std::string get_mime_type() const = 0;
    
    /**
     * Get human-readable format name
     * @return Format name (e.g., "JSON", "INI")
     */
    virtual std::string get_format_name() const = 0;
    
    /**
     * Validate format constraints before serialization
     * @param context Serialization context
     * @return Optional error if validation fails
     */
    virtual std::optional<SerializationError> validate_pre_serialization(
        const SerializationContext& context) const = 0;
};

using FormatSerializerPtr = std::unique_ptr<FormatSerializer>;

} // namespace configgui

#endif // CONFIGGUI_FORMAT_SERIALIZER_H
```

**Interface Contract**:
- Subclasses implement format-specific serialization logic
- All methods provide clear error semantics (Result<T>)
- Pre-serialization validation prevents invalid operations

---

### 5. JSON Serializer Implementation

**File**: `src/core/serializers/json_serializer.h` (excerpt)

```cpp
#ifndef CONFIGGUI_JSON_SERIALIZER_H
#define CONFIGGUI_JSON_SERIALIZER_H

#include "format_serializer.h"
#include <nlohmann/json.hpp>

namespace configgui {

class JsonSerializer : public FormatSerializer {
public:
    SerializationResult serialize(
        const SerializationContext& context) override;
    
    Result<nlohmann::json> deserialize(
        const std::string& content) override;
    
    std::string get_mime_type() const override { return "application/json"; }
    std::string get_format_name() const override { return "JSON"; }
    
    std::optional<SerializationError> validate_pre_serialization(
        const SerializationContext& context) const override;

private:
    // Helper methods for JSON-specific operations
    std::string pretty_print(const nlohmann::json& data) const;
};

} // namespace configgui

#endif // CONFIGGUI_JSON_SERIALIZER_H
```

**Implementation Notes**:
- Wraps existing nlohmann_json library
- Pretty-prints JSON for human readability
- Validates JSON structure (no undefined values)
- Preserves number precision (no float rounding)

---

### 6. INI Serializer Implementation

**File**: `src/core/serializers/ini_serializer.h` (excerpt)

```cpp
#ifndef CONFIGGUI_INI_SERIALIZER_H
#define CONFIGGUI_INI_SERIALIZER_H

#include "format_serializer.h"
#include <nlohmann/json.hpp>
#include <string>
#include <map>

namespace configgui {

class IniSerializer : public FormatSerializer {
public:
    SerializationResult serialize(
        const SerializationContext& context) override;
    
    Result<nlohmann::json> deserialize(
        const std::string& content) override;
    
    std::string get_mime_type() const override { return "text/plain"; }
    std::string get_format_name() const override { return "INI"; }
    
    std::optional<SerializationError> validate_pre_serialization(
        const SerializationContext& context) const override;

private:
    // Flattening: JSON object hierarchy → dot-notation keys
    void flatten_json(
        const nlohmann::json& json,
        const std::string& prefix,
        std::map<std::string, std::string>& result);
    
    // Unflattening: dot-notation keys → JSON object hierarchy
    nlohmann::json unflatten_ini(
        const std::map<std::string, std::string>& ini_map);
    
    // Format-specific helpers
    std::string escape_ini_value(const std::string& value) const;
    std::string unescape_ini_value(const std::string& value) const;
    bool is_array_index(const std::string& key) const;
    
    // mINI library integration
    std::string serialize_to_ini_string(
        const std::map<std::string, std::string>& flat_map) const;
    
    std::map<std::string, std::string> parse_ini_string(
        const std::string& content) const;
};

} // namespace configgui

#endif // CONFIGGUI_INI_SERIALIZER_H
```

**Implementation Details**:

**Flattening Algorithm**:
```
Input:  { "db": { "host": "localhost", "port": 5432 } }
Output: { "db.host": "localhost", "db.port": "5432" }

Input:  { "servers": ["web", "api", "db"] }
Output: { "servers.0": "web", "servers.1": "api", "servers.2": "db" }
```

**INI Output Format**:
```ini
[configuration]
db.host=localhost
db.port=5432
servers.0=web
servers.1=api
servers.2=db
```

**Escaping Rules**:
- Preserve `[a-zA-Z0-9._]` characters as-is
- Escape spaces and special chars: `\s` for space, `\n` for newline, etc.
- Empty values represented as empty string: `key=`

---

### 7. Serializer Factory

**File**: `src/core/serializers/serializer_factory.h`

```cpp
#ifndef CONFIGGUI_SERIALIZER_FACTORY_H
#define CONFIGGUI_SERIALIZER_FACTORY_H

#include "format_serializer.h"
#include "../models/format_type.h"
#include <memory>

namespace configgui {

class SerializerFactory {
public:
    /**
     * Create serializer for specified format
     * @param format JSON or INI
     * @return Unique pointer to format-specific serializer
     * @throws std::invalid_argument if format is unsupported
     */
    static FormatSerializerPtr create_serializer(FormatType format);
    
    /**
     * Create serializer based on file extension
     * @param file_path Path with .json or .ini extension
     * @return Unique pointer to appropriate serializer
     * @throws std::invalid_argument if extension not recognized
     */
    static FormatSerializerPtr create_serializer_from_path(
        const std::string& file_path);
};

} // namespace configgui

#endif // CONFIGGUI_SERIALIZER_FACTORY_H
```

**Usage Pattern**:
```cpp
auto serializer = SerializerFactory::create_serializer(FormatType::INI);
auto result = serializer->serialize(context);
if (result.is_ok()) {
    write_to_file(result.value());
} else {
    handle_error(result.error());
}
```

---

## GUI Integration Points

### Qt GUI: Format Selection Dialog

**File**: `src/ui/format_selection_dialog.h`

```cpp
#ifndef CONFIGGUI_FORMAT_SELECTION_DIALOG_H
#define CONFIGGUI_FORMAT_SELECTION_DIALOG_H

#include <QDialog>
#include "../core/models/format_type.h"

class QRadioButton;
class QLabel;

class FormatSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit FormatSelectionDialog(QWidget* parent = nullptr);
    
    // Get selected format (valid only if exec() returned Accepted)
    configgui::FormatType selected_format() const;
    
    // Set default format
    void set_default_format(configgui::FormatType format);

private slots:
    void on_format_changed();

private:
    void setup_ui();
    void connect_signals();
    
    QRadioButton* json_radio_;
    QRadioButton* ini_radio_;
    QLabel* format_description_;
};

#endif // CONFIGGUI_FORMAT_SELECTION_DIALOG_H
```

### HTML GUI: Format Selection Form

**File**: `resources/html/format-selection.html`

```html
<div id="formatSelectionModal" class="modal hidden">
  <div class="modal-content">
    <h2>Save Configuration</h2>
    <p>Select the file format for saving your configuration:</p>
    
    <div class="format-options">
      <label class="format-option">
        <input type="radio" name="format" value="json" checked>
        <span class="format-name">JSON</span>
        <span class="format-description">
          Human-readable format, widely supported
        </span>
      </label>
      
      <label class="format-option">
        <input type="radio" name="format" value="ini">
        <span class="format-name">INI</span>
        <span class="format-description">
          Legacy format for Windows and enterprise systems
        </span>
      </label>
    </div>
    
    <div class="modal-actions">
      <button id="saveBtnFormat" class="btn btn-primary">Save</button>
      <button id="cancelBtnFormat" class="btn btn-secondary">Cancel</button>
    </div>
  </div>
</div>
```

**JavaScript Handler** (`resources/html/format-selection.js`):

```javascript
class FormatSelector {
    constructor() {
        this.modal = document.getElementById('formatSelectionModal');
        this.selectedFormat = 'json';
    }
    
    async show() {
        return new Promise((resolve) => {
            const saveBtn = document.getElementById('saveBtnFormat');
            const cancelBtn = document.getElementById('cancelBtnFormat');
            
            saveBtn.onclick = () => {
                this.selectedFormat = document.querySelector(
                    'input[name="format"]:checked'
                ).value;
                this.modal.classList.add('hidden');
                resolve(this.selectedFormat);
            };
            
            cancelBtn.onclick = () => {
                this.modal.classList.add('hidden');
                resolve(null);
            };
            
            this.modal.classList.remove('hidden');
        });
    }
    
    getSelectedFormat() {
        return this.selectedFormat;
    }
}
```

---

## Data Flow Diagrams

### Save Workflow: Qt Application

```
User clicks Save
    ↓
Show Format Selection Dialog (QDialog)
    ↓
User selects format (JSON or INI)
    ↓
Show File Save Dialog (native)
    ↓
User confirms file path
    ↓
Validate current config against schema
    ↓
Create SerializationContext
    ↓
Get appropriate serializer (Factory)
    ↓
Serialize to target format
    ↓
Write to file (handle errors)
    ↓
Show success/error message
```

### Save Workflow: HTML Application

```
User clicks Save
    ↓
Show Format Selection Modal (HTML/JS)
    ↓
User selects format (JSON or INI)
    ↓
POST /api/save with config + format
    ↓
Server validates config against schema
    ↓
Server serializes to target format
    ↓
Server responds with download
    ↓
Browser triggers file download
    ↓
Show success/error message
```

---

## API Contracts & Endpoints

### Core Library API (C++)

**Primary Function**:
```cpp
SerializationResult serialize_configuration(
    const nlohmann::json& config,
    FormatType format,
    const std::string& optional_schema_uri = "");
```

**Parameters**:
- `config`: Valid JSON configuration data
- `format`: JSON or INI
- `optional_schema_uri`: Schema identifier for validation (optional)

**Returns**:
- Success: Serialized content as std::string
- Error: SerializationError with code and message

### HTTP API (HTML Server)

**POST /api/save**

Request:
```json
{
  "config": { /* configuration object */ },
  "format": "json" or "ini",
  "filename": "config.json" or "config.ini"
}
```

Response (Success 200):
```json
{
  "status": "success",
  "file_size": 1024,
  "format": "json"
}
```

Response (Error 400):
```json
{
  "status": "error",
  "error_code": "INVALID_DATA",
  "message": "Configuration validation failed",
  "details": "..."
}
```

---

## Validation & Constraints

### Pre-Serialization Validation

All serializers enforce:
1. Configuration is valid JSON
2. Configuration passes schema validation
3. Target format can represent the data (or provide clear error)
4. File path is writable

### Data Type Constraints

| Type | JSON | INI | Notes |
|------|------|-----|-------|
| String | ✅ | ✅ | Escaping applied for special chars |
| Number | ✅ | ✅ (as string) | Integer and float both supported |
| Boolean | ✅ | ✅ (true/false) | No compatibility issues |
| Null | ✅ | ✅ (empty value) | Represented as empty key |
| Array | ✅ | ✅ (indexed) | Converted to `.0`, `.1`, etc. |
| Object | ✅ | ✅ (flattened) | Converted to dot-notation |

### Round-Trip Guarantees

**JSON → JSON**: 100% preservation  
**JSON → INI → JSON**: 100% preservation (with flattening/unflattening)  
**INI → JSON → INI**: ✅ Valid (may differ in whitespace/formatting but semantically identical)

---

## Extension Points

### Custom Serializers

To add new format (e.g., YAML, TOML):

1. Inherit from `FormatSerializer`
2. Implement all virtual methods
3. Register in `SerializerFactory::create_serializer()`
4. Add format type to `FormatType` enum

**Example**: Adding YAML support

```cpp
class YamlSerializer : public FormatSerializer {
    SerializationResult serialize(...) override;
    Result<nlohmann::json> deserialize(...) override;
    // ... other overrides
};

// In factory:
case FormatType::YAML:
    return std::make_unique<YamlSerializer>();
```

---

## Summary

**Models Defined**: 7 (FormatType, SerializationContext, SerializationResult, FormatSerializer base class, JsonSerializer, IniSerializer, SerializerFactory)

**GUI Integration**: Qt dialog + HTML modal with JavaScript handler

**Data Flows**: Clear serialization workflows for both applications

**Extension Ready**: Factory pattern enables new format support

**Ready for Phase 2**: ✅ All data models and contracts defined, ready for task breakdown
