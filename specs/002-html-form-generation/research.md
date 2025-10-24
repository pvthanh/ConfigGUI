# Research: HTML Form Generation Implementation

**Date**: October 24, 2025  
**Feature Branch**: `002-html-form-generation`  
**Status**: Phase 0 Research Complete

---

## NEEDS CLARIFICATION - Research Results

This document resolves all NEEDS CLARIFICATION items from the Implementation Plan by researching options, evaluating against feature requirements, and making justified decisions.

---

## Decision 1: HTTP Server Library

**NEEDS CLARIFICATION**: Embedded HTTP library vs standalone server vs framework approach

### Context
- Feature requires: Local HTTP server, schema selection interface, form generation service
- Constraints: Must work in Qt-free environments; MISRA C++ compliance; C++17 compatibility
- Performance: Startup <2s, form generation <1s, validation feedback <200ms
- Scale: 10 concurrent users (MVP)

### Options Researched

#### Option A: Embedded C++ HTTP Library
**Candidates**: cpp-httplib, crow, Beast (Asio-based)

**Advantages**:
- ✅ No external dependencies beyond C++17
- ✅ Single binary deployment
- ✅ MISRA C++ compatible with careful use
- ✅ Full control over request handling
- ✅ Good performance for single-machine use
- ✅ Integrates cleanly with existing C++ codebase

**Disadvantages**:
- ❌ Limited HTTP/2 support in simpler libraries
- ❌ Manual routing and middleware handling
- ❌ Requires careful error handling for MISRA compliance

**Best Fit**: cpp-httplib (header-only, MIT licensed, widely used)
- Single header file (httplib.h)
- Supports both synchronous and async operations
- Clean API matching requirement pattern
- Active maintenance, MISRA-friendly with annotations

#### Option B: Standalone Server (Node.js wrapper)
**Approach**: C++ core exposes JSON API, Node.js Express server wraps it

**Advantages**:
- ✅ Rich HTTP/HTML ecosystem in Node.js
- ✅ Easy frontend development with npm packages
- ✅ Rapid prototyping capability

**Disadvantages**:
- ❌ Requires Node.js runtime (violates "no external dependencies" goal)
- ❌ Adds deployment complexity
- ❌ Process management overhead
- ❌ Not deployable on systems without Node.js
- ❌ Violates Qt-free requirement (adds Node.js dependency instead)

**Rejected**: Not suitable for Qt-free deployment goal

#### Option C: Framework Approach (Qt for WebEngine)
**Approach**: Use Qt WebEngine for HTML UI (but requires Qt)

**Disadvantages**:
- ❌ Defeats purpose of HTML-only deployment (still requires Qt)
- ❌ Not available in non-Qt environments
- ❌ Same footprint as Qt desktop app

**Rejected**: Fundamentally incompatible with MVP goals

### Decision
**✅ CHOSEN: cpp-httplib (Embedded C++ HTTP Library)**

**Rationale**:
- Aligns with MISRA C++ compliance requirements
- Zero external process dependencies
- Single-binary deployment achievable
- Sufficient for 10 concurrent users (MVP scope)
- Integrates with existing C++ codebase
- No Qt dependencies - enables non-Qt deployment

**Implementation Approach**:
```cpp
// src/html/server.h/cpp
class HttpServer {
    std::unique_ptr<httplib::Server> server_;  // Smart pointer for MISRA compliance
    std::string schemas_dir_;
    std::shared_ptr<SchemaValidator> validator_;  // Shared from core
public:
    bool start(int port, const std::string& schemas_dir);
    // Routes:
    // GET  / → main form (schema selection)
    // POST /api/schema/validate → validate schema file
    // POST /api/form/generate → generate HTML form from schema
    // POST /api/config/save → save configuration
    // GET  /api/config/load → load configuration
};
```

**Build Integration**:
- CMake: `find_package(httplib)` or download header-only version
- Compilation: No additional build flags needed
- MISRA compliance: Use static analysis with config for http library

---

## Decision 2: Frontend Framework & Form Generation

**NEEDS CLARIFICATION**: Vanilla JS vs lightweight framework vs full framework

### Context
- Feature requires: Dynamic HTML forms matching Qt layout/validation
- Constraints: 95%+ visual/functional parity with Qt; real-time validation feedback (<200ms)
- Performance: Form generation <1s; validation feedback <200ms
- Scale: Single-user initially (MVP)
- Browser support: Modern browsers (Chrome, Firefox, Safari, Edge last 2 years)

### Options Researched

#### Option A: Vanilla JavaScript with HTML Templates
**Approach**: Use HTML template tags + DOM API + vanilla event listeners

**Advantages**:
- ✅ No build step required
- ✅ Minimal JavaScript dependencies
- ✅ Easy to debug (no transpilation)
- ✅ Lightweight bundle (~10KB for form logic)
- ✅ Direct DOM manipulation enables fine-grained control for Qt parity

**Disadvantages**:
- ❌ Manual state management (error messages, field validation state)
- ❌ More boilerplate for complex form generation
- ❌ Manual event delegation for dynamic forms

**Example approach**:
```javascript
// src/html/assets/js/form-generator.js
class FormGenerator {
    constructor(schema, validator) {
        this.schema = schema;
        this.validator = validator;  // Backend validator
        this.formState = {};
    }
    
    generateForm(containerId) {
        const container = document.getElementById(containerId);
        const form = document.createElement('form');
        
        for (const [fieldName, fieldSchema] of Object.entries(this.schema.properties)) {
            const field = this.createField(fieldName, fieldSchema);
            field.addEventListener('change', async (e) => {
                const result = await this.validateField(fieldName, e.target.value);
                this.showFeedback(fieldName, result);
            });
            form.appendChild(field);
        }
        container.appendChild(form);
    }
    
    async validateField(fieldName, value) {
        const response = await fetch('/api/validate', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ field: fieldName, value: value })
        });
        return response.json();
    }
}
```

#### Option B: Lightweight Framework (Alpine.js or htmx)
**Approach**: Alpine.js for reactive UI without build step

**Advantages**:
- ✅ Minimal footprint (~15KB gzipped)
- ✅ No build step
- ✅ Reactive data binding for clean validation feedback
- ✅ HTML-centric (x-* attributes declarative)

**Disadvantages**:
- ❌ Alpine.js not optimized for complex form layouts
- ❌ Still requires learning curve for new framework
- ❌ Performance not significantly better than vanilla JS for this use case

**Better fit for this project**: Alpine could work but adds unnecessary abstraction

#### Option C: Full Framework (Vue, React, Angular)
**Approach**: Modern SPA framework with build tooling

**Advantages**:
- ✅ Rich component ecosystem
- ✅ Strong validation/form libraries
- ✅ Good debugging tools

**Disadvantages**:
- ❌ Build step required (Webpack, Vite, etc.)
- ❌ Large bundle size (Vue min: 100KB gzipped)
- ❌ Overkill for single-form MVP
- ❌ Deployment complexity (requires Node.js build)
- ❌ Learning curve for team

**Rejected**: Over-engineered for MVP scope

#### Option D: Server-Side Rendering (SSR) Hybrid
**Approach**: Generate HTML on server, minimal JS on client

**Advantages**:
- ✅ Excellent initial load performance
- ✅ Works without JavaScript
- ✅ SEO-friendly (not relevant here, but beneficial architecture)

**Disadvantages**:
- ❌ Reduced real-time validation responsiveness
- ❌ More server load (form regeneration per request)

**Better for later phases**: Could optimize form generation later

### Decision
**✅ CHOSEN: Vanilla JavaScript with HTML Templates**

**Rationale**:
- Minimum viable complexity for MVP
- No build step enables rapid iteration
- Direct control enables Qt parity
- Fast development with native browser APIs
- Easy to migrate to lightweight framework later if needed

**Implementation Approach**:
```
src/html/assets/
├── index.html              # Main form (schema selection)
├── form.html               # Form template
├── js/
│   ├── form-generator.js   # Dynamic form creation
│   ├── validator.js        # Client-side validation mirror
│   ├── config-manager.js   # Configuration save/load
│   └── main.js             # App initialization
└── css/
    └── styles.css          # Form styling matching Qt
```

**Qt Parity Implementation**:
- Validation feedback: CSS classes for error states (matches Qt feedback)
- Field types: Input generators for text, number, select, checkbox, etc.
- Nested objects: Recursive form generation for nested schemas
- Rule display: Tooltip system showing schema rules (like Qt tooltips)

---

## Decision 3: Build System Enhancements

**NEEDS CLARIFICATION**: How to compile both Qt and HTML targets

### Context
- Current: Single CMakeLists.txt compiling Qt application
- New requirement: Two targets (Qt app + HTML server) from same codebase
- Constraint: Must not break existing Qt build
- Goal: Shared core reused by both targets

### Options Researched

#### Option A: Conditional Compilation with CMake
**Approach**: Use CMake conditionals to build different targets

```cmake
# CMakeLists.txt (root)
cmake_minimum_required(VERSION 3.16)
project(ConfigGUI)

# Core library (always built)
add_library(configgui_core
    src/core/schema/schema_loader.cpp
    src/core/io/json_reader.cpp
    # ... other core files
)

# Qt target (optional)
option(BUILD_QT_APP "Build Qt desktop application" ON)
if(BUILD_QT_APP)
    add_executable(ConfigGUI 
        src/qt/main.cpp
        src/qt/ui/form_generator.cpp
        # ... other Qt files
    )
    target_link_libraries(ConfigGUI configgui_core Qt6::Core Qt6::Gui Qt6::Widgets)
endif()

# HTML server target (optional)
option(BUILD_HTML_SERVER "Build HTML server" OFF)
if(BUILD_HTML_SERVER)
    add_executable(configgui_server
        src/html/main.cpp
        src/html/server.cpp
        # ... other server files
    )
    target_link_libraries(configgui_server configgui_core)
endif()
```

**Advantages**:
- ✅ Clear build control
- ✅ Shared core library reused
- ✅ Each target independently compilable
- ✅ Default builds Qt (backward compatible)

**Disadvantages**:
- ❌ Need to manage two targets in CI/CD
- ❌ Could get complex with shared dependencies

**Best for**: Flexible development and deployment

#### Option B: Separate CMakeLists.txt per module
**Approach**: Root CMakeLists.txt delegates to src/core, src/qt, src/html

```cmake
# CMakeLists.txt (root)
add_subdirectory(src/core)
option(BUILD_QT "Build Qt components" ON)
if(BUILD_QT)
    add_subdirectory(src/qt)
endif()
option(BUILD_HTML "Build HTML server" OFF)
if(BUILD_HTML)
    add_subdirectory(src/html)
endif()
```

**Advantages**:
- ✅ Modular organization
- ✅ Each component can evolve independently
- ✅ Clear dependency boundaries

**Disadvantages**:
- ❌ More CMakeLists.txt files to maintain
- ❌ Harder to test changes across modules

#### Option C: Build Flags Approach
**Approach**: Single executable with runtime selection of UI backend

```cpp
// src/main.cpp
int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--html") {
        // Launch HTML server
        return html_server_main(argc, argv);
    } else {
        // Launch Qt app
        return qt_app_main(argc, argv);
    }
}
```

**Advantages**:
- ✅ Single deployment binary
- ✅ Simple to switch between UIs

**Disadvantages**:
- ❌ Still links both Qt and HTTP server (larger binary)
- ❌ Defeats goal of Qt-free HTML deployment
- ❌ Not suitable for embedded systems without Qt

**Rejected**: Doesn't meet Qt-free requirement

### Decision
**✅ CHOSEN: Conditional Compilation with CMake + Separate Build Targets**

**Rationale**:
- Qt build remains default (backward compatible): `cmake .. && make`
- HTML deployment builds without Qt: `cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON .. && make`
- Shared core cleanly extracted into library
- Clear separation enables independent testing

**Build Commands**:
```bash
# Default: build only Qt
cmake -B build && make -C build

# Qt only (explicit)
cmake -B build -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF && make -C build

# HTML server only (for Docker/embedded)
cmake -B build -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON && make -C build

# Both (for testing)
cmake -B build -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON && make -C build
```

---

## Decision 4: Shared Core Extraction Strategy

**NEEDS CLARIFICATION**: How to refactor without breaking Qt build

### Context
- Current code structure: Qt code and core mixed in src/core/, src/io/, src/utils/
- Goal: Extract to enable HTML server build without Qt
- Constraint: Must not break existing Qt builds during refactoring
- Challenge: Gradual refactoring without stopping Qt development

### Options Researched

#### Option A: Create Abstraction Layers (Recommended)
**Approach**: Define interfaces in core, let Qt/HTML provide implementations

```cpp
// src/core/schema/schema_loader.h
class SchemaLoader {
public:
    virtual ~SchemaLoader() = default;
    virtual Result<nlohmann::json> loadSchema(const std::string& path) const = 0;
};

// src/core/schema/file_schema_loader.h - concrete implementation (Qt-independent)
class FileSchemaLoader : public SchemaLoader {
public:
    Result<nlohmann::json> loadSchema(const std::string& path) const override {
        // File I/O using std::filesystem (C++17)
    }
};

// Usage: Both Qt and HTML can use FileSchemaLoader
auto loader = std::make_unique<FileSchemaLoader>(schemas_dir);
```

**Advantages**:
- ✅ Clean separation of interface vs implementation
- ✅ Qt and HTML can use same implementations
- ✅ Testable in isolation
- ✅ Gradual migration path
- ✅ No Qt #include in core header files

**Disadvantages**:
- ❌ Initial refactoring effort to define interfaces
- ❌ More virtual functions (minor performance impact)

#### Option B: Use CMake Object Libraries
**Approach**: Compile core once as object library, link to both targets

```cmake
# src/core/CMakeLists.txt
add_library(configgui_core_objs OBJECT
    schema_loader.cpp
    json_reader.cpp
    # ...
)

# src/qt/CMakeLists.txt
add_executable(ConfigGUI $<TARGET_OBJECTS:configgui_core_objs> main.cpp ...)

# src/html/CMakeLists.txt
add_executable(configgui_server $<TARGET_OBJECTS:configgui_core_objs> main.cpp ...)
```

**Advantages**:
- ✅ No virtual function overhead
- ✅ Single compilation of core code
- ✅ Simple to implement

**Disadvantages**:
- ❌ Object library approach less clear semantically
- ❌ Harder to test core independently
- ❌ Less flexible for future changes

#### Option C: Static Library with Careful Dependencies
**Approach**: Build core as static library, ensure no Qt dependencies

```cmake
# src/core/CMakeLists.txt
add_library(configgui_core STATIC
    schema_loader.cpp
    json_reader.cpp
    # ...
)
# NO target_link_libraries(configgui_core Qt6::Core)

# src/qt/CMakeLists.txt
target_link_libraries(ConfigGUI configgui_core Qt6::Core Qt6::Gui Qt6::Widgets)

# src/html/CMakeLists.txt
target_link_libraries(configgui_server configgui_core)
```

**Advantages**:
- ✅ Standard library linking
- ✅ Clear dependency graph
- ✅ Testable core library

**Disadvantages**:
- ❌ Requires careful #include discipline (Qt headers not in core headers)
- ❌ Build time (recompile for each target)

### Decision
**✅ CHOSEN: Static Library + Abstraction Layers Approach**

**Implementation Path**:

**Phase 1: Preparation**
```cpp
// Step 1: Create abstract interfaces in core (no Qt dependencies)
src/core/
├── schema/
│   ├── schema_loader.h (pure interface + file implementation)
│   └── schema_validator.h
├── io/
│   ├── json_reader.h (pure interface + implementation)
│   └── file_manager.h (interface for file I/O)
└── types.h (Result<T>, common exceptions)
```

**Phase 2: Extract Implementation**
```cpp
// Step 2: Move implementations to core/ (no Qt #includes)
// Verify with clang: grep -r "include.*Qt" src/core/  → should be empty

// Step 3: Create static library
// CMake: add_library(configgui_core STATIC ...)
```

**Phase 3: Integration**
```cpp
// Step 4: Qt links against core (unchanged usage)
target_link_libraries(ConfigGUI configgui_core Qt6::Core ...)

// Step 5: HTML server links against core (new usage)
target_link_libraries(configgui_server configgui_core)
```

**Verification**:
```bash
# Ensure no Qt in core build
nm src/core/CMakeFiles/configgui_core.dir/*.cpp.o | grep -i Qt → should be empty

# Verify both targets link against core
nm build/ConfigGUI | grep configgui_core → should have symbols
nm build/configgui_server | grep configgui_core → should have symbols
```

---

## Research Decisions Summary

| Decision | Choice | Rationale |
|----------|--------|-----------|
| HTTP Server | cpp-httplib (embedded C++) | MISRA compliance, single binary, zero external dependencies, Qt-free deployment |
| Frontend | Vanilla JavaScript + HTML templates | Minimal complexity, no build step, direct control for Qt parity, fast iteration |
| Build System | CMake conditional targets | Clear separation, backward compatible, enables independent deployment |
| Core Extraction | Static library + abstraction layers | Reusable by both targets, testable, maintainable, MISRA-compliant |

## Technical Stack - FINALIZED

**Backend**:
- Language: C++17 with MISRA C++ compliance
- HTTP Server: cpp-httplib (header-only)
- Core Libraries: nlohmann/json, json-schema-validator, libyaml
- Build: CMake 3.16+
- Testing: CMake test framework

**Frontend** (HTML):
- HTML5, CSS3, ES6 JavaScript (no frameworks)
- Vanilla DOM API for form generation
- HTTP fetch API for async calls
- No build step required

**Qt Application** (unchanged):
- Qt 6.x (Core, Gui, Widgets)
- Uses same C++ core as HTML server
- Desktop deployment

**Deployment Options**:
- **Option 1**: Qt application (includes Qt dependencies)
- **Option 2**: HTML server only (no Qt - for embedded/Docker)
- **Option 3**: Both (for systems with both capabilities)

## Next: Phase 1 Design

With all NEEDS CLARIFICATION resolved, ready to proceed to Phase 1 Design phase which will generate:
- `data-model.md` - Data structures and entities
- `contracts/openapi.yaml` - HTTP API specification
- `quickstart.md` - Developer setup guide
- Update agent context

---

**Research Complete**: All 4 NEEDS CLARIFICATION items resolved with justified decisions ready for implementation.
