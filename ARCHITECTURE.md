# ConfigGUI Architecture

## Overview

ConfigGUI is a **dual-target C++17 application** that provides schema-driven configuration UI in two forms:

1. **Qt GUI Application** - Desktop GUI using Qt 6.x
2. **HTML Server Application** - Web-based interface with embedded HTTP server

Both targets share a **common core library** with zero duplication, enabling consistent functionality across platforms.

## Architecture Pattern

```
┌─────────────────────────────────────────────────────┐
│                  Shared Core Library                 │
│  (Schema Loading, Validation, I/O - Qt Independent) │
│                                                     │
│  • schema.{h,cpp}              - Schema definition  │
│  • configuration_data.{h,cpp}  - Config data model  │
│  • validation_error.{h,cpp}    - Error reporting   │
│  • schema_loader.{h,cpp}       - Schema parsing    │
│  • schema_validator.{h,cpp}    - Validation logic  │
└─────────────────────────────────────────────────────┘
            ▲                           ▲
            │                           │
    ┌───────┴──────────┐      ┌────────┴─────────┐
    │   Qt GUI Module   │      │ HTML Server Mod  │
    │  (src/qt/)        │      │  (src/html/)     │
    │                   │      │                  │
    │ • main_window.*   │      │ • http_server.*  │
    │ • form_gen.*      │      │ • handlers.*     │
    │ • widgets.*       │      │ • assets/        │
    │ • validators/     │      │ • JavaScript     │
    │ • ui/*            │      │                  │
    └─────────┬─────────┘      └────────┬─────────┘
              │                        │
        ConfigGUI                ConfigGUIServer
        (Desktop App)           (HTTP Server)
```

## Module Organization

### Core Library: `src/core/CMakeLists.txt`

**Target**: `ConfigGUICore` (static library)

**Purpose**: Shared logic for both Qt and HTML implementations

**Key Files**:
- `schema.{h,cpp}` - JSON/YAML schema representation
- `configuration_data.{h,cpp}` - Configuration instance storage
- `validation_error.{h,cpp}` - Validation error handling
- `form_state.{h,cpp}` - Form state management
- `result.h` - Result<T> wrapper for error handling
- `error_types.h` - Error type definitions
- `schema_loader.{h,cpp}` - Load schemas from files
- `schema_validator.{h,cpp}` - Validate configurations against schemas

**Dependencies**:
- `nlohmann_json` - JSON parsing
- `nlohmann_json_schema_validator` - JSON schema validation
- `yaml-cpp` - YAML I/O
- C++17 Standard Library

**Key Design**:
- No Qt dependencies - pure C++17 code
- Header-only utilities in `result.h` and `error_types.h`
- All validation logic centralized and testable
- Type-safe error handling with Result<T> pattern

### Qt Module: `src/qt/CMakeLists.txt`

**Target**: `ConfigGUI` (executable)

**Purpose**: Qt GUI application wrapper around core

**Key Files**:
- `ui/main_window.{h,cpp}` - Main application window
- `ui/widget_factory.{h,cpp}` - Qt widget generation
- `ui/form_generator.{h,cpp}` - Form generation from schema
- `ui/validation_feedback_widget.{h,cpp}` - Validation UI
- `validators/*` - Qt-specific validators
- `ui/array_widget.{h,cpp}` - Array field handling
- `ui/dictionary_widget.{h,cpp}` - Object field handling
- (Other specialized widgets)

**Dependencies**:
- `ConfigGUICore` (static library)
- `Qt6::Core`
- `Qt6::Gui`
- `Qt6::Widgets`

**Key Design**:
- Thin wrapper - delegates all logic to core
- Qt widgets translate core schema to UI elements
- Validators translate core errors to UI feedback
- Can be built independently: `cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF`

### HTML Module: `src/html/CMakeLists.txt`

**Target**: `ConfigGUIServer` (executable, Phase 3)

**Purpose**: HTTP server + web-based form UI

**Structure** (to be implemented):
```
src/html/
├── server/
│   ├── main.cpp              - Server entry point
│   ├── http_server.{h,cpp}   - cpp-httplib wrapper
│   └── request_router.{h,cpp} - Route requests to handlers
├── handlers/
│   ├── schema_handler.{h,cpp}   - Schema list/load endpoints
│   ├── form_handler.{h,cpp}     - Form generation endpoints
│   └── validation_handler.{h,cpp} - Validation endpoints
└── assets/
    ├── index.html           - Main form UI
    ├── style.css            - Form styling
    ├── form.js              - Form interaction
    └── validate.js          - Client-side validation
```

**Dependencies**:
- `ConfigGUICore` (static library)
- `cpp-httplib` (header-only)
- C++17 Standard Library
- HTML5 + CSS3 + ES6 JavaScript (vanilla, no frameworks)

**Key Design**:
- No Qt dependency - pure C++ HTTP server
- Embedded HTTP server using cpp-httplib
- Vanilla JavaScript (no build step, no npm)
- Can be Dockerized for deployment
- Can be built independently: `cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON`

## Dependency Flow

```
Incoming Configuration File
    │
    ├─→ [Core Library]
    │   ├─ SchemaLoader → Parse JSON/YAML schema
    │   ├─ Schema → Represent schema structure
    │   └─ SchemaValidator → Validate config against schema
    │
    ├─→ Qt Path
    │   └─ FormGenerator (Qt) → Create Qt widgets → ConfigGUI
    │
    └─→ HTML Path
        └─ FormHandler (HTTP) → Generate HTML form → ConfigGUIServer
```

## Build System

### Conditional Compilation

The dual-target build is managed via CMake options:

```bash
# Both targets
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON

# Qt only
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF

# HTML only
cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON
```

### Module CMakeLists Structure

```
CMakeLists.txt (root)
├─ Add options (BUILD_QT_APP, BUILD_HTML_SERVER)
├─ Find packages (conditional Qt, always core deps)
├─ FetchContent cpp-httplib (conditional)
└─ add_subdirectory(src)
    │
    └─ src/CMakeLists.txt
        ├─ add_subdirectory(core) [always]
        ├─ if(BUILD_QT_APP) add_subdirectory(qt)
        └─ if(BUILD_HTML_SERVER) add_subdirectory(html)
            │
            ├─ src/core/CMakeLists.txt
            │   └─ add_library(ConfigGUICore STATIC ...)
            │
            ├─ src/qt/CMakeLists.txt
            │   └─ add_executable(ConfigGUI ...)
            │
            └─ src/html/CMakeLists.txt
                └─ (placeholder, Phase 3)
```

## Key Design Principles

### 1. **No Core Qt Dependency**
- Core library is pure C++17
- Qt is only in the UI layer
- Enables HTML deployment without Qt

### 2. **100% Code Reuse**
- All schema/validation logic in core
- Both Qt and HTML use identical validation rules
- No duplicate business logic

### 3. **Type Safety**
- Result<T> pattern for error handling
- Strong types for schema entities
- Compile-time validation where possible

### 4. **MISRA C++ Compliance**
- Strict compiler flags (-Wall -Wextra -Werror -Wpedantic)
- No virtual destructor violations
- No old-style casts
- Automatic format checking with clang-tidy

### 5. **Modular Independence**
- Each target builds independently
- Clear dependency boundaries
- Testable in isolation

## Cross-Target Consistency

### Validation
- **Core Logic**: Same SchemaValidator for both
- **Execution**: Both sync/real-time and form-level validation
- **Errors**: Consistent ValidationError messages

### Schema Support
- **Input**: Both Qt and HTML support JSON and YAML
- **Features**: Both support all schema types (string, number, array, object, etc.)
- **Rules**: Both enforce pattern, min/max, enum, etc.

### Configuration I/O
- **Format**: Both use JSON/YAML for persistence
- **Location**: Both read/write from local filesystem

## Testing Strategy

### Unit Tests (Qt app)
```cpp
// tests/unit/ - Core functionality
- Schema loading and parsing
- Schema validation rules
- Configuration data model
- Error handling
```

### Integration Tests (Qt app)
```cpp
// tests/integration/ - End-to-end flows
- Load schema → Generate form → Validate → Save
- Error recovery paths
- Edge cases
```

### HTML Tests (Phase 3)
```javascript
// tests/html/ - Server and client
- HTTP endpoint behavior
- Form generation correctness
- Client-side validation
- Schema consistency
```

## Deployment Scenarios

### Desktop (Qt App)
```bash
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF
make
./ConfigGUI
```

### Web Server (HTML)
```bash
cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON
make
./ConfigGUIServer --port 8080
# Browse: http://localhost:8080
```

### Both (Development)
```bash
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON
make
./ConfigGUI          # Qt app
./ConfigGUIServer    # Web server
```

### Container (HTML)
```dockerfile
FROM ubuntu:22.04
# Install dependencies
COPY . /app
WORKDIR /app
RUN cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON -B build && \
    cmake --build build
ENTRYPOINT ["./build/ConfigGUIServer"]
```

## Performance Considerations

### Core Library
- Static library (no shared library overhead)
- Header-only utilities eliminate inline overhead
- No dynamic allocation in hot paths (where possible)

### Qt Application
- Direct Qt signals/slots (no message passing overhead)
- Lazy widget creation
- Schema cached in memory

### HTML Server
- Embedded HTTP server (no process/socket overhead)
- JSON serialization (streaming-friendly)
- Client-side validation reduces server load

## Future Extensions

### Phase 2: Core Extraction
- Refactor I/O classes to shared core
- Extract validation rules to reusable component
- Add test coverage for core functionality

### Phase 3: HTML Server
- Implement HTTP endpoints
- Add JavaScript form generation
- Client-side validation

### Phase 4+: Advanced Features
- Export configurations
- Docker deployment
- Docker Compose support
- Advanced schema features

## Files Modified/Created

**Phase 1 (Current)**:
- ✅ `CMakeLists.txt` - Dual-target build configuration
- ✅ `src/CMakeLists.txt` - Modular organization
- ✅ `src/core/CMakeLists.txt` - Core library (ConfigGUICore)
- ✅ `src/qt/CMakeLists.txt` - Qt application
- ✅ `src/html/CMakeLists.txt` - HTML server (placeholder)
- ✅ `ARCHITECTURE.md` - This document

**Phase 2**: Core extraction and refactoring

**Phase 3**: HTML server implementation

---

*Generated during Phase 1 Implementation - T018*
*Last Updated: 2025-10*
