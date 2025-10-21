# Component Contract: Configuration Writer (I/O)

**Scope**: FileHandler, ConfigReader, ConfigWriter components  
**Responsibility**: Load and save configuration files in JSON and YAML formats

## Interface Definition

### Primary Component: FileHandler

**Class**: `FileHandler`  
**Location**: `src/io/config_file_handler.h/cpp`

**Purpose**: Coordinate loading/saving of configuration files in both JSON and YAML formats

#### Public Methods

```cpp
class FileHandler : public QObject {
    Q_OBJECT
    
public:
    explicit FileHandler(QObject* parent = nullptr);
    
    // Load configuration from file (auto-detect format)
    Result<ConfigurationData, FileError> loadConfig(
        const QString& filePath,
        const JSONSchema& schema);
    
    // Load configuration from JSON specifically
    Result<ConfigurationData, FileError> loadJsonConfig(
        const QString& filePath,
        const JSONSchema& schema);
    
    // Load configuration from YAML specifically
    Result<ConfigurationData, FileError> loadYamlConfig(
        const QString& filePath,
        const JSONSchema& schema);
    
    // Save configuration to file (user selects format)
    Result<void, FileError> saveConfig(
        const QString& filePath,
        const ConfigurationData& data,
        FileFormat format);
    
    // Get supported file extensions
    QStringList getFileExtensions(FileFormat format) const;
    
    // Auto-detect format from file extension
    FileFormat detectFormat(const QString& filePath) const;

signals:
    void loadProgress(int bytesRead, int totalBytes);
    void saveProgress(int itemsSaved, int totalItems);
    void loadCompleted(const ConfigurationData& data);
    void saveCompleted();

private:
    std::unique_ptr<JsonConfigReader> jsonReader_;
    std::unique_ptr<YamlConfigReader> yamlReader_;
    std::unique_ptr<ConfigWriter> configWriter_;
};
```

#### Load Contract

**Input**:
| Parameter | Type | Constraints | Error Behavior |
|-----------|------|-------------|----------------|
| filePath | QString | Must exist, readable | FileError::NotFound |
| schema | JSONSchema | Must be valid | FileError::InvalidSchema |

**Output - Success**:
```cpp
ConfigurationData {
    schemaName: schema.name,
    values: {JSON object from file},
    fieldTouched: {all fields marked as loaded},
    sourceFile: filePath,
    sourceFormat: detected format,
    lastModified: file modification time
}
```

**Output - Error**:
```cpp
enum class FileError {
    NotFound,                  // File doesn't exist
    PermissionDenied,          // Can't read file
    ReadFailed,                // I/O error reading file
    InvalidJson,               // JSON parse error
    InvalidYaml,               // YAML parse error
    InvalidSchema,             // Schema validation failed
    SchemaMismatch,            // Data doesn't match schema structure
    UnsupportedFormat,         // File extension not .json or .yaml
    FileTooLarge               // File exceeds size limit (10MB)
};
```

#### Example Load Usage

```cpp
FileHandler handler;

// Load with auto-detect format
auto loadResult = handler.loadConfig(
    "/etc/app.yaml",
    schema
);

if (loadResult.isOk()) {
    ConfigurationData data = loadResult.unwrap();
    qDebug() << "Loaded from:" << data.sourceFormat;
    // Populate form with data
} else {
    FileError error = loadResult.unwrapErr();
    if (error == FileError::SchemaMismatch) {
        // Show dialog with schema differences
    }
}
```

---

### Secondary Component: JsonConfigReader

**Class**: `JsonConfigReader`  
**Location**: `src/io/json_config_reader.h/cpp`

#### Public Methods

```cpp
class JsonConfigReader {
public:
    // Read JSON config file
    Result<nlohmann::json, FileError> readJson(
        const QString& filePath) const;
    
    // Parse JSON string
    Result<nlohmann::json, FileError> parseJson(
        const QString& jsonString) const;
    
    // Validate JSON against schema
    Result<void, FileError> validateAgainstSchema(
        const nlohmann::json& data,
        const JSONSchema& schema) const;
    
    // Get file size and modification time
    Result<QFileInfo, FileError> getFileInfo(
        const QString& filePath) const;
};
```

#### JSON Parsing Details

```cpp
// Implementation pattern
Result<nlohmann::json, FileError> readJson(const QString& filePath) {
    QFile file(filePath);
    
    if (!file.exists()) {
        return Err(FileError::NotFound);
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return Err(FileError::PermissionDenied);
    }
    
    try {
        QString content = QString::fromUtf8(file.readAll());
        auto json = nlohmann::json::parse(content.toStdString());
        
        if (!json.is_object()) {
            return Err(FileError::InvalidJson);
        }
        
        return Ok(json);
        
    } catch (const nlohmann::json::exception& e) {
        return Err(FileError::InvalidJson);
    }
}
```

---

### Tertiary Component: YamlConfigReader

**Class**: `YamlConfigReader`  
**Location**: `src/io/yaml_config_reader.h/cpp`

#### Public Methods

```cpp
class YamlConfigReader {
public:
    // Read YAML config file
    Result<nlohmann::json, FileError> readYaml(
        const QString& filePath) const;
    
    // Parse YAML string
    Result<nlohmann::json, FileError> parseYaml(
        const QString& yamlString) const;
    
    // Convert YAML node to JSON
    nlohmann::json yamlToJson(const YAML::Node& node) const;
};
```

#### YAML Parsing Details

```cpp
// Implementation pattern
Result<nlohmann::json, FileError> readYaml(const QString& filePath) {
    QFile file(filePath);
    
    if (!file.exists()) {
        return Err(FileError::NotFound);
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return Err(FileError::PermissionDenied);
    }
    
    try {
        QString content = QString::fromUtf8(file.readAll());
        YAML::Node root = YAML::Load(content.toStdString());
        
        // Convert YAML to JSON
        nlohmann::json json = yamlToJson(root);
        
        if (!json.is_object()) {
            return Err(FileError::InvalidYaml);
        }
        
        return Ok(json);
        
    } catch (const YAML::Exception& e) {
        return Err(FileError::InvalidYaml);
    }
}

// Recursive YAML to JSON conversion
nlohmann::json yamlToJson(const YAML::Node& node) {
    if (node.IsScalar()) {
        // Scalar value
        std::string value = node.as<std::string>();
        return nlohmann::json::parse(
            '"' + value + '"');  // Return as JSON string
            
    } else if (node.IsSequence()) {
        // Array
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& item : node) {
            arr.push_back(yamlToJson(item));
        }
        return arr;
        
    } else if (node.IsMap()) {
        // Object
        nlohmann::json obj = nlohmann::json::object();
        for (const auto& pair : node) {
            std::string key = pair.first.as<std::string>();
            obj[key] = yamlToJson(pair.second);
        }
        return obj;
        
    } else {
        // Null
        return nlohmann::json(nullptr);
    }
}
```

---

### Quaternary Component: ConfigWriter

**Class**: `ConfigWriter`  
**Location**: `src/io/config_writer.h/cpp`

**Purpose**: Write ConfigurationData to JSON or YAML format

#### Public Methods

```cpp
class ConfigWriter {
public:
    // Write config as JSON
    Result<void, FileError> writeJson(
        const QString& filePath,
        const nlohmann::json& data,
        int indent = 2) const;
    
    // Write config as YAML
    Result<void, FileError> writeYaml(
        const QString& filePath,
        const nlohmann::json& data) const;
    
    // Get JSON string (no file)
    Result<QString, FileError> toJsonString(
        const nlohmann::json& data,
        int indent = 2) const;
    
    // Get YAML string (no file)
    Result<QString, FileError> toYamlString(
        const nlohmann::json& data) const;
};
```

#### Save Contract

**Input**:
| Parameter | Type | Constraints |
|-----------|------|-------------|
| filePath | QString | Directory must be writable |
| data | nlohmann::json | Valid configuration object |
| format | FileFormat | JSON or YAML |

**Output - Success**:
```cpp
// File created/overwritten
// Original backup created as .bak (optional)
// Permissions preserved from original file (if existed)
```

**Output - Error**:
```cpp
enum class WriteError {
    PermissionDenied,      // Can't write to directory
    DiskFull,              // No space available
    WriteFailed,           // I/O error writing
    InvalidPath,           // Path is directory or invalid
    EncodingError          // UTF-8 encoding failed
};
```

#### JSON Writing

```cpp
// Implementation pattern
Result<void, FileError> writeJson(
    const QString& filePath,
    const nlohmann::json& data,
    int indent) const {
    
    try {
        QString jsonStr = QString::fromStdString(
            data.dump(indent)
        );
        
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return Err(FileError::PermissionDenied);
        }
        
        if (!file.write(jsonStr.toUtf8())) {
            return Err(FileError::WriteFailed);
        }
        
        file.close();
        return Ok();
        
    } catch (const std::exception& e) {
        return Err(FileError::WriteFailed);
    }
}
```

#### YAML Writing

```cpp
// Implementation pattern
Result<void, FileError> writeYaml(
    const QString& filePath,
    const nlohmann::json& data) const {
    
    try {
        YAML::Node root = jsonToYaml(data);
        
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return Err(FileError::PermissionDenied);
        }
        
        std::stringstream ss;
        ss << root;
        QString yamlStr = QString::fromStdString(ss.str());
        
        if (!file.write(yamlStr.toUtf8())) {
            return Err(FileError::WriteFailed);
        }
        
        file.close();
        return Ok();
        
    } catch (const std::exception& e) {
        return Err(FileError::WriteFailed);
    }
}
```

---

## File Format Support

### JSON Format

- **File Extensions**: `.json`
- **MIME Type**: `application/json`
- **Charset**: UTF-8
- **Indentation**: 2 spaces (configurable)
- **Preservation**: Comments lost

**Example**:
```json
{
  "appName": "MyApplication",
  "port": 8080,
  "database": {
    "host": "localhost",
    "port": 5432
  }
}
```

### YAML Format

- **File Extensions**: `.yaml`, `.yml`
- **MIME Type**: `application/x-yaml`
- **Charset**: UTF-8
- **Structure**: Indentation-based
- **Preservation**: Comments lost (v1.0)

**Example**:
```yaml
appName: MyApplication
port: 8080
database:
  host: localhost
  port: 5432
```

---

## Format Auto-Detection

```cpp
FileFormat FileHandler::detectFormat(const QString& filePath) const {
    QString ext = QFileInfo(filePath).suffix().toLower();
    
    if (ext == "json") {
        return FileFormat::JSON;
    } else if (ext == "yaml" || ext == "yml") {
        return FileFormat::YAML;
    } else {
        // Try to detect by content
        return detectFormatByContent(filePath);
    }
}

FileFormat detectFormatByContent(const QString& filePath) {
    // Read first line, check for YAML indicators (: or -)
    // Otherwise assume JSON
}
```

---

## Save Dialog & Format Selection

```cpp
// Usage in UI
void MainWindow::onSaveClick() {
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Configuration",
        "",
        "JSON files (*.json);;YAML files (*.yaml *.yml);;All Files (*)"
    );
    
    if (filePath.isEmpty()) return;
    
    // Determine format from filter selected or file extension
    FileFormat format = handler_.detectFormat(filePath);
    
    // If file doesn't exist, ask user which format to use
    if (!QFile::exists(filePath)) {
        QMessageBox::StandardButton button = 
            QMessageBox::question(this, "Format",
            "Save as YAML or JSON?",
            QMessageBox::Yes | QMessageBox::No);
        
        format = (button == QMessageBox::Yes) ?
            FileFormat::YAML : FileFormat::JSON;
    }
    
    auto result = handler_.saveConfig(filePath, data, format);
    
    if (!result.isOk()) {
        showError("Failed to save: " + result.unwrapErr().toString());
    }
}
```

---

## Error Recovery

### Load Error Handling

```cpp
// If schema mismatch, offer user options:
// 1. Cancel (don't load)
// 2. Load anyway (with validation errors highlighted)
// 3. View differences

// If file corrupted but parseable:
// Show partial data warning, allow user to fix
```

### Save Error Handling

```cpp
// Automatic backup before overwrite
QFile original(filePath);
if (original.exists()) {
    original.copy(filePath + ".bak");
}

// Atomic write (write to temp, then move)
QString tempPath = filePath + ".tmp";
writeToFile(tempPath, data);
QFile::remove(filePath);      // Remove old
QFile::rename(tempPath, filePath);  // Atomic rename
```

---

## Performance Targets

| Operation | Target | Notes |
|-----------|--------|-------|
| Load JSON (<1MB) | <100ms | File I/O + parsing |
| Load YAML (<1MB) | <200ms | Conversion to JSON |
| Save JSON | <50ms | Direct serialization |
| Save YAML | <100ms | JSON to YAML conversion |

---

## Testing Contract

### Unit Tests

- ✅ Load valid JSON → ConfigurationData returned
- ✅ Load valid YAML → ConfigurationData returned
- ✅ Load invalid JSON → FileError::InvalidJson
- ✅ Load missing file → FileError::NotFound
- ✅ Load with schema mismatch → FileError::SchemaMismatch
- ✅ Save config as JSON → File created with correct format
- ✅ Save config as YAML → File created with correct format
- ✅ Auto-detect JSON from extension → JSON reader used
- ✅ Auto-detect YAML from extension → YAML reader used

### Integration Tests

- ✅ Load JSON → Populate form → Save as YAML → Load YAML → Data unchanged
- ✅ Large file handling (5MB)
- ✅ Special characters in values (Unicode, escapes)

---

## Contract Version

**Version**: 1.0  
**Last Updated**: 2025-10-20  
**Status**: Ready for Implementation

