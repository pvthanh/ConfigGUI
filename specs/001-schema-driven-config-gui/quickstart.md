# Developer Quickstart Guide

**Feature**: Schema-Driven Configuration GUI  
**Version**: 1.0  
**Updated**: 2025-10-20

## Table of Contents

1. [Prerequisites & Setup](#prerequisites--setup)
2. [Building the Project](#building-the-project)
3. [Running Tests](#running-tests)
4. [Project Structure Overview](#project-structure-overview)
5. [Key Concepts](#key-concepts)
6. [Common Development Tasks](#common-development-tasks)
7. [Debugging Tips](#debugging-tips)

---

## Prerequisites & Setup

### System Requirements

- **OS**: Linux, Windows 10+, or macOS 10.14+
- **Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **CMake**: Version 3.20 or later
- **Qt**: Version 6.2 or later

### Install Dependencies

#### On Ubuntu/Debian
```bash
sudo apt-get install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-declarative-dev \
    libyaml-dev \
    clang-tidy \
    doxygen
```

#### On macOS (with Homebrew)
```bash
brew install cmake qt yaml-cpp clang-format doxygen
```

#### On Windows
- Download Qt 6.x from https://www.qt.io/download
- Install CMake from https://cmake.org/download/
- Install Visual Studio 2019 or later
- Install libyaml via vcpkg: `vcpkg install yaml-cpp`

### Clone & Setup Repository

```bash
# Clone repository
git clone <repo-url> ConfigGUI
cd ConfigGUI

# Checkout feature branch
git checkout 001-schema-driven-config-gui

# Create build directory
mkdir build
cd build

# Configure CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Or with clang-tidy integration:
cmake -DCMAKE_CXX_CLANG_TIDY="clang-tidy;-header-filter=.;-checks=modernize-*,readability-*" ..
```

---

## Building the Project

### Initial Build

```bash
# From build/ directory
cmake --build . --config Debug

# Or with make directly
make -j$(nproc)
```

### Build Targets

```bash
# Build main application
cmake --build . --target ConfigGUI

# Build unit tests
cmake --build . --target test_validators

# Build all tests
cmake --build . --target all

# Build documentation
cmake --build . --target docs
```

### Clean Build

```bash
# Remove all build artifacts
rm -rf build/
mkdir build
cd build
cmake ..
cmake --build .
```

---

## Running Tests

### Unit Tests (Google Test)

```bash
# Run all unit tests
./tests/unit/test_validators
./tests/unit/test_schema_loader
./tests/unit/test_config_data_model

# Run specific test
./tests/unit/test_validators --gtest_filter="*ValidateField*"

# Run with verbose output
./tests/unit/test_validators --gtest_verbose
```

### Integration Tests (Qt Test)

```bash
# Run all integration tests
./tests/integration/test_schema_to_form
./tests/integration/test_config_load_save
./tests/integration/test_real_time_validation

# Run with output
./tests/integration/test_schema_to_form -v2
```

### Static Analysis (clang-tidy)

```bash
# Run MISRA C++ checks
cmake --build . --target clang-tidy

# Or manually
clang-tidy src/core/*.cpp -- -std=c++17 -I/usr/include/qt6

# Check specific violations
clang-tidy src/core/schema_loader.cpp -- \
    -checks="*" \
    -header-filter=".*" | grep -E "(MISRA|error)"
```

### Code Coverage

```bash
# Build with coverage
cmake -DCMAKE_CXX_FLAGS="--coverage" ..
cmake --build .
ctest

# Generate coverage report
gcovr --print-summary --html-details coverage.html
```

---

## Project Structure Overview

### Directory Layout

```
ConfigGUI/
├── src/                    # Source code
│   ├── main.cpp
│   ├── core/              # Business logic
│   ├── ui/                # Qt UI components
│   ├── models/            # Qt data models
│   ├── validators/        # Validation logic
│   ├── io/                # File I/O
│   └── utils/             # Utilities
│
├── tests/                 # Test code
│   ├── unit/
│   ├── integration/
│   └── CMakeLists.txt
│
├── resources/             # Data files
│   ├── schemas/
│   ├── configs/
│   └── qss/
│
├── docs/                  # Documentation
│   ├── ARCHITECTURE.md
│   └── CONTRIBUTING.md
│
├── CMakeLists.txt         # Build configuration
├── .clang-tidy            # MISRA C++ rules
├── .clang-format          # Code formatting
└── README.md
```

### Source Code Layers

**Core Layer** (`src/core/`)
- Schema loading and parsing
- Configuration data models
- File I/O abstractions

**UI Layer** (`src/ui/`)
- Qt widgets and main window
- Form generation from schema
- Widget factory

**Validators Layer** (`src/validators/`)
- Real-time field validation
- Constraint checking
- Plugin interface

**I/O Layer** (`src/io/`)
- JSON file reading/writing
- YAML file reading/writing
- Format detection

---

## Key Concepts

### 1. JSON Schema as Single Source of Truth

The JSON Schema file defines the entire form structure. No hardcoded UI:

```json
{
  "type": "object",
  "required": ["port"],
  "properties": {
    "port": {
      "type": "integer",
      "minimum": 1024,
      "maximum": 65535
    }
  }
}
```

This schema automatically generates a form with:
- One integer input field named "port"
- Spinner constrained to 1024-65535
- Required field marker

### 2. Model-View-Controller (MVC) Pattern

```
Data Layer (ConfigurationData)
    ↑
    ├─ Model (QAbstractItemModel subclass)
    │
    ├─ View (Qt Widgets)
    │  ├─ QLineEdit
    │  ├─ QSpinBox
    │  └─ QComboBox
    │
    └─ Delegate (Qt Delegates for custom rendering)
```

### 3. Real-Time Validation Flow

```
User edits field
    ↓
Qt signal (textChanged, valueChanged)
    ↓
FormGenerator slot (onFieldChanged)
    ↓
SchemaValidator (validateField)
    ↓
UI update (show/hide errors)
```

### 4. Signal/Slot Connections

All user interactions flow through Qt signals:

```cpp
connect(portSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &MainWindow::onPortChanged);
```

### 5. Smart Pointer Management

All objects use smart pointers (MISRA C++ requirement):

```cpp
std::unique_ptr<SchemaLoader> loader(new SchemaLoader());
auto result = loader->loadSchema("app.schema.json");
// Automatic cleanup when loader goes out of scope
```

---

## Common Development Tasks

### Adding Support for a New Schema Type

1. **Extend FieldType enum** (`src/ui/widget_factory.h`)
   ```cpp
   enum class FieldType {
       String,
       Integer,
       Number,
       Boolean,
       Array,
       Object,
       DateString,  // NEW
   };
   ```

2. **Add Widget Creator** (`src/ui/widget_factory.cpp`)
   ```cpp
   QWidget* WidgetFactory::createDateWidget(
       const nlohmann::json& schema,
       QWidget* parent) {
       
       auto* dateEdit = new QDateEdit(parent);
       // ... configure date widget
       return dateEdit;
   }
   ```

3. **Update Type Mapping** (in createWidget)
   ```cpp
   if (type == "date") {
       return createDateWidget(schema, parent);
   }
   ```

4. **Add Tests** (`tests/unit/test_widget_factory.cpp`)
   ```cpp
   TEST_F(WidgetFactoryTest, CreatesDateWidget) {
       nlohmann::json schema = {
           {"type", "date"}
       };
       // Test date widget creation
   }
   ```

### Adding Custom Validation

1. **Create Validator Plugin** (`src/validators/custom_validator.h`)
   ```cpp
   class CustomValidator : public ValidatorPlugin {
   public:
       bool canHandle(const nlohmann::json& schema) override;
       ValidationResult validate(
           const QString& field,
           const QVariant& value,
           const nlohmann::json& schema) override;
   };
   ```

2. **Register Plugin** (`src/validators/schema_validator.cpp`)
   ```cpp
   void SchemaValidator::loadPlugins() {
       registerPlugin(std::make_unique<CustomValidator>());
   }
   ```

### Adding a New File Format (TOML)

1. **Create Reader** (`src/io/toml_config_reader.h/cpp`)
   ```cpp
   class TomlConfigReader {
   public:
       Result<nlohmann::json, FileError> readToml(
           const QString& filePath) const;
   };
   ```

2. **Add to FileHandler** (`src/io/config_file_handler.h/cpp`)
   ```cpp
   Result<ConfigurationData, FileError> loadTomlConfig(
       const QString& filePath,
       const JSONSchema& schema);
   ```

3. **Update FileFormat Enum**
   ```cpp
   enum class FileFormat {
       JSON,
       YAML,
       TOML  // NEW
   };
   ```

---

## Debugging Tips

### Enable Debug Output

```cpp
// In main.cpp
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(configGUI, "configgui.app")

qDebug(configGUI) << "Loading schema:" << schemaPath;
```

```bash
# Set environment variable
export QT_LOGGING_RULES="configgui.app.debug=true"
./ConfigGUI
```

### Inspect JSON Data

```cpp
// Pretty-print JSON in debugger
std::cout << configData.dump(2) << std::endl;
```

### Qt Creator Debugging

1. **Open project** in Qt Creator
2. **Set breakpoints** by clicking left margin
3. **Run in Debug mode** (Ctrl+D)
4. **Inspect variables** in Locals panel
5. **Step through code** (F10 = step, F11 = step into, F5 = continue)

### Common Issues

| Problem | Solution |
|---------|----------|
| Qt not found | Set `Qt6_DIR` in cmake command |
| clang-tidy errors | Check `.clang-tidy` config, add exception marker |
| Validation not firing | Check signal connections in `connectWidgetSignals()` |
| Form not updating | Verify `markFieldDirty()` called |
| YAML parsing fails | Check indentation in YAML file |

### Performance Profiling

```bash
# Profile with perf (Linux)
sudo perf record -g ./ConfigGUI
sudo perf report

# Or use Qt Creator profiler
# Tools > Analyzer > QML Profiler (for UI performance)
```

---

## MISRA C++ Compliance Checking

### Before Committing Code

```bash
# Run full MISRA check
clang-tidy src/**/*.cpp -- \
    -std=c++17 \
    -I/usr/include/qt6 \
    -I/usr/include/nlohmann \
    | grep -v "^src" | sort | uniq
```

### Document Exceptions

```cpp
// MISRA C++ Exception: Qt signal/slot mechanism requires
// dynamic_cast, which violates RULE-6-4-2. This is necessary
// for proper Qt integration.
//
// Date: 2025-10-20
// Reviewer: [reviewer name]

connect(widget, &QLineEdit::textChanged,
        this, &MainWindow::onTextChanged);
```

---

## Code Review Checklist

Before submitting PR:

- [ ] Builds without errors: `cmake --build .`
- [ ] All tests pass: `ctest`
- [ ] No clang-tidy violations: `cmake --build . --target clang-tidy`
- [ ] Code formatted: `clang-format -i src/**/*.cpp`
- [ ] MISRA exceptions documented
- [ ] New code has unit tests
- [ ] Commit message explains "why", not just "what"

---

## Getting Help

### Documentation
- API Reference: `docs/API_REFERENCE.md`
- Architecture: `docs/ARCHITECTURE.md`
- Contributing: `docs/CONTRIBUTING.md`

### Common Resources
- Qt Documentation: https://doc.qt.io/
- JSON Schema: https://json-schema.org/
- YAML Spec: https://yaml.org/

### Debugging Resources
- Qt Creator Debugging: https://doc.qt.io/qtcreator/creator-debugging.html
- CMake Troubleshooting: https://cmake.org/cmake/help/latest/
- MISRA C++ Guidelines: Contact project maintainers

---

**Happy coding!** 🚀

