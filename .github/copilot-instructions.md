# configGui Development Guidelines

Auto-generated from all feature plans. Last updated: 2025-10

## Active Technologies
- C++17 (core) + C++ with Qt6 (Qt GUI) + ES6 JavaScript/HTML5 (HTML GUI) (003-multi-format-save)
- File system (JSON and INI files) (003-multi-format-save)

### Core (All Platforms)
- **Language/Version**: C++17 with MISRA C++ compliance
- **Core Libraries**: nlohmann/json, json-schema-validator, libyaml
- **Patterns**: Result<T> for error handling, static core library

### Qt Application (001-schema-driven-config-gui)
- **Technology**: Qt 6.x (Core, Gui, Widgets, Test)
- **Build**: `-DBUILD_QT_APP=ON`
- **Deployment**: Desktop application
- **Dependencies**: Qt6, core library, validators, I/O

### HTML Server (002-html-form-generation)
- **Technology**: cpp-httplib (embedded HTTP server), HTML5/CSS3, ES6 JavaScript (vanilla, no frameworks)
- **Build**: `-DBUILD_HTML_SERVER=ON`
- **Deployment**: Web server (Docker, systemd, Kubernetes)
- **Dependencies**: cpp-httplib (header-only), core library, JavaScript assets

## Project Structure (Phase 1)

```
configGui/
├── CMakeLists.txt              # Root: Dual-target configuration
├── src/
│   ├── CMakeLists.txt          # Modular organization
│   ├── core/                   # Shared library (always built)
│   │   ├── CMakeLists.txt      # Creates ConfigGUICore static library
│   │   ├── schema.{h,cpp}
│   │   ├── configuration_data.{h,cpp}
│   │   ├── validation_error.{h,cpp}
│   │   ├── schema_loader.{h,cpp}
│   │   ├── schema_validator.{h,cpp}
│   │   ├── form_state.{h,cpp}
│   │   ├── result.h
│   │   └── error_types.h
│   ├── qt/                     # Qt-specific application (conditional)
│   │   ├── CMakeLists.txt      # Creates ConfigGUI executable
│   │   └── (references ../ui, ../validators, ../io, ../utils)
│   ├── html/                   # HTML server module (conditional, Phase 3)
│   │   ├── CMakeLists.txt      # Creates ConfigGUIServer executable
│   │   ├── server/             # HTTP server implementation
│   │   └── handlers/           # Request handlers
│   ├── ui/                     # Qt-specific UI widgets
│   ├── validators/             # Validation logic
│   ├── io/                     # File I/O (JSON, YAML)
│   └── utils/                  # Utility functions
├── resources/
│   ├── html/                   # Web assets (Phase 3)
│   │   ├── index.html
│   │   ├── style.css
│   │   └── form.js
│   ├── configs/                # Example configurations
│   └── schemas/                # Example schemas
├── tests/
│   ├── CMakeLists.txt
│   ├── unit/                   # Core and validator tests
│   ├── integration/            # Qt application flow tests
│   └── html/                   # HTTP server tests (Phase 3)
├── ARCHITECTURE.md             # Module organization and design
└── BUILD.md                    # Build instructions for all scenarios
```

## Build Configurations

### Option 1: Dual-Target Build
```bash
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON ..
# Produces: ConfigGUI (Qt) + ConfigGUIServer (HTTP)
```

### Option 2: Qt-Only Build
```bash
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF ..
# Produces: ConfigGUI (Qt desktop app)
# Qt is required dependency
```

### Option 3: HTML-Only Build
```bash
cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON ..
# Produces: ConfigGUIServer (HTTP server)
# Qt NOT required - enables Docker deployment
# cpp-httplib automatically fetched via FetchContent
```

## Module Dependencies

```
ConfigGUI (Qt app) ──┐
                     ├─→ ConfigGUICore (static lib, C++17, no Qt)
ConfigGUIServer ─────┤    ├─ nlohmann_json
(HTTP server)        │    ├─ json-schema-validator
                     │    └─ yaml-cpp
                     └─→ cpp-httplib (for server only)
```

## Key Principles

1. **Core Library is Qt-Free**
   - All business logic (schema, validation) in core
   - No Qt dependencies in `src/core/`
   - Enables HTML server without Qt

2. **Shared Core Library**
   - `ConfigGUICore` static library
   - Built unconditionally (always created)
   - Linked by both Qt and HTML targets

3. **Modular CMakeLists**
   - Root: Global options, dependencies
   - src/CMakeLists.txt: Module organization
   - Module-specific: Independent configurations

4. **Conditional Compilation**
   - Qt-specific code only when BUILD_QT_APP=ON
   - HTTP server only when BUILD_HTML_SERVER=ON
   - Dependencies resolved based on targets

5. **MISRA C++ Compliance**
   - Strict compiler flags (-Wall -Wextra -Werror -Wpedantic)
   - All modules subject to same flags
   - clang-tidy integration

## Development Guidelines

### Adding New Core Logic
- Place in `src/core/` (no Qt dependencies!)
- Must be buildable in ConfigGUICore library
- Will be available to both Qt and HTML

### Adding Qt UI Components
- Place in `src/ui/` (Qt-specific widgets)
- Reference core library only
- Only built when BUILD_QT_APP=ON

### Adding HTTP Endpoints (Phase 3)
- Place in `src/html/handlers/`
- Reference core library only
- Only built when BUILD_HTML_SERVER=ON

### Adding Tests
- Unit tests: `tests/unit/` (test core logic)
- Integration tests: `tests/integration/` (test Qt flows)
- HTML tests: `tests/html/` (test server, Phase 3)

## Build and Test Commands

```bash
# Configure all targets
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON -B build

# Build
cmake --build build --parallel 4

# Run tests
ctest --test-dir build --output-on-failure

# Run specific target
cmake --build build --target ConfigGUI    # Qt app only
cmake --build build --target ConfigGUICore # Core lib only

# Install
cmake --install build --prefix /usr/local
```

## CMake Patterns

### Conditional Subdirectory
```cmake
# src/CMakeLists.txt
add_subdirectory(core)              # Always
if(BUILD_QT_APP)
    add_subdirectory(qt)            # Qt-specific
endif()
if(BUILD_HTML_SERVER)
    add_subdirectory(html)          # HTML server
endif()
```

### Creating Static Core Library
```cmake
# src/core/CMakeLists.txt
add_library(ConfigGUICore STATIC ${CORE_SOURCES})
target_link_libraries(ConfigGUICore PUBLIC
    nlohmann_json::nlohmann_json
    nlohmann_json_schema_validator
    yaml-cpp
)
set(CONFIGGUI_CORE_LIB ConfigGUICore PARENT_SCOPE)
```

### Conditional Dependency
```cmake
# Root CMakeLists.txt
if(BUILD_QT_APP)
    find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
endif()
```

### FetchContent for Header-Only Library
```cmake
FetchContent_Declare(cpp-httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.0
)
if(BUILD_HTML_SERVER)
    FetchContent_MakeAvailable(cpp-httplib)
endif()
```

## Code Style

- **Language**: C++17 with MISRA C++ compliance
- **Naming**: snake_case for variables/functions, CamelCase for types/classes
- **Error Handling**: Use Result<T> pattern, never exceptions in core
- **Memory**: Prefer stack allocation, document any heap allocations
- **Formatting**: Consistent with existing code, use clang-format

## Recent Changes (Phase 1)

- ✅ Dual-target CMake build system (BUILD_QT_APP, BUILD_HTML_SERVER options)
- ✅ Modular CMakeLists structure (root → src/ → core/qt/html)
- ✅ ConfigGUICore static library (core logic, no Qt dependency)
- ✅ Conditional Qt configuration (only when BUILD_QT_APP=ON)
- ✅ cpp-httplib dependency via FetchContent (conditional on BUILD_HTML_SERVER)
- ✅ All 3 build scenarios verified working (dual, Qt-only, HTML-only)
- ✅ ARCHITECTURE.md documenting module organization
- ✅ BUILD.md with build instructions for all scenarios
- ✅ Phase 1 directory structure complete

## Next Steps (Phase 2)

1. Extract I/O classes to core library
2. Move validators to core library
3. Update tests to use core library
4. Refactor Qt app to use core library
5. Remove duplication between targets

<!-- MANUAL ADDITIONS START -->
### Phase 1 Completion Checklist
- [x] T001: Root CMakeLists.txt dual-target configuration
- [x] T002: src/CMakeLists.txt modular organization
- [x] T003: src/core/CMakeLists.txt core library
- [x] T004: src/qt/CMakeLists.txt Qt application module
- [x] T005: src/html/CMakeLists.txt HTML server module
- [x] T006: Add cpp-httplib dependency via FetchContent
- [x] T007: Test dual-target build scenario
- [x] T008: Test Qt-only build scenario
- [x] T009: Test HTML-only build scenario
- [x] T010-T017: Create phase 1 directories
- [x] T018: Create ARCHITECTURE.md
- [x] T019: Create BUILD.md
- [x] T020: Update copilot-instructions.md

**Phase 1 Status**: ✅ COMPLETE (20/20 tasks)

<!-- MANUAL ADDITIONS END -->

