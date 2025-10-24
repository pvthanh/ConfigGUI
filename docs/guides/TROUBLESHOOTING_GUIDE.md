# ConfigGUI Troubleshooting Guide

**Version**: 1.0.0  
**Date**: October 20, 2025  
**Purpose**: Common issues, solutions, and debugging techniques

## Table of Contents

1. [Build & Compilation Issues](#build--compilation-issues)
2. [Runtime Issues](#runtime-issues)
3. [Configuration Issues](#configuration-issues)
4. [Performance Issues](#performance-issues)
5. [UI/Display Issues](#uidisplay-issues)
6. [Debugging Techniques](#debugging-techniques)
7. [Known Limitations](#known-limitations)
8. [Getting Help](#getting-help)

---

## Build & Compilation Issues

### Issue: CMake Configuration Fails

**Symptoms**:
```
CMake Error: Qt6 not found
CMake Error: nlohmann/json not found
```

**Root Cause**: Missing dependencies or incorrect paths

**Solutions**:

1. **Verify Qt 6 installation**:
   ```bash
   qt6 --version
   # or check installation path
   locate qmake
   ```

2. **Install missing dependencies**:
   ```bash
   # Ubuntu/Debian
   sudo apt install qt6-base-dev qt6-tools-dev libyaml-dev
   
   # macOS
   brew install qt@6 libyaml
   
   # Windows (with vcpkg)
   vcpkg install qt6:x64-windows libyaml:x64-windows
   ```

3. **Specify Qt path explicitly**:
   ```bash
   cmake -DCMAKE_PREFIX_PATH=/path/to/qt6 ..
   ```

4. **Update CMakeLists.txt** if needed:
   ```cmake
   find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED PATHS /path/to/qt6)
   ```

---

### Issue: Compilation Errors - Undefined References

**Symptoms**:
```
undefined reference to `configgui::Logger::getInstance()'
undefined reference to `configgui::StringUtils::trim(const std::string&)'
```

**Root Cause**: Linking issues or missing implementation files

**Solutions**:

1. **Check all source files are included in CMakeLists.txt**:
   ```cmake
   add_library(configgui_core
       src/core/schema.cpp
       src/core/configuration_data.cpp
       src/utils/logger.cpp
       src/utils/string_utils.cpp
       # ... ensure all .cpp files are listed
   )
   ```

2. **Verify link libraries**:
   ```cmake
   target_link_libraries(configgui_app
       configgui_core
       Qt6::Core
       Qt6::Gui
       Qt6::Widgets
       nlohmann_json::nlohmann_json
   )
   ```

3. **Clean rebuild**:
   ```bash
   rm -rf build
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug
   make -j4
   ```

---

### Issue: Header File Not Found

**Symptoms**:
```
fatal error: 'core/schema.h' file not found
```

**Root Cause**: Include paths not configured correctly

**Solutions**:

1. **Check CMakeLists.txt includes source directory**:
   ```cmake
   target_include_directories(configgui_core PUBLIC
       ${CMAKE_CURRENT_SOURCE_DIR}/src
   )
   ```

2. **Verify file exists in correct location**:
   ```bash
   ls -la src/core/schema.h
   ```

3. **Clean and reconfigure CMake**:
   ```bash
   rm -rf build CMakeCache.txt
   cmake -B build
   cmake --build build
   ```

---

### Issue: C++ Standard Mismatch

**Symptoms**:
```
error: 'optional' is not a member of 'std'
error: 'std::any' was not declared
```

**Root Cause**: Compiler not using C++17 standard

**Solutions**:

1. **Specify C++17 in CMakeLists.txt**:
   ```cmake
   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   set(CMAKE_CXX_EXTENSIONS OFF)
   ```

2. **Verify compiler supports C++17**:
   ```bash
   g++ --version  # Must be >= 7.0
   clang --version  # Must be >= 5.0
   ```

3. **Explicitly enable C++17 during build**:
   ```bash
   cmake -DCMAKE_CXX_STANDARD=17 ..
   ```

---

---

## Runtime Issues

### Issue: Application Crashes on Startup

**Symptoms**:
```
Segmentation fault (core dumped)
SIGABRT: Aborted
```

**Root Cause**: Uninitialized objects, null pointer access, or dependency issue

**Solutions**:

1. **Run under debugger**:
   ```bash
   gdb ./configgui_app
   (gdb) run
   # Get backtrace on crash
   (gdb) bt
   ```

2. **Check initialization order**:
   - Ensure Logger is initialized before use
   - Verify Qt application created before widgets
   - Check static initializers don't depend on each other

3. **Enable debug logging**:
   ```cpp
   Logger::getInstance().debug("Starting application...");
   ```

4. **Valgrind memory check**:
   ```bash
   valgrind --leak-check=full ./configgui_app
   ```

---

### Issue: Memory Leaks Detected

**Symptoms**:
```
LEAK SUMMARY:
    definitely lost: 1,024 bytes in 2 blocks
    indirectly lost: 512 bytes in 1 blocks
```

**Root Cause**: Not releasing resources, dangling pointers, or incomplete cleanup

**Solutions**:

1. **Use smart pointers (MISRA compliant)**:
   ```cpp
   // ✓ Correct
   auto validator = std::make_unique<TypeValidator>();
   auto validators = std::make_shared<IValidator>();
   
   // ✗ Incorrect (memory leak)
   auto validator = new TypeValidator();  // No delete!
   ```

2. **Ensure proper cleanup**:
   ```cpp
   // ✓ RAII pattern
   {
       JsonReader reader;
       auto result = reader.readFile("config.json");
       // Automatically cleaned up when scope ends
   }
   ```

3. **Check for circular shared_ptr references**:
   ```cpp
   // ✓ Use weak_ptr to break cycles
   class Schema {
   private:
       std::weak_ptr<Validator> validator;  // Not shared_ptr!
   };
   ```

---

### Issue: Qt Application Fails to Create Main Window

**Symptoms**:
```
QWidget::setGeometry: Unable to set geometry
Failed to create platform plugin
```

**Root Cause**: Qt initialization issue or display server problem

**Solutions**:

1. **Verify Qt libraries are present**:
   ```bash
   ldd ./configgui_app | grep Qt
   ```

2. **Set display environment**:
   ```bash
   # Linux
   export DISPLAY=:0
   export QT_QPA_PLATFORM=xcb
   
   # Wayland (modern Linux)
   export QT_QPA_PLATFORM=wayland
   ```

3. **Run with platform plugin debugging**:
   ```bash
   ./configgui_app -platform offscreen  # Headless mode for testing
   ```

---

---

## Configuration Issues

### Issue: Schema Validation Always Fails

**Symptoms**:
```
Configuration failed validation
Error: Field 'username' does not match type 'string'
```

**Root Cause**: Schema mismatch or incorrect data format

**Solutions**:

1. **Validate schema first**:
   ```cpp
   try {
       auto schema = JSONSchema(json::parse(schema_str));
       auto props = schema.properties();
       std::cout << "Schema valid, properties: " << props.size() << std::endl;
   } catch (const std::exception& e) {
       std::cerr << "Invalid schema: " << e.what() << std::endl;
   }
   ```

2. **Check field types match schema**:
   ```json
   // Schema
   {"properties": {"port": {"type": "integer"}}}
   
   // ✓ Correct config
   {"port": 5432}
   
   // ✗ Wrong type
   {"port": "5432"}  // String instead of integer
   ```

3. **Enable validation debugging**:
   ```cpp
   auto errors = validator.validate(config, schema);
   for (const auto& error : errors) {
       std::cerr << "Field: " << error.path() << std::endl;
       std::cerr << "Error: " << error.message() << std::endl;
       std::cerr << "Code: " << error.code() << std::endl;
   }
   ```

---

### Issue: Cannot Load Configuration File

**Symptoms**:
```
Error: File not found [FILE_NOT_FOUND]
Error: Permission denied
```

**Root Cause**: File path issue or file permissions

**Solutions**:

1. **Check file exists and is readable**:
   ```bash
   ls -la config.json
   file config.json
   ```

2. **Use absolute path**:
   ```cpp
   // ✓ More reliable
   auto result = reader.readFile("/home/user/config.json");
   
   // ✗ May fail depending on working directory
   auto result = reader.readFile("config.json");
   ```

3. **Check file permissions**:
   ```bash
   # Make file readable
   chmod 644 config.json
   
   # Make directory readable/accessible
   chmod 755 configs/
   ```

4. **Handle file paths programmatically**:
   ```cpp
   #include "utils/file_utils.h"
   
   std::string config_path = "config.json";
   if (!fileutils::exists(config_path)) {
       std::cerr << "File not found: " << config_path << std::endl;
       // Use default or prompt user
   }
   ```

---

### Issue: JSON/YAML Parsing Error

**Symptoms**:
```
Error: Unexpected character 'x' at line 1, column 3
Error: JSON parse error
```

**Root Cause**: Malformed JSON/YAML syntax

**Solutions**:

1. **Validate JSON syntax**:
   ```bash
   # Using jq
   jq . config.json
   
   # Using Python
   python3 -m json.tool config.json
   ```

2. **Validate YAML syntax**:
   ```bash
   # Using yamllint
   yamllint config.yaml
   
   # Using Python
   python3 -c "import yaml; yaml.safe_load(open('config.yaml'))"
   ```

3. **Check common mistakes**:
   ```json
   // ✗ Missing quotes on keys
   {port: 5432}
   
   // ✓ Correct
   {"port": 5432}
   
   // ✗ Trailing comma
   {"host": "localhost", "port": 5432,}
   
   // ✓ Correct
   {"host": "localhost", "port": 5432}
   ```

4. **Pretty-print for debugging**:
   ```cpp
   JsonWriter writer;
   auto result = writer.toString(config);
   if (result.is_success()) {
       std::cout << std::move(result).value() << std::endl;  // Pretty printed
   }
   ```

---

### Issue: Pattern Validation Fails Unexpectedly

**Symptoms**:
```
Error: 'user@example.com' does not match pattern '...'
```

**Root Cause**: Regular expression doesn't match or special characters escaped incorrectly

**Solutions**:

1. **Test regex separately**:
   ```cpp
   #include <regex>
   
   std::string pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
   std::regex email_regex(pattern);
   std::string test_value = "user@example.com";
   
   if (std::regex_match(test_value, email_regex)) {
       std::cout << "Match!" << std::endl;
   } else {
       std::cout << "No match" << std::endl;
   }
   ```

2. **Check JSON escaping**:
   ```json
   // ✗ Backslashes need escaping in JSON
   {"pattern": "^[0-9]+\.[0-9]+$"}
   
   // ✓ Correct (double backslash)
   {"pattern": "^[0-9]+\\.[0-9]+$"}
   ```

3. **Use online regex tester**:
   - Visit [regex101.com](https://regex101.com)
   - Paste pattern and test values
   - Verify against ECMAScript dialect

---

---

## Performance Issues

### Issue: Application Runs Slowly

**Symptoms**:
- Configuration loading takes > 1 second
- UI is sluggish
- High CPU usage

**Root Cause**: Inefficient validation, many schema lookups, regex recompilation

**Solutions**:

1. **Profile application**:
   ```bash
   # Using Linux perf
   perf record -g ./configgui_app
   perf report
   
   # Using Valgrind
   valgrind --tool=callgrind ./configgui_app
   kcachegrind callgrind.out.*
   ```

2. **Check for repeated regex compilation**:
   ```cpp
   // ✗ Recompiles regex each time
   for (int i = 0; i < 1000; ++i) {
       std::regex pattern(regex_str);  // SLOW!
       if (std::regex_match(value, pattern)) { }
   }
   
   // ✓ Use PatternValidator (internally caches)
   PatternValidator validator;
   for (int i = 0; i < 1000; ++i) {
       auto result = validator.validate(value, schema);  // FAST!
   }
   ```

3. **Optimize schema lookups**:
   ```cpp
   // ✗ Slow for large schemas (O(n) each time)
   bool exists = false;
   for (auto it = schema["properties"].begin(); it != schema["properties"].end(); ++it) {
       if (it.key() == field_name) { exists = true; break; }
   }
   
   // ✓ Fast with optimized schema (O(1))
   if (schema.hasProperty(field_name)) { /* ... */ }
   ```

4. **Enable optimization builds**:
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   make -j4
   ```

5. **Check for blocking I/O**:
   ```cpp
   // ✗ Blocking file read blocks UI
   auto result = reader.readFile(large_file);
   
   // ✓ Move to background thread
   std::thread([](){ 
       auto result = reader.readFile(large_file);
       // Emit signal when done
   }).detach();
   ```

---

### Issue: Memory Usage Too High

**Symptoms**:
- Application uses > 500MB RAM
- Memory grows over time
- System becomes unresponsive

**Root Cause**: Large cache, inefficient data structures, or memory leaks

**Solutions**:

1. **Check memory usage**:
   ```bash
   # Real-time monitoring
   top -p $(pgrep -f configgui_app)
   
   # Detailed memory stats
   ps aux | grep configgui_app
   ```

2. **Profile memory allocation**:
   ```bash
   valgrind --tool=massif ./configgui_app
   ms_print massif.out.*
   ```

3. **Reduce regex cache size** (if needed):
   ```cpp
   // In pattern_validator.cpp, limit cache size
   const size_t MAX_CACHE_SIZE = 100;  // Limit patterns cached
   ```

4. **Clear unused cached data**:
   ```cpp
   // Periodically clear caches
   PatternValidator::clearCache();  // If implemented
   ```

---

### Issue: Validation Takes Too Long

**Symptoms**:
- Validating large configuration takes > 500ms
- Pattern validation is slow
- Schema lookups slow

**Root Cause**: Inefficient validators or large configuration

**Solutions**:

1. **Check optimization flags in CMakeLists.txt**:
   ```cmake
   # ✓ Good
   set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
   
   # ✗ Debug (slow)
   set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
   ```

2. **Use Release build for performance testing**:
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   ```

3. **Verify regex cache is working**:
   ```cpp
   // Add debug output
   std::cout << "Regex cache hits: " << cache_hits << std::endl;
   std::cout << "Regex cache misses: " << cache_misses << std::endl;
   
   // First few calls should show misses, then only hits
   ```

---

---

## UI/Display Issues

### Issue: Form Not Displaying Correctly

**Symptoms**:
- Fields not shown
- Layout broken
- Widgets overlapping

**Root Cause**: Schema issues, layout bugs, or Qt misconfiguration

**Solutions**:

1. **Verify schema has properties**:
   ```cpp
   if (schema.properties().is_object() && !schema.properties().empty()) {
       std::cout << "Schema has " << schema.properties().size() << " properties" << std::endl;
   } else {
       std::cerr << "Schema has no properties!" << std::endl;
   }
   ```

2. **Enable Qt debug logging**:
   ```cpp
   QLoggingCategory::setFilterRules("qt.qpa.*=true");
   ```

3. **Check for unsupported field types**:
   ```cpp
   // WidgetFactory handles: string, number, integer, boolean, array, object
   // Other types may need custom implementation
   ```

4. **Verify layout is set**:
   ```cpp
   auto form = generateForm(schema);
   // Ensure form has valid layout
   if (!form->layout()) {
       qWarning() << "Form has no layout!";
   }
   ```

---

### Issue: Validation Feedback Not Showing

**Symptoms**:
- Error messages not displayed
- Invalid fields not highlighted
- Status not updated

**Root Cause**: ValidationFeedbackWidget not updated or display issue

**Solutions**:

1. **Ensure feedback widget is connected**:
   ```cpp
   // After validation, call display method
   auto errors = validator.validate(config, schema);
   feedback_widget->setErrors(errors);
   ```

2. **Check stylesheet for error styling**:
   ```cpp
   // Ensure error label has visible styling
   error_label->setStyleSheet("color: red; font-weight: bold;");
   ```

3. **Verify error vector is not empty**:
   ```cpp
   if (!errors.empty()) {
       for (const auto& error : errors) {
           std::cerr << "Error: " << error.toString() << std::endl;
       }
       feedback_widget->setErrors(errors);
   }
   ```

---

### Issue: Qt Platform Theme Issues

**Symptoms**:
- Colors look wrong
- Fonts too small/large
- UI unreadable

**Root Cause**: Qt theme misconfiguration or high DPI issue

**Solutions**:

1. **Set Qt theme explicitly**:
   ```bash
   export QT_STYLE_OVERRIDE=fusion  # or 'windows', 'macOS'
   ./configgui_app
   ```

2. **Fix high DPI scaling**:
   ```bash
   export QT_QPA_ENABLE_DPI_SCALING=1
   export QT_QPA_PLATFORM_THEME=gnome  # or 'kde', 'windows'
   ```

3. **Set font size**:
   ```cpp
   QFont font = qApp->font();
   font.setPointSize(11);  // Adjust as needed
   qApp->setFont(font);
   ```

---

---

## Debugging Techniques

### Enable Debug Logging

```cpp
#include "utils/logger.h"

// Set logging level
#define ENABLE_DEBUG 1

Logger::getInstance().debug("Starting validation...");
Logger::getInstance().info("Configuration loaded: " + config.toJson());
Logger::getInstance().warn("Using deprecated field format");
Logger::getInstance().error("Validation failed for field: " + field_name);
```

### Build with Debug Symbols

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4
```

### Run Under Debugger

```bash
# GDB
gdb --args ./configgui_app config.json

# LLDB (macOS)
lldb -- ./configgui_app config.json

# Commands
(gdb) break main
(gdb) run
(gdb) step
(gdb) next
(gdb) print variable_name
(gdb) backtrace
```

### Add Debug Output

```cpp
#include <iostream>

std::cout << "DEBUG: Function entered" << std::endl;
std::cout << "DEBUG: Value = " << value << std::endl;
std::cout << "DEBUG: Path = " << path << std::endl;
```

### Use Assertions

```cpp
#include <cassert>

assert(config != nullptr && "Configuration must not be null");
assert(!schema.properties().empty() && "Schema must have properties");
```

### Profile with Callgrind

```bash
valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./configgui_app
kcachegrind callgrind.out
```

### Trace System Calls

```bash
strace ./configgui_app  # See system calls
strace -e trace=file ./configgui_app  # Only file operations
```

---

---

## Known Limitations

### 1. No Exception Support (MISRA Compliance)
- All errors use `Result<T, E>` pattern
- No try/catch blocks
- Must explicitly check for failures

### 2. Qt6 Dependency
- Only runs on systems with Qt6 installed
- Cannot use alternative GUI frameworks without refactoring
- Requires compatible compiler

### 3. Pattern Validation Regex Performance
- Regex compilation is expensive first time
- Cache helps, but very complex patterns may be slow
- Pattern validation only supports ECMAScript dialect

### 4. Large Schema Scalability
- Tested with schemas up to 1000 properties
- Very large schemas (>10,000 properties) untested
- Field indexing helps but still O(1) from construction

### 5. Circular References
- No support for circular JSON references
- Schema must have acyclic structure
- Cyclic data will cause infinite recursion

### 6. Unicode Support
- Standard ASCII/UTF-8 support
- No special handling for other encodings
- YAML reader supports UTF-8

### 7. Thread Safety
- Regex cache is thread-safe
- Other components assume single-threaded use
- Must serialize access to FormState and ConfigurationData

---

---

## Getting Help

### 1. Check Documentation
- Read [README.md](README.md) for general information
- See [API_REFERENCE.md](API_REFERENCE.md) for API details
- Review [DESIGN_DOCUMENT.md](DESIGN_DOCUMENT.md) for architecture

### 2. Enable Debug Output
```cpp
Logger::getInstance().debug("Enable debug logging to trace issues");
```

### 3. Reproduce Minimal Example
Create smallest possible example that shows the issue:
```cpp
#include "core/schema.h"
#include "core/configuration_data.h"

int main() {
    try {
        JSONSchema schema(json::parse(R"({"type": "object"})"));
        ConfigurationData config;
        // Reproduce issue here...
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
```

### 4. Check Issue Tracker
- Search existing issues on GitHub/GitLab
- Check PHASE_3_COMPLETION_REPORT.md for known issues

### 5. Review Test Cases
Look at test files for usage examples:
- `tests/unit/test_pattern_validator.cpp` - Pattern validation examples
- `tests/unit/test_schema.cpp` - Schema loading and validation
- `tests/integration/test_workflows.cpp` - Complete workflow examples

### 6. Contact Support
For additional help:
- Review `EXECUTIVE_SUMMARY.md` for project overview
- Check `FILE_INVENTORY.md` for all available files
- See `INDEX.md` for comprehensive documentation index

---

**Troubleshooting Guide v1.0.0**  
**Date**: October 20, 2025  
**Status**: Production Ready
