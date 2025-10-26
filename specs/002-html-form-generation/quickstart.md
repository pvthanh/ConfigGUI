# Developer Quickstart: HTML Form Generation

**Date**: October 24, 2025  
**Feature Branch**: `002-html-form-generation`  
**Status**: Phase 1 Design Complete

---

## Overview

This guide helps developers set up, build, and run the HTML form generation feature locally. Both the Qt desktop application and HTML server can be built and run independently.

---

## Prerequisites

### Required
- **C++17 compiler** (GCC 7+, Clang 5+, MSVC 2015+)
- **CMake 3.16+** - For building
- **nlohmann/json** - JSON library (likely already installed for Qt build)
- **json-schema-validator** - Schema validation (likely already installed)
- **libyaml** - YAML support (likely already installed)
- **cpp-httplib** - Will be downloaded as header-only dependency

### Optional
- **Docker** - For containerized deployment (post-MVP)
- **Postman** - For API testing
- **Modern web browser** - For HTML interface testing (Chrome, Firefox, Safari, Edge)

### Already Installed (Qt build)
All core dependencies from existing ConfigGUI setup:
```bash
# Check existing installations
dpkg -l | grep -E "nlohmann|yaml"
pkg-config --modversion json-schema-validator
```

---

## Build Setup

### Option 1: Build Qt Application (Default)

This is the existing build process - unchanged by HTML feature.

```bash
# Clone/navigate to repository
cd /path/to/configGui

# Configure build (default: Qt only)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --target ConfigGUI -j$(nproc)

# Run
./build/bin/ConfigGUI
```

**Build Output**:
```
build/
├── bin/
│   └── ConfigGUI          # Qt desktop application (unchanged)
├── lib/
│   └── libconfiggui_core.a    # NEW: Shared core library
└── CMakeFiles/
    └── ... (build intermediates)
```

### Option 2: Build HTML Server Only

For deployment in non-Qt environments (embedded, Docker, lightweight servers).

```bash
# Configure for HTML server only
cmake -B build-html \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_QT_APP=OFF \
    -DBUILD_HTML_SERVER=ON

# Build
cmake --build build-html -j$(nproc)

# Run
./build-html/bin/configgui_server --port 8080 --schemas-dir ./resources/schemas
```

**Build Output**:
```
build-html/
├── bin/
│   └── configgui_server   # HTML HTTP server (NEW)
└── lib/
    └── libconfiggui_core.a
```

**Run Options**:
```bash
# Default port 8080, schemas from ./resources/schemas
./build-html/bin/configgui_server

# Custom port
./build-html/bin/configgui_server --port 3000

# Custom schemas directory
./build-html/bin/configgui_server --port 8080 --schemas-dir /etc/configgui/schemas

# Both parameters
./build-html/bin/configgui_server --port 8080 --schemas-dir ./resources/schemas

# Help
./build-html/bin/configgui_server --help
```

### Option 3: Build Both Qt and HTML Server

For complete feature testing.

```bash
# Configure for both
cmake -B build-all \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_QT_APP=ON \
    -DBUILD_HTML_SERVER=ON

# Build both
cmake --build build-all -j$(nproc)

# Run Qt app
./build-all/bin/ConfigGUI

# Run HTML server in another terminal
./build-all/bin/configgui_server --port 8080
```

---

## CMakeLists.txt Structure

### Root CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(ConfigGUI)

# Build options
option(BUILD_QT_APP "Build Qt desktop application" ON)
option(BUILD_HTML_SERVER "Build HTML server" OFF)

# Shared core library (always built)
add_library(configgui_core STATIC
    src/core/schema/schema_loader.cpp
    src/core/schema/schema_validator.cpp
    src/core/io/json_reader.cpp
    src/core/io/yaml_reader.cpp
    # ... other core files
)

# Qt target (conditional)
if(BUILD_QT_APP)
    find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
    add_executable(ConfigGUI
        src/qt/main.cpp
        src/qt/ui/form_generator.cpp
        # ... other Qt files
    )
    target_link_libraries(ConfigGUI configgui_core Qt6::Core Qt6::Gui Qt6::Widgets)
endif()

# HTML server target (conditional)
if(BUILD_HTML_SERVER)
    add_executable(configgui_server
        src/html/main.cpp
        src/html/server.cpp
        # ... other server files
    )
    target_link_libraries(configgui_server configgui_core)
endif()
```

### Build Verification

```bash
# Verify Qt build doesn't link HTML server
nm build/bin/ConfigGUI | grep -i "httplib\|http_server" → should be empty

# Verify HTML server build doesn't link Qt
nm build-html/bin/configgui_server | grep -i "Qt" → should be empty

# Verify both use shared core
nm build/bin/ConfigGUI | grep "configgui_core" → should have symbols
nm build-html/bin/configgui_server | grep "configgui_core" → should have symbols
```

---

## Running the Applications

### Qt Application

```bash
# Build (if not already built)
cmake -B build && cmake --build build --target ConfigGUI

# Run
./build/bin/ConfigGUI

# Expected output:
# - Qt window opens with main form
# - Schema selection dropdown
# - Form generation from selected schema
# - Real-time validation feedback
```

### HTML Server

```bash
# Build with HTML support
cmake -B build-html \
    -DBUILD_QT_APP=OFF \
    -DBUILD_HTML_SERVER=ON

# Build
cmake --build build-html

# Run
./build-html/bin/configgui_server --port 8080 --schemas-dir ./resources/schemas

# Expected console output:
# [INFO] ConfigGUI HTML Server v1.0.0
# [INFO] Listening on http://localhost:8080
# [INFO] Schemas directory: ./resources/schemas
# [INFO] Found 3 schemas: app_config, database_config, user_config

# Access in browser:
# Open http://localhost:8080 in web browser
```

**HTML Browser Interface**:
1. **Main Form** (index.html):
   - List of available schemas
   - Select a schema to generate form
   - Upload custom schema option

2. **Generated Form**:
   - Fields for each schema property
   - Real-time validation feedback
   - Save/Cancel buttons

3. **Configuration Saved**:
   - Success message
   - Download generated config file
   - Option to edit another schema

---

## API Testing

### Using curl

```bash
# Start server
./build-html/bin/configgui_server --port 8080

# In another terminal:

# 1. List available schemas
curl http://localhost:8080/api/schemas
# Response:
# [
#   { "id": "app_config", "title": "Application Configuration", ... },
#   { "id": "database_config", "title": "Database Configuration", ... }
# ]

# 2. Load a schema
curl -X POST http://localhost:8080/api/schema/app_config/load
# Response: Parsed schema with properties and validation rules

# 3. Validate a field
curl -X POST http://localhost:8080/api/validate \
  -H "Content-Type: application/json" \
  -d '{
    "field_name": "port",
    "value": 500,
    "schema_id": "app_config"
  }'
# Response: { "valid": false, "errors": [ { "message": "Port must be >= 1024" } ] }

# 4. Save configuration
curl -X POST http://localhost:8080/api/config/save \
  -H "Content-Type: application/json" \
  -d '{
    "schema_id": "app_config",
    "data": {
      "app_name": "MyApp",
      "port": 8080
    }
  }'
# Response: { "success": true, "config_path": "/configs/app_config.json" }
```

### Using Postman

1. **Import OpenAPI**:
   - Open Postman
   - Click "Import"
   - Select `specs/002-html-form-generation/contracts/openapi.yaml`
   - Postman auto-creates collection with all endpoints

2. **Test Endpoints**:
   - Set server URL to `http://localhost:8080`
   - Test each endpoint from collection
   - Verify request/response against specification

3. **Environment Setup**:
   ```json
   {
     "server": "http://localhost:8080",
     "schema_id": "app_config",
     "port": 8080
   }
   ```

---

## Development Workflow

### Phase 1: Core Extraction (Baseline)

1. **Verify existing Qt build works**:
   ```bash
   cmake -B build && cmake --build build --target ConfigGUI
   ./build/bin/ConfigGUI  # Should work unchanged
   ```

2. **Extract to src/core/**:
   - Move schema_loader.cpp/h to src/core/schema/
   - Move json_reader.cpp/h to src/core/io/
   - Update includes in Qt code
   - Re-test Qt build

3. **Create configgui_core library**:
   - Add library target to CMakeLists.txt
   - Qt links against library
   - Test: Qt app should run identically

### Phase 2: HTML Server Implementation

1. **Add HTTP server code**:
   ```
   src/html/
   ├── main.cpp
   ├── server.h/cpp
   ├── request_handler.h/cpp
   ├── form_service.h/cpp
   └── assets/
       ├── index.html
       ├── form.html
       └── js/
           ├── form-generator.js
           └── main.js
   ```

2. **Build HTML server target**:
   ```bash
   cmake -B build-html -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON
   cmake --build build-html
   ```

3. **Test HTML endpoints**:
   - Start server: `./build-html/bin/configgui_server`
   - Test main form in browser
   - Test API endpoints with curl

### Phase 3: Feature Complete

1. **Full integration test**:
   ```bash
   # Build both
   cmake -B build-both -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON
   cmake --build build-both
   
   # Run Qt app
   ./build-both/bin/ConfigGUI &
   
   # Run HTML server
   ./build-both/bin/configgui_server --port 8081 &
   
   # Test both apps with same schema
   ```

2. **Validation parity test**:
   - Fill form in Qt → Save config
   - Fill same form in HTML → Save config
   - Compare saved JSON files (should be identical)
   - Compare validation errors (should be identical)

3. **Performance benchmarks**:
   - Measure startup time: < 2 seconds
   - Measure form generation: < 1 second
   - Measure validation latency: < 200ms

---

## Troubleshooting

### Qt Build Issues

**Problem**: CMake can't find Qt6
```bash
# Solution: Specify Qt path
cmake -B build -DQt6_DIR=/opt/Qt6/lib/cmake/Qt6
```

**Problem**: Previous Qt files not rebuilding
```bash
# Solution: Clean build
rm -rf build
cmake -B build && cmake --build build
```

### HTML Server Issues

**Problem**: Port already in use
```bash
# Solution: Use different port
./build-html/bin/configgui_server --port 8081

# Or kill process on port 8080:
lsof -ti:8080 | xargs kill -9
```

**Problem**: Schemas directory not found
```bash
# Solution: Verify schemas path
ls -la ./resources/schemas/
# Should contain *.json or *.yaml files
```

**Problem**: cpp-httplib not found
```bash
# Solution: Download header manually
mkdir -p third_party
curl https://github.com/yhirose/cpp-httplib/releases/download/v0.14.0/httplib.h \
  -o third_party/httplib.h
# Add to CMakeLists.txt:
# include_directories(third_party)
```

### Validation Issues

**Problem**: Real-time validation too slow
```javascript
---

## Deployment Options

### Standalone Deployment

Run the compiled HTML server directly:

```bash
# Build HTML server
cmake -B build -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON
cmake --build build

# Create resources
mkdir -p resources/{schemas,configs}
cp /path/to/schemas/*.json resources/schemas/

# Run server
./build/src/html/ConfigGUIServer --port 8080 --host 0.0.0.0
```

**Access**: Open browser to `http://localhost:8080`

### Docker Deployment

Deploy in a containerized environment:

```bash
# Build Docker image
docker build -f Dockerfile.html -t configgui-html:3.0 .

# Run container
docker run -p 8080:8080 configgui-html:3.0

# Verify health
curl http://localhost:8080/health
```

### Kubernetes Deployment

For production-scale deployments:

```bash
# Apply Kubernetes manifest
kubectl apply -f k8s/configgui-deployment.yaml

# Verify
kubectl get pods -l app=configgui

# Port forward
kubectl port-forward svc/configgui-html 8080:80
```

**See**: `DEPLOYMENT.md` for complete deployment guide

---

## Production Checklist

Before deploying to production:

- [ ] Build with `-DCMAKE_BUILD_TYPE=Release`
- [ ] Run all unit tests (see Test section above)
- [ ] Verify health endpoint: `curl http://localhost:8080/health`
- [ ] Test with your schemas: `curl http://localhost:8080/api/schemas`
- [ ] Configure schema directory with real schemas
- [ ] Set resource limits (Docker/Kubernetes)
- [ ] Enable monitoring/logging
- [ ] Document deployment steps for your environment

---

## Validation Issues

**Problem**: Real-time validation too slow
```javascript
// Solution: Add debounce in form-generator.js
function debounce(func, delay) {
    let timeout;
    return function(...args) {
        clearTimeout(timeout);
        timeout = setTimeout(() => func.apply(this, args), delay);
    };
}

field.addEventListener('change', debounce(validateField, 200));
```

---

## Next Steps
```

---

## Next Steps

1. **Run Quick Test**:
   ```bash
   # Quick validation that everything compiles
   cmake -B build-quick \
     -DBUILD_QT_APP=OFF \
     -DBUILD_HTML_SERVER=ON
   cmake --build build-quick
   
   # Should complete without errors
   ```

2. **Review Changes**:
   - Check extracted code in src/core/
   - Verify includes in src/qt/ and src/html/
   - Ensure MISRA compliance with static analysis

3. **Run Tests**:
   - Unit tests for core library
   - Integration tests for HTML server
   - Contract tests for API endpoints

4. **Performance Benchmark**:
   - Measure startup times
   - Measure validation latency
   - Test with large schemas (1000+ properties)

---

## Key Files to Know

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Root build configuration |
| `src/core/schema/schema_loader.h` | Schema loading interface |
| `src/core/schema/schema_validator.h` | Validation logic (shared) |
| `src/html/server.h` | HTTP server implementation |
| `src/html/assets/js/form-generator.js` | Form generation logic |
| `specs/002-html-form-generation/contracts/openapi.yaml` | API specification |
| `resources/schemas/*.json` | Test schema files |

---

## Documentation References

- **Feature Spec**: `specs/002-html-form-generation/spec.md`
- **Implementation Plan**: `specs/002-html-form-generation/plan.md`
- **Research**: `specs/002-html-form-generation/research.md`
- **Data Model**: `specs/002-html-form-generation/data-model.md`
- **API Contract**: `specs/002-html-form-generation/contracts/openapi.yaml`

---

**Quickstart Ready**: Follow this guide to build, run, and test both Qt and HTML versions.
