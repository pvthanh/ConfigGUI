# Phase 0: Research & Technology Decisions

**Date**: 2025-10-20  
**Feature**: Schema-Driven Configuration GUI  
**Status**: Complete

## Overview

This document consolidates research and decision-making for critical technology choices and implementation strategies for ConfigGUI.

---

## 1. JSON Schema Validator Library

### Decision: nlohmann/json-schema

**Selected**: nlohmann/json-schema v3.x (header-only C++ implementation)

### Rationale

- **Draft 7 Support**: Full JSON Schema Draft 7 compliance (our selected version)
- **C++17 Compatible**: Aligns with project language requirement
- **Header-Only**: Simplifies integration and build configuration
- **Performance**: Efficient validation without external dependencies
- **MISRA Compatible**: No problematic language constructs
- **Community**: Well-maintained, widely used in Qt/C++ projects
- **Integration**: Works well with nlohmann/json library (also using for JSON parsing)

### Alternatives Considered

1. **AJV (Another JSON Schema Validator)** 
   - Reason Rejected: JavaScript-based, not suitable for C++ desktop app
   
2. **Everit JSON Schema** 
   - Reason Rejected: Java-based, incompatible with C++ requirement
   
3. **Custom Validator Implementation**
   - Reason Rejected: High complexity, maintenance burden, bug risk, time-intensive

### Implementation Strategy

```cpp
// Core validation pattern
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

class SchemaValidator {
    nlohmann::json_schema::json_validator validator_;
    
public:
    Result<void, ValidationError> validate(const nlohmann::json& data) {
        try {
            validator_.validate(data);
            return Ok();
        } catch (const nlohmann::json_schema::validation_error& e) {
            return Err(ValidationError{e.what(), e.path()});
        }
    }
};
```

### Integration Points

- Schema loading: Load JSON schema → create nlohmann json-schema validator
- Real-time validation: Field changes → validate data against schema
- Error reporting: Validation errors → user-friendly messages in UI

---

## 2. YAML Parser Library

### Decision: libyaml with C++ bindings (yaml-cpp)

**Selected**: yaml-cpp v0.7.0+ (C++ wrapper around libyaml)

### Rationale

- **Mature Implementation**: libyaml is production-proven (used in system utilities)
- **C++ Bindings**: yaml-cpp provides idiomatic C++ interface
- **MISRA Compatible**: No problematic constructs in wrapper
- **Round-Trip Capable**: Can preserve YAML structure/comments if needed
- **Performance**: Reasonable performance for configuration files
- **Integration**: Works well with nlohmann/json for data model conversion

### Alternatives Considered

1. **Rapid YAML (ryml)**
   - Reason Rejected: Newer library, less community adoption, unproven in production
   
2. **pugixml for YAML**
   - Reason Rejected: XML library, not ideal for YAML parsing
   
3. **Manual YAML parsing**
   - Reason Rejected: High complexity, error-prone, maintenance burden

### Implementation Strategy

```cpp
#include <yaml-cpp/yaml.h>

class YamlConfigReader {
public:
    Result<ConfigurationData, FileError> readYaml(const QString& filePath) {
        try {
            YAML::Node root = YAML::LoadFile(filePath.toStdString());
            // Convert YAML to JSON for validation
            nlohmann::json data = yamlToJson(root);
            return Ok(ConfigurationData{data});
        } catch (const YAML::Exception& e) {
            return Err(FileError{e.what()});
        }
    }
    
private:
    nlohmann::json yamlToJson(const YAML::Node& node) {
        // Recursive conversion logic
    }
};
```

### Integration Points

- File loading: YAML file → parse → convert to JSON → validate against schema
- File saving: JSON data → convert to YAML format → write to file
- Format detection: File extension (.yaml, .yml) determines parser

---

## 3. Dynamic Widget Generation Strategy

### Decision: Schema-to-Qt Widget Factory Pattern

**Pattern**: Factory Method + Strategy Pattern

### Type Mapping

| JSON Schema Type | Qt Widget | Constraints Mapped |
|-----------------|-----------|-------------------|
| string | QLineEdit | min/maxLength, pattern |
| number | QDoubleSpinBox | minimum, maximum, multipleOf |
| integer | QSpinBox | minimum, maximum, multipleOf |
| boolean | QCheckBox | N/A |
| enum | QComboBox | enum values |
| array (with items) | QListWidget/QTableWidget | maxItems, minItems |
| object (nested) | QGroupBox + nested widgets | additionalProperties |

### Implementation Strategy

```cpp
class WidgetFactory {
public:
    std::unique_ptr<QWidget> createWidget(
        const nlohmann::json& schemaField,
        QWidget* parent = nullptr) {
        
        std::string type = schemaField["type"].get<std::string>();
        
        if (type == "string") {
            return createStringWidget(schemaField, parent);
        } else if (type == "integer") {
            return createIntegerWidget(schemaField, parent);
        } else if (type == "boolean") {
            return createBooleanWidget(schemaField, parent);
        }
        // ... additional types
        
        return nullptr;
    }
    
private:
    std::unique_ptr<QWidget> createStringWidget(
        const nlohmann::json& schema, 
        QWidget* parent) {
        
        auto widget = std::make_unique<QLineEdit>(parent);
        
        // Apply constraints
        if (schema.contains("minLength")) {
            widget->setMinimumWidth(200);
        }
        if (schema.contains("pattern")) {
            QValidator* validator = new QRegularExpressionValidator(
                QRegularExpression(schema["pattern"].get<std::string>().c_str()),
                widget.get()
            );
            widget->setValidator(validator);
        }
        
        return widget;
    }
};
```

### Challenges & Solutions

**Challenge**: Schema types may not map 1-to-1 to Qt widgets  
**Solution**: Use custom widgets for complex types (e.g., QDateEdit for date strings, QFileDialog for file paths with custom x-* hints)

**Challenge**: Nested objects and arrays  
**Solution**: Use QGroupBox for objects, QTableWidget for arrays with automatic widget factory recursion

### Extensibility

Plugin architecture allows adding custom widget types:

```cpp
class WidgetRendererPlugin {
public:
    virtual ~WidgetRendererPlugin() = default;
    virtual bool canRender(const nlohmann::json& schema) = 0;
    virtual std::unique_ptr<QWidget> render(
        const nlohmann::json& schema, 
        QWidget* parent) = 0;
};
```

---

## 4. Real-Time Validation Implementation

### Decision: Qt Signal/Slot Pattern with Deferred Validation

**Pattern**: Model → Validate → Update UI (Observer Pattern)

### Implementation Strategy

```cpp
class FormGenerator : public QObject {
    Q_OBJECT
    
private:
    QMap<QString, QWidget*> fieldWidgets_;
    QTimer validationTimer_;
    
public slots:
    void onFieldChanged(const QString& fieldName) {
        // Debounce: delay validation to avoid excessive checks
        validationTimer_.stop();
        validationTimer_.start(100); // 100ms delay
    }
    
private slots:
    void performValidation() {
        // Collect current values from all widgets
        nlohmann::json currentData = collectFormData();
        
        // Validate against schema
        auto result = validator_.validate(currentData);
        
        // Update UI with errors
        updateValidationUI(result);
    }
    
private:
    void updateValidationUI(const ValidationResult& result) {
        for (const auto& error : result.errors) {
            QWidget* widget = fieldWidgets_[error.field];
            if (widget) {
                widget->setStyleSheet("border: 1px solid red;");
                // Show error tooltip
                widget->setToolTip(error.message);
            }
        }
    }
};
```

### Performance Optimization

- **Debouncing**: 100ms delay between field changes and validation reduces CPU usage
- **Incremental Validation**: Validate individual fields first, then full schema
- **Lazy Loading**: Only validate visible fields for large forms

### Challenges & Solutions

**Challenge**: Qt signals fire continuously during editing  
**Solution**: Use QTimer to debounce validation (100ms)

**Challenge**: Validation blocks UI thread  
**Solution**: Use QThread for long validation operations on large configs

---

## 5. YAML Support Integration

### Decision: Unified Data Model Approach

**Strategy**: All internal processing uses JSON; YAML conversion at I/O boundaries

### Conversion Pipeline

```
YAML File → YAML Parser → Intermediate JSON → Schema Validation → ConfigurationData
ConfigurationData → Select Output Format (JSON/YAML) → Write File
```

### Implementation Strategy

```cpp
class FileHandler {
public:
    // Reading: Auto-detect format from extension
    Result<ConfigurationData, FileError> loadConfig(const QString& filePath) {
        if (filePath.endsWith(".yaml") || filePath.endsWith(".yml")) {
            return loadYamlConfig(filePath);
        } else if (filePath.endsWith(".json")) {
            return loadJsonConfig(filePath);
        }
        return Err(FileError{"Unsupported file format"});
    }
    
    // Writing: User selects format
    Result<void, FileError> saveConfig(
        const QString& filePath,
        const ConfigurationData& data,
        FileFormat format) {
        
        switch (format) {
            case FileFormat::JSON:
                return saveJsonConfig(filePath, data);
            case FileFormat::YAML:
                return saveYamlConfig(filePath, data);
        }
        return Err(FileError{"Unsupported format"});
    }
};
```

### YAML Structure Preservation

For v1.0, YAML is treated as equivalent to JSON (structure not preserved):
- YAML comments are discarded
- YAML anchors/aliases are expanded
- Result: Clean, normalized configuration data

**Future Enhancement** (v1.1): Add YAML-preserving mode if users request it

### Considerations

- **Round-Trip**: Loading YAML → JSON → saving as YAML may lose formatting
- **Compatibility**: JSON valid in YAML, so all JSON configs work in YAML too
- **User Experience**: Save dialog shows format selector; default respects input format

---

## 6. MISRA C++ Compliance Strategy

### Decision: Enforce via clang-tidy + Documented Exceptions

**Approach**: Automated checking with manual review of violations

### Implementation Strategy

#### .clang-tidy Configuration

```yaml
Checks: 'modernize-*,readability-*,clang-analyzer-*,-modernize-use-trailing-return-type'
HeaderFilterRegex: '^src/.*\.h$'
WarningsAsErrors: 'portability-*,readability-identifier-naming'
```

#### MISRA C++ Violations Handling

| Rule | How Handled | Status |
|------|------------|--------|
| No raw pointers | Use std::unique_ptr, QSharedPointer | Enforced |
| No C-style casts | Use static_cast, dynamic_cast, Qt casts | Enforced |
| const-correctness | Const reference parameters, const methods | Enforced |
| No variable-length arrays | Use std::vector, std::array | Enforced |
| No unions | Use std::variant if needed | Enforced |
| Error handling | Use Result<T> pattern, no bare exceptions | Enforced |

#### Qt Signal/Slot Exception

Qt's signal/slot mechanism requires some deviations from strict MISRA:
- **Issue**: Qt uses unsafe casting and void pointers internally
- **Mitigation**: Isolate signal/slot connections in separate translation units with exception markers
- **Documentation**: Each exception documented with `// MISRA C++ Exception: [reason]`

Example:

```cpp
// File: ui/signal_adapters.cpp (MISRA Exception Zone)
// MISRA C++ Exception: Qt signal/slot mechanism requires dynamic_cast

connect(widget, &QLineEdit::textChanged,
        this, [this](const QString& text) {
            onFieldChanged(text);
        });
```

#### CI/CD Integration

```bash
# Pre-commit hook
clang-tidy src/**/*.cpp -- -std=c++17 -I/usr/include/qt6 | grep -v "MISRA"
# Returns non-zero if violations found (except documented exceptions)
```

### Compliance Checklist

```cpp
// Code Review Checklist for MISRA C++
// - [ ] All new pointers use smart pointers
// - [ ] No C-style casts (check for: (type*), (type), etc.)
// - [ ] All parameters marked const where appropriate
// - [ ] No VLAs (use std::vector)
// - [ ] Error handling uses Result<T> pattern
// - [ ] clang-tidy passes without violations
// - [ ] Exceptions documented with MISRA marker
```

---

## 7. Additional Technology Choices

### Build System: CMake 3.20+

**Rationale**: 
- Qt 6 standard build system
- Cross-platform (Linux, Windows, macOS)
- Good integration with clang-tidy
- Version control friendly

**Configuration**:
```cmake
cmake_minimum_required(VERSION 3.20)
project(ConfigGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(yaml-cpp REQUIRED)
```

### Testing: Google Test + Qt Test

**Rationale**:
- Google Test: Unit tests for core logic (validators, models)
- Qt Test: Widget and UI integration tests
- Comprehensive coverage of business logic

### Code Formatting: clang-format

**Rationale**:
- Industry standard C++ formatter
- Compatible with MISRA checking
- Automated via pre-commit hooks

### Documentation: Doxygen

**Rationale**:
- Standard C++ documentation tool
- Auto-generates from code comments
- HTML and PDF output

---

## Summary of Decisions

| Technology | Choice | Rationale | Alternatives |
|-----------|--------|-----------|--------------|
| Validator | nlohmann/json-schema | Draft 7, C++17, header-only | Custom impl, commercial libs |
| YAML Parser | yaml-cpp | Mature, C++ bindings | ryml, manual parsing |
| Widget Gen | Factory + Strategy | Extensible, maintainable | Custom code generator |
| Validation | Qt Signals + Debounce | Non-blocking, responsive | Async threads |
| YAML Support | JSON-normalized | Simple, effective for v1.0 | YAML-preserving mode |
| MISRA | clang-tidy enforcement | Automated, consistent | Manual review only |
| Build | CMake 3.20+ | Qt 6 standard | qmake, other build systems |

---

## Next Steps (Phase 1)

1. ✅ Generate data-model.md with entity definitions
2. ✅ Create contracts/ directory with component interfaces
3. ✅ Create quickstart.md with developer setup
4. ✅ Update agent context for implementation

