# Getting Started with ConfigGUI

**Version**: 2.0  
**Date**: October 28, 2025  
**Audience**: New users and developers

---

## Quick Start (5 minutes)

### For Desktop (Qt GUI)

```bash
# 1. Clone and build
git clone <repository-url>
cd configGui
mkdir build && cd build
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF ..
cmake --build . --parallel 4

# 2. Run the application
./src/qt/ConfigGUI

# 3. Load a schema
- Click "Load Schema"
- Select a schema file from resources/schemas/
- Form generates automatically

# 4. Load a configuration
- Click "Load Configuration"
- Select config.json, config.yaml, or config.ini
- Form pre-fills with values

# 5. Save configuration
- Edit form fields
- Click "Save Configuration"
- Choose format and location
```

### For Web Server

```bash
# 1. Build HTTP server
cd build
cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON ..
cmake --build . --parallel 4

# 2. Run the server
./src/html/ConfigGUIServer

# 3. Access in browser
# Open: http://localhost:8080

# 4. Upload schema and configuration
# Use the web UI to upload files
```

### Using Docker

```bash
# Build Docker image
docker build -f Dockerfile.html -t configgui-server .

# Run container
docker run -p 8080:8080 configgui-server

# Access
# Open: http://localhost:8080
```

---

## System Requirements

### Common
- **OS**: Linux, macOS, or Windows
- **CMake**: 3.16 or higher
- **C++ Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **Git**: For source control

### For Qt GUI
- **Qt**: 6.x (Core, Gui, Widgets, Test components)
- **Qt Development Headers**: Needed for compilation

### For HTTP Server
- **No Qt required** - runs standalone
- **Docker**: Optional (for containerized deployment)

### For Development
- **Google Test**: For running unit tests
- **Python 3.7+**: Optional (for test scripts)

---

## Installation Steps

### Step 1: Clone Repository

```bash
git clone https://github.com/pvthanh/ConfigGUI.git
cd configGui
```

### Step 2: Install Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-tools-dev \
    libyaml-dev \
    nlohmann-json3-dev
```

#### macOS
```bash
brew install cmake qt@6 yaml
```

#### Windows
- Install CMake: https://cmake.org/download/
- Install Qt 6: https://www.qt.io/download
- Install MSVC: Visual Studio Build Tools 2019+

### Step 3: Create Build Directory

```bash
mkdir build
cd build
```

### Step 4: Configure Project

**Option 1: Build Everything (Qt + Server)**
```bash
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON ..
```

**Option 2: Qt Only**
```bash
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF ..
```

**Option 3: Server Only**
```bash
cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON ..
```

### Step 5: Build

```bash
cmake --build . --parallel 4
```

### Step 6: Run Tests (Optional)

```bash
ctest --output-on-failure
```

---

## First Run: Step-by-Step

### Scenario 1: Using Qt GUI

1. **Launch Application**
   ```bash
   ./src/qt/ConfigGUI
   ```

2. **Load a Schema**
   - Click "📋 Load Schema" button
   - Browse to `resources/schemas/config.schema.json`
   - Form generates automatically from schema

3. **View Generated Form**
   - See form fields matching schema structure
   - Required fields marked with `*`
   - Sections are collapsible

4. **Load Existing Configuration**
   - Click "⚙️ Load Configuration" inside form
   - Select `config.json`, `config.yaml`, or `config.ini`
   - Form pre-fills with values

5. **Edit Values**
   - Modify form fields as needed
   - Changes auto-validate against schema

6. **Save Configuration**
   - Click "💾 Save Configuration"
   - Choose format: JSON, YAML, or INI
   - Choose save location
   - File saves to local PC and server backup

### Scenario 2: Using Web Server

1. **Start Server**
   ```bash
   ./src/html/ConfigGUIServer
   # Server listens on http://localhost:8080
   ```

2. **Open Browser**
   - Navigate to http://localhost:8080
   - See clean, responsive web interface

3. **Upload Schema**
   - Drag-drop or click to upload JSON schema
   - File format auto-detected
   - Form generates immediately

4. **Upload Configuration**
   - Click "Load Configuration in Form"
   - Upload JSON, YAML, or INI file
   - Form pre-populates

5. **Edit and Save**
   - Modify form values
   - Click "Save"
   - Choose format (JSON/YAML/INI)
   - File saves locally and to server

---

## Working with Different File Formats

### JSON Configuration

```json
{
  "database": {
    "host": "localhost",
    "port": 5432,
    "name": "myapp"
  },
  "logging": {
    "level": "info",
    "output": "file"
  }
}
```

**Load**: `File → Open → config.json`  
**Save**: Format Selection → JSON

### YAML Configuration

```yaml
database:
  host: localhost
  port: 5432
  name: myapp

logging:
  level: info
  output: file
```

**Load**: `File → Open → config.yaml`  
**Save**: Format Selection → YAML

### INI Configuration

```ini
[database]
host=localhost
port=5432
name=myapp

[logging]
level=info
output=file
```

**Load**: `File → Open → config.ini`  
**Save**: Format Selection → INI

---

## Supported Features

### Schema Features
- ✅ Nested objects with arbitrary depth
- ✅ Arrays of primitives and objects
- ✅ Object dictionaries (key-value pairs)
- ✅ Type validation (string, number, boolean, object, array)
- ✅ Enum/dropdown selections
- ✅ Min/Max value constraints
- ✅ Pattern validation with regex
- ✅ Default values for all types
- ✅ Required vs optional fields
- ✅ Nested section support in YAML/INI

### File Format Support
| Format | Load | Save | Notes |
|--------|------|------|-------|
| JSON | ✅ | ✅ | Primary format |
| YAML | ✅ | ✅ | Human-friendly |
| INI | ✅ | ✅ | Section-based |

### Storage Options
| Option | Details |
|--------|---------|
| Local PC | File save via OS file dialog |
| Server Backup | HTTP POST to /api/config |
| Hybrid | Both simultaneously (recommended) |

---

## Common Tasks

### Task: Load Multiple Configurations

```
1. Load Schema → Schema1 generates form
2. In form, click "Load Configuration in Form"
3. Modal opens with saved configurations
4. Select different configs to load/preview
5. Form updates with selected config data
```

### Task: Create New Configuration

```
1. Load Schema → Form appears
2. Form starts with default values
3. Edit all fields as needed
4. Click "Save"
5. Choose: Local PC | Server | Both
6. New configuration saved
```

### Task: Export Configuration in Different Format

```
1. Load schema and config
2. Form shows current data
3. Click "Save"
4. Choose INI format (to convert)
5. File saves in new format
```

### Task: Validate Configuration

```
1. Load schema
2. Load configuration (auto-validates on load)
3. Edit fields
4. Validation occurs on field change
5. Red highlight shows errors
6. Save only works if valid
```

---

## Troubleshooting

### Build Issues

**Problem**: Qt6 not found
```bash
# Solution: Specify Qt path
cmake -DCMAKE_PREFIX_PATH=/opt/qt6 ..
```

**Problem**: Missing nlohmann/json
```bash
# Solution: Install via package manager
sudo apt-get install nlohmann-json3-dev
```

**Problem**: CMake version too old
```bash
# Solution: Update CMake
cmake --version  # Check current
# Download and install 3.16+ from cmake.org
```

### Runtime Issues

**Problem**: "Failed to load schema"
- Check file path is correct
- Verify file is valid JSON schema
- Check file permissions (readable)

**Problem**: "Configuration doesn't match schema"
- Ensure config structure matches schema
- Check all required fields are present
- Validate types (string vs number, etc)

**Problem**: "Cannot save to local PC"
- Check file permissions in target directory
- Ensure disk space available
- Verify filename is valid

**Problem**: Server returns 403/404
- Check server is running: `curl http://localhost:8080`
- Verify port 8080 is not in use
- Check firewall settings

---

## Next Steps

- 📖 Read [USER_GUIDE.md](USER_GUIDE.md) for detailed features
- 🏗️ Review [DESIGN.md](DESIGN.md) to understand architecture
- 📚 Check [API_REFERENCE.md](API_REFERENCE.md) for C++ API details
- 🚀 See [DEPLOYMENT.md](DEPLOYMENT.md) for production setup

---

## Example: Complete Workflow

```
1. Start Application
   $ ./src/qt/ConfigGUI

2. Load Schema
   ✓ Select: resources/schemas/config.schema.json
   ✓ Form auto-generates with all defined fields

3. View Generated Form
   ✓ See nested objects (collapsible sections)
   ✓ See arrays with +/- buttons
   ✓ See validation indicators

4. Load Existing Config
   ✓ Click: "Load Configuration in Form"
   ✓ Select: resources/configs/sample.ini
   ✓ Form pre-fills with INI data

5. Edit Values
   ✓ Change database host
   ✓ Add logging rule
   ✓ See validation in real-time

6. Save Configuration
   ✓ Click: "Save Configuration"
   ✓ Choose: INI format
   ✓ Enter: "app_config.ini"
   ✓ Saves to: Local PC + Server

7. Verify
   ✓ File saved locally: ~/Downloads/app_config.ini
   ✓ Server backup: accessible via web UI
```

