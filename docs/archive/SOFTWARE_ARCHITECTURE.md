# ConfigGUI Software Architecture

**Version**: 1.0  
**Date**: 2024-10-26  
**Status**: Production Ready

---

## Table of Contents

1. [Overview](#overview)
2. [Core Architecture](#core-architecture)
3. [Dual-Target Design](#dual-target-design)
4. [Module Organization](#module-organization)
5. [Build System](#build-system)
6. [Technology Stack](#technology-stack)
7. [Data Flow](#data-flow)
8. [Key Design Patterns](#key-design-patterns)

---

## Overview

ConfigGUI is a schema-driven configuration management tool that provides both Qt GUI (desktop) and HTTP Server (web) interfaces. It allows users to load JSON schemas, generate dynamic forms, and manage configuration files with validation.

**Key Features:**
- ✅ Dual-target: Qt GUI desktop app + HTTP web server
- ✅ Schema-driven: Auto-generates forms from JSON schemas
- ✅ MISRA C++17 compliant core library
- ✅ Support for complex nested objects and arrays
- ✅ JSON and YAML file I/O
- ✅ Form state management and validation

---

## Core Architecture

### Three-Layer Architecture

```
┌─────────────────────────────────────────────────┐
│  Presentation Layer                             │
├─────────────────────────────────────────────────┤
│  Qt GUI Application  │  HTML Web Server         │
├─────────────────────────────────────────────────┤
│  UI/Presentation Layer                          │
├─────────────────────────────────────────────────┤
│  Business Logic (Form Generation, Validation)  │
├─────────────────────────────────────────────────┤
│  Core Library (No Qt dependency)                │
│  - Schema loading & validation                  │
│  - Data serialization                           │
│  - I/O operations                               │
├─────────────────────────────────────────────────┤
│  External Dependencies                          │
│  - nlohmann/json, json-schema-validator, libyaml
└─────────────────────────────────────────────────┘
```

### Key Principle: Qt-Free Core

The `ConfigGUICore` static library contains all business logic **without Qt dependencies**. This enables:
- ✅ Reusable across platforms
- ✅ HTML server doesn't need Qt
- ✅ Easy testing without GUI
- ✅ Clean separation of concerns

---

## Dual-Target Design

### Conditional Compilation

```cmake
# CMakeLists.txt root configuration
option(BUILD_QT_APP "Build Qt GUI application" ON)
option(BUILD_HTML_SERVER "Build HTTP web server" ON)

# Both targets use the shared core library
ConfigGUICore (static)
    ↓
├── ConfigGUI (Qt application)
│   └── Optional: Qt6 Core, Gui, Widgets
│
└── ConfigGUIServer (HTTP server)
    └── Optional: cpp-httplib (header-only)
```

### Build Scenarios

**Scenario 1: Full Build (Default)**
```bash
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON ..
# Produces: ConfigGUI + ConfigGUIServer + ConfigGUICore
```

**Scenario 2: Qt-Only**
```bash
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF ..
# Produces: ConfigGUI + ConfigGUICore
# Qt is required
```

**Scenario 3: Server-Only (Docker)**
```bash
cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON ..
# Produces: ConfigGUIServer + ConfigGUICore
# Qt NOT required - enables containerization
```

---

## Module Organization

### Directory Structure

```
configGui/
├── src/
│   ├── core/                    # Shared business logic (NO Qt)
│   │   ├── schema.{h,cpp}
│   │   ├── configuration_data.{h,cpp}
│   │   ├── validation_error.{h,cpp}
│   │   ├── schema_loader.{h,cpp}
│   │   ├── schema_validator.{h,cpp}
│   │   ├── form_state.{h,cpp}
│   │   ├── result.h             # Result<T> for error handling
│   │   └── error_types.h
│   │
│   ├── ui/                      # Qt-specific UI widgets
│   │   ├── main_window.{h,cpp}
│   │   ├── form_generator.{h,cpp}
│   │   ├── widget_factory.{h,cpp}
│   │   ├── object_array_widget.{h,cpp}
│   │   ├── array_widget.{h,cpp}
│   │   ├── dictionary_widget.{h,cpp}
│   │   └── ... (other Qt widgets)
│   │
│   ├── html/                    # HTTP server implementation
│   │   ├── server/
│   │   └── handlers/
│   │
│   ├── validators/              # Schema validation
│   ├── io/                       # File I/O (JSON, YAML)
│   └── utils/                    # Utility functions
│
├── resources/
│   ├── schemas/                 # Example JSON schemas
│   ├── configs/                 # Example configurations
│   └── html/                    # Web assets (HTML/CSS/JS)
│
├── tests/
│   ├── unit/
│   ├── integration/
│   └── html/
│
└── CMakeLists.txt               # Root configuration
```

### Core Library Modules

| Module | Purpose | Key Classes |
|--------|---------|------------|
| **Schema** | Schema parsing and manipulation | `JSONSchema`, `SchemaProperty` |
| **Configuration Data** | Runtime data representation | `ConfigurationData` |
| **Validation** | Schema validation logic | `SchemaValidator`, `ValidationError` |
| **I/O** | File reading/writing | `JsonReader`, `JsonWriter`, `YamlReader`, `YamlWriter` |
| **Form State** | Form data management | `FormState` |

### UI Layer Modules (Qt Only)

| Module | Purpose |
|--------|---------|
| **Form Generator** | Creates form widgets from schema |
| **Widget Factory** | Creates specialized widgets by type |
| **Object Array Widget** | Handles arrays of complex objects |
| **Dictionary Widget** | Handles key-value maps |
| **Array Widget** | Handles simple arrays |
| **Main Window** | Application window and menu bar |

---

## Build System

### CMake Architecture

**Root CMakeLists.txt:**
- Global options (BUILD_QT_APP, BUILD_HTML_SERVER)
- Dependency resolution
- Subdirectory management

**Module CMakeLists.txt:**
- Independent target configuration
- Local dependencies
- Source file management

### Dependency Management

**External Dependencies:**
```cmake
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(json-schema-validator REQUIRED)
find_package(yaml-cpp REQUIRED)

# Optional: Qt6 (only if BUILD_QT_APP=ON)
if(BUILD_QT_APP)
    find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
endif()

# Optional: cpp-httplib (via FetchContent if BUILD_HTML_SERVER=ON)
if(BUILD_HTML_SERVER)
    FetchContent_Declare(cpp-httplib ...)
    FetchContent_MakeAvailable(cpp-httplib)
endif()
```

### Compiler Flags

All modules use MISRA C++17 compliance:
```cmake
-Wall -Wextra -Werror -Wpedantic
-std=c++17
```

---

## Technology Stack

### Core Libraries

| Library | Version | Purpose |
|---------|---------|---------|
| **nlohmann/json** | 3.2.0+ | JSON parsing & manipulation |
| **json-schema-validator** | Latest | JSON Schema validation |
| **yaml-cpp** | Latest | YAML file I/O |
| **C++** | C++17 | Language |

### Qt Application

| Component | Version | Purpose |
|-----------|---------|---------|
| **Qt** | 6.x | GUI framework |
| **Qt Core** | 6.x | Core functionality |
| **Qt Gui** | 6.x | Graphics & events |
| **Qt Widgets** | 6.x | UI components |
| **Qt Test** | 6.x | Unit testing |

### Web Server

| Library | Version | Purpose |
|---------|---------|---------|
| **cpp-httplib** | 0.11.0+ | HTTP server (header-only) |
| **HTML5/CSS3** | Current | Client interface |
| **JavaScript** | ES6 | Client-side logic |

---

## Data Flow

### Form Generation Flow

```
JSON Schema
    ↓
SchemaLoader (parse JSON)
    ↓
JSONSchema (structured representation)
    ↓
SchemaValidator (validation)
    ↓
FormGenerator (Qt) / HTML Generator (Web)
    ↓
UI Widgets / HTML Form
    ↓
User Input
    ↓
FormState (collect data)
    ↓
ConfigurationData (serialize)
    ↓
JSON/YAML Output
```

### Configuration Save Flow

```
User modifies form
    ↓
FormState captures changes
    ↓
Data collection (recursive for nested objects)
    ↓
SchemaValidator validates data
    ↓
JsonWriter / YamlWriter serializes
    ↓
File I/O
    ↓
config.json / config.yaml
```

---

## Key Design Patterns

### 1. Result<T> Pattern (Error Handling)

```cpp
// Core library uses Result<T> instead of exceptions
Result<JSONSchema, FileError> result = SchemaLoader::load("schema.json");

if (result.is_ok()) {
    JSONSchema schema = result.value();
    // Process schema
} else {
    FileError error = result.error();
    // Handle error
}
```

**Benefits:**
- No exceptions in core library
- Explicit error handling
- MISRA C++17 compliant

### 2. Factory Pattern (Widget Creation)

```cpp
// WidgetFactory creates appropriate widgets by schema type
WidgetFactory factory;
QWidget* widget = factory.createWidget(field_schema);

// Handles:
// - Enums → QComboBox
// - Numbers with range → QSpinBox/QDoubleSpinBox
// - Arrays of objects → ObjectArrayWidget
// - Key-value maps → DictionaryWidget
// - Nested objects → Recursive form generation
```

### 3. Schema-Driven Generation

```cpp
// No hardcoded forms - all generated from schema
FormGenerator generator;
generator.generateFromSchema(schema);

// For any schema change:
// 1. Update JSON schema file
// 2. Reload in application
// 3. Form automatically regenerates
```

### 4. Separation of Concerns

```
Core Library (No UI)
    ↓
UI Layer (Presentation)
    ├── Qt Implementation
    └── Web Implementation
```

---

## Key Characteristics

### Strengths

✅ **Modular**: Clean separation between core logic and UI  
✅ **Flexible**: Dual-target support (Qt + Web)  
✅ **Maintainable**: MISRA C++17 compliance  
✅ **Testable**: Core library testable without UI  
✅ **Extensible**: Easy to add new field types or validators  
✅ **Safe**: Strong error handling with Result<T> pattern  

### Design Decisions

| Decision | Rationale |
|----------|-----------|
| **Qt-Free Core** | Enable code reuse across platforms |
| **Static Core Library** | Simple linking, no runtime dependencies |
| **Result<T> Instead of Exceptions** | MISRA C++17 compliance, explicit error handling |
| **Schema-Driven Forms** | No hardcoded UI, full flexibility |
| **Dual-Target CMake** | Support desktop and server deployments |
| **cpp-httplib** | Header-only, minimal dependencies for server |

---

## Configuration Files

### Schema File (`config.schema.json`)

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "log": { "type": "object", ... },
    "configureRules": { "type": "object", ... },
    ...
  },
  "required": ["log", "configureRules"]
}
```

### Configuration File (`config.json`)

```json
{
  "log": {
    "level": "info",
    "file": "output/app.log"
  },
  "configureRules": { ... }
}
```

---

## API Design

### Core Library API

```cpp
// Schema Loading
Result<JSONSchema, FileError> load_schema(const std::string& path);

// Validation
Result<ValidatedData, ValidationError> validate(
    const JSONSchema& schema,
    const json& data
);

// I/O Operations
Result<json, IOError> read_json(const std::string& path);
Result<void, IOError> write_json(const std::string& path, const json& data);
```

### Qt GUI API

```cpp
// Form Generation
FormGenerator gen;
gen.generateFromSchema(schema);

// Data Collection
json form_data = gen.getFormData();
```

### Web Server API

```
GET /schemas              - List available schemas
GET /schemas/{name}       - Get schema by name
POST /form/submit         - Submit form data
GET /config/{name}        - Get configuration
```

---

## Summary

ConfigGUI follows a **layered, modular architecture** with a **Qt-free core library** that enables both desktop and web deployments. The **schema-driven design** eliminates hardcoded forms, and the **Result<T> pattern** ensures robust error handling. This design supports **easy maintenance**, **platform flexibility**, and **code reuse** across different deployment scenarios.

---

**For Next Phase:**
- Consider database backend for configuration storage
- Add role-based access control for web server
- Implement configuration version control
- Add advanced validation rules
