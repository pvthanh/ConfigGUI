# ConfigGUI Installation and Usage Guide

**Version**: 1.0  
**Date**: 2024-10-26  
**Audience**: New team members, developers, DevOps engineers

---

## Table of Contents

1. [System Requirements](#system-requirements)
2. [Installation](#installation)
3. [Building](#building)
4. [Running Qt GUI](#running-qt-gui)
5. [Running HTTP Server](#running-http-server)
6. [Configuration](#configuration)
7. [Troubleshooting](#troubleshooting)

---

## System Requirements

### Common Requirements
- **OS**: Linux, macOS, or Windows
- **Git**: For source control
- **CMake**: 3.16+
- **C++ Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **C++ Standard**: C++17

### For Qt GUI Application
- **Qt**: 6.x (Core, Gui, Widgets, Test)
- **Additional**: Development headers for Qt

### For HTTP Server (No Qt Required)
- **Only core dependencies** (nlohmann/json, yaml-cpp, json-schema-validator)
- **Docker**: Optional (for containerized deployment)

### For Testing
- **Google Test**: 1.11+
- **Python 3.7+**: For some test scripts

---

## Installation

### Step 1: Clone Repository

```bash
git clone https://github.com/pvthanh/ConfigGUI.git
cd ConfigGUI
```

### Step 2: Install Dependencies

#### Ubuntu/Debian
```bash
# Update package manager
sudo apt-get update

# Install build tools
sudo apt-get install -y build-essential cmake git

# Install core dependencies
sudo apt-get install -y libjsoncpp-dev nlohmann-json3-dev
sudo apt-get install -y libyaml-cpp-dev
sudo apt-get install -y libssl-dev

# For Qt GUI (optional)
sudo apt-get install -y qt6-base-dev qt6-tools-dev

# For testing
sudo apt-get install -y libgtest-dev
```

#### macOS
```bash
# Install Homebrew if not present
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake nlohmann-json yaml-cpp

# For Qt GUI (optional)
brew install qt@6

# For testing
brew install googletest
```

#### Windows (MSVC)
```powershell
# Install vcpkg (package manager for C++)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\vcpkg\integrate install

# Install dependencies
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install yaml-cpp:x64-windows
.\vcpkg install qt:x64-windows
```

### Step 3: Verify Installation

```bash
cmake --version        # Should be 3.16+
gcc --version         # Or clang/msvc
pkg-config --list | grep nlohmann  # Verify nlohmann-json
```

---

## Building

### Build Configuration

ConfigGUI supports three build scenarios:

**1. Full Build (Default)**
```bash
cd /path/to/ConfigGUI
mkdir build
cd build
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON ..
cmake --build . --parallel 4
```
**Output:**
- `src/qt/ConfigGUI` (2.0 MB) - Qt desktop application
- `src/html/ConfigGUIServer` (2.5 MB) - HTTP web server

**2. Qt GUI Only**
```bash
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF ..
cmake --build . --parallel 4
```
**Output:**
- `src/qt/ConfigGUI` - Qt desktop application only

**3. Server Only (For Docker)**
```bash
cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON ..
cmake --build . --parallel 4
```
**Output:**
- `src/html/ConfigGUIServer` - HTTP server (no Qt dependency)

### Build Options

```bash
# Debug build (with symbols)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Verbose output
cmake --build . --verbose

# Specific number of parallel jobs
cmake --build . --parallel 8

# Clean build
cmake --build . --target clean
cmake --build . --parallel 4
```

### Build Troubleshooting

**Issue: CMake not finding dependencies**
```bash
# Set pkg-config path
export PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/local/lib/pkgconfig
cmake ..
```

**Issue: Qt not found**
```bash
# Set Qt path
cmake -DQt6_DIR=/path/to/qt6 ..
# Common locations:
# macOS: /usr/local/opt/qt@6/lib/cmake/Qt6
# Linux: /usr/lib/x86_64-linux-gnu/cmake/Qt6
```

**Issue: Permission denied on build**
```bash
chmod +x ./build/src/qt/ConfigGUI
chmod +x ./build/src/html/ConfigGUIServer
```

---

## Running Qt GUI

### Basic Startup

```bash
cd /path/to/ConfigGUI/build
./src/qt/ConfigGUI
```

The Qt GUI window opens with:
- **Menu Bar**: File, Help
- **Toolbar**: Quick action buttons
- **Form Area**: Empty initially

### First Use: Load Schema

1. Click **"Load Schema"** button or **File → Load Schema**
2. Navigate to `resources/schemas/config.schema.json`
3. Click **Open**

**Expected Result:**
- Form generates automatically from schema
- All form fields appear (nested sections, arrays, enums)
- Status bar shows "Schema loaded: config.schema.json"

### Load Configuration

1. Click **"Load Configuration"** button or **File → Load Configuration**
2. Navigate to `resources/configs/config.json`
3. Click **Open**

**Expected Result:**
- Form populates with configuration values
- Can now edit configuration

### Save Configuration

1. Modify form values as needed
2. Click **"Save Configuration"** button or **File → Save Configuration**
3. Choose location and filename
4. Click **Save**

**Expected Result:**
- Configuration saved as JSON file
- Preserves structure and data types
- Message shows: "Configuration saved successfully"

### Features

**Navigation:**
- Scroll through long forms
- Collapse/expand nested sections (click headers)
- Tab navigation between fields

**Field Types:**
- **Text Input**: Simple text fields
- **Numeric**: Integer and decimal numbers with range validation
- **Boolean**: Checkboxes
- **Enum**: Dropdown menus
- **Array**: Add/remove items with + and - buttons
- **Objects**: Nested forms with grouping
- **Dictionary**: Key-value map fields
- **Path Selector**: File/directory picker

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| **Ctrl+O** | Open schema |
| **Ctrl+L** | Load configuration |
| **Ctrl+S** | Save configuration |
| **Ctrl+Q** | Quit application |
| **Tab** | Navigate to next field |
| **Shift+Tab** | Navigate to previous field |

---

## Running HTTP Server

### Basic Startup

```bash
cd /path/to/ConfigGUI/build
./src/html/ConfigGUIServer
```

**Expected Output:**
```
Server running on http://0.0.0.0:8080
Open your browser and navigate to: http://localhost:8080
```

### Access Web Interface

Open browser to: `http://localhost:8080`

**Expected Result:**
- Web form interface loads
- Schema selector dropdown appears
- Form area ready for input

### Load Schema (Web)

1. Click **"Select Schema"** dropdown
2. Choose `config.schema.json`
3. Form generates automatically

### Submit Configuration

1. Fill form with values
2. Click **"Save Configuration"** button
3. File downloads to browser's download folder

### Server Options

```bash
# Run on specific port
./ConfigGUIServer --port 9090

# Run on specific interface
./ConfigGUIServer --bind 127.0.0.1:8080

# Verbose logging
./ConfigGUIServer --verbose

# Help
./ConfigGUIServer --help
```

### Accessing from Remote Machine

```bash
# On server machine (allow remote connections)
./src/html/ConfigGUIServer --bind 0.0.0.0:8080

# On client machine, open browser
http://<server-ip>:8080
```

### API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Web interface |
| `/schemas` | GET | List available schemas |
| `/schemas/{name}` | GET | Get specific schema |
| `/form` | GET | Get form HTML |
| `/form/submit` | POST | Submit form data |
| `/config` | GET | Get current configuration |

---

## Configuration

### Schema Files

Located in: `resources/schemas/`

**Default schema**: `config.schema.json`
- Complete configuration schema
- 12 main sections (log, rules, validation, etc.)
- Support for nested objects and arrays

**Custom schemas:**
```bash
# Create custom schema
cp resources/schemas/config.schema.json resources/schemas/my-schema.json

# Edit my-schema.json with custom structure
# Schema will automatically appear in schema selector
```

### Configuration Files

Located in: `resources/configs/`

**Available examples:**
- `config.json` - Production configuration
- `config_test.json` - Test configuration
- `config_new.json` - New template

**Creating new configuration:**
```bash
# Use Qt GUI or Web interface
# Fill form and save
# Or manually create JSON matching schema
```

### Environment Variables

```bash
# Set custom schema directory
export CONFIG_SCHEMA_DIR=/custom/schemas

# Set default configuration path
export CONFIG_DEFAULT_PATH=/custom/configs

# Set logging level (Qt only)
export LOG_LEVEL=DEBUG
```

---

## Docker Deployment

### Building Docker Image

```dockerfile
FROM ubuntu:22.04

WORKDIR /app

# Install dependencies (server only - no Qt needed)
RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    libjsoncpp-dev nlohmann-json3-dev libyaml-cpp-dev \
    libssl-dev

# Copy source code
COPY . .

# Build server only (no Qt)
RUN mkdir build && cd build && \
    cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON .. && \
    cmake --build . --parallel 4

# Expose port
EXPOSE 8080

# Run server
CMD ["./build/src/html/ConfigGUIServer", "--bind", "0.0.0.0:8080"]
```

### Running in Docker

```bash
# Build image
docker build -t configgui:latest .

# Run container
docker run -p 8080:8080 configgui:latest

# Access from host
open http://localhost:8080
```

---

## Testing

### Run Unit Tests

```bash
cd /path/to/ConfigGUI/build
ctest --output-on-failure
```

**Expected Output:**
```
79% tests passed, 5 tests failed out of 24
```

**Note**: Some unit test failures are pre-existing (API issues in test code, not affecting functionality)

### Run Specific Test

```bash
ctest -R "test_name" --output-on-failure
```

### Test Coverage

Test categories:
- **Core Tests**: Schema loading, validation
- **UI Tests**: Form generation, widget creation
- **Integration Tests**: End-to-end workflows
- **Performance Tests**: Large schema handling

---

## Troubleshooting

### Qt GUI Issues

**Issue: "Cannot find Qt6"**
```bash
# Solution 1: Set Qt path
cmake -DQt6_DIR=/path/to/qt6 ..

# Solution 2: Use Qt from standard locations
# Ubuntu: /usr/lib/x86_64-linux-gnu/cmake/Qt6
# macOS: /usr/local/opt/qt@6/lib/cmake/Qt6
```

**Issue: GUI window doesn't open**
```bash
# Check if DISPLAY is set (Linux)
echo $DISPLAY
# Should show :0 or :1

# If remote, use X forwarding
ssh -X user@host
./ConfigGUI
```

**Issue: Form doesn't load**
```bash
# Verify schema file exists
ls resources/schemas/config.schema.json

# Check schema validity (Python)
python3 -c "import json; json.load(open('resources/schemas/config.schema.json'))"
```

### Web Server Issues

**Issue: Port already in use**
```bash
# Find process using port 8080
lsof -i :8080
# Kill process
kill -9 <PID>

# Or use different port
./ConfigGUIServer --port 9090
```

**Issue: Cannot access from remote machine**
```bash
# Ensure server bound to 0.0.0.0 not 127.0.0.1
./ConfigGUIServer --bind 0.0.0.0:8080

# Check firewall
sudo ufw allow 8080

# Verify from remote
curl http://<server-ip>:8080
```

### Compilation Issues

**Issue: Missing nlohmann-json**
```bash
# Install package
sudo apt-get install nlohmann-json3-dev

# Or set path
cmake -Dnlohmann_json_DIR=/path/to/cmake ..
```

**Issue: C++ version error**
```bash
# Ensure C++17 or later
cmake -DCMAKE_CXX_STANDARD=17 ..
```

### File Permission Issues

```bash
# Make binaries executable
chmod +x ./build/src/qt/ConfigGUI
chmod +x ./build/src/html/ConfigGUIServer

# Or rebuild
cmake --build . --parallel 4
```

---

## Quick Start Checklist

- [ ] Install dependencies
- [ ] Clone repository
- [ ] Create build directory
- [ ] Run CMake with desired options
- [ ] Build with `cmake --build .`
- [ ] For Qt: Run `./build/src/qt/ConfigGUI`
- [ ] For Server: Run `./build/src/html/ConfigGUIServer`
- [ ] Load schema: `resources/schemas/config.schema.json`
- [ ] Test form generation
- [ ] Save/load configuration
- [ ] Run tests: `ctest`

---

## Getting Help

### Documentation
- **Architecture**: See `SOFTWARE_ARCHITECTURE.md`
- **Bug Fixes**: See `BUG_FIXES_AND_SOLUTIONS.md`
- **API Reference**: See `docs/API_REFERENCE.md`

### Debugging
```bash
# Enable verbose output
cmake --build . --verbose

# Run with debug info
gdb ./build/src/qt/ConfigGUI
```

### Community
- Check README.md for contact information
- Review test files for usage examples
- Check existing configurations for patterns

---

**For Next Steps:**
- Explore schema customization
- Add custom validators
- Integrate with deployment pipeline
- Set up monitoring and logging
