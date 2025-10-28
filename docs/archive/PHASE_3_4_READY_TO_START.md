# Phase 3-4 Quick Reference: Continuing Implementation

**Last Updated**: October 27, 2025  
**Session Status**: Phase 1-2 Complete ✅ | Phase 3 Ready to Start 🔄

---

## Current State

✅ **What's Done**:
- Core data models (FormatType, SerializationContext, SerializationResult)
- Serializer interface (FormatSerializer, SerializerFactory)
- I/O layer (ConfigurationReader, ConfigurationWriter)
- JSON serializer implemented and compiling
- INI serializer structure complete
- Test fixtures (config_complex.json, config_expected.ini)
- Build system: Dual-target CMake working

✅ **Compilation Status**: PASSING
```bash
cmake -B build -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON
cmake --build build --parallel 4
# Output: 100% success, ConfigGUI + ConfigGUIServer created
```

---

## Files Created (21 Total)

### Core Models
- `src/core/models/format_type.h` - FormatType enum, conversion helpers
- `src/core/models/format_type.cpp` - Implementation with MIME types
- `src/core/models/serialization_context.h` - Context struct with factory methods
- `src/core/models/serialization_result.h` - Result<T> template + error enum
- `src/core/models/serialization_result.cpp` - Error message strings

### Serializers
- `src/core/serializers/format_serializer.h` - Abstract base (5 pure virtuals)
- `src/core/serializers/serializer_factory.h` - Factory interface
- `src/core/serializers/serializer_factory.cpp` - Factory implementation
- `src/core/serializers/json_serializer.h` - JSON contract
- `src/core/serializers/json_serializer.cpp` - JSON implementation
- `src/core/serializers/ini_serializer.h` - INI contract
- `src/core/serializers/ini_serializer.cpp` - INI implementation (flatten/unflatten)

### I/O Layer
- `src/core/io/configuration_writer.h` - Atomic write interface
- `src/core/io/configuration_writer.cpp` - Temp file + atomic rename
- `src/core/io/configuration_reader.h` - Safe read interface
- `src/core/io/configuration_reader.cpp` - File reading + deserialization

### Test Data
- `tests/fixtures/config_complex.json` - Test configuration (7 levels deep)
- `tests/fixtures/config_expected.ini` - Expected INI output

### Build Configuration
- `CMakeLists.txt` - mINI FetchContent setup
- `src/core/CMakeLists.txt` - Module groups + linking

---

## What Needs To Happen Next

### Phase 3: User Story 1 - JSON Serializer (Tasks T022-T028)

**Task T022-T023: Unit Tests**
```bash
# File: tests/unit/test_json_serializer.cpp
# Tests:
# - Serialize simple types (strings, numbers, booleans)
# - Serialize complex types (nested, arrays)
# - Deserialize valid JSON
# - Detect and reject invalid JSON
# - Pretty-printing option
# - Error handling
```

**Task T024: Integration Tests**
```bash
# File: tests/integration/test_json_save_workflow.cpp
# Scenarios:
# - Load config → Save as JSON → Load again → Verify identical
# - Handle permissions denied errors
# - Handle disk full scenarios
```

**Task T028: Factory Registration**
```cpp
// In serializer_factory.cpp
case FormatType::JSON:
    return std::make_unique<JsonSerializer>(context);
```

### Phase 4: User Story 2 - INI Serializer (Tasks T029-T036)

**Task T029-T030: Unit Tests**
```bash
# File: tests/unit/test_ini_serializer.cpp
# Tests:
# - Flatten nested objects (db.host.name)
# - Array notation (.0, .1, .2)
# - Round-trip: JSON → INI → JSON
# - Special character escaping
# - Section organization
# - Valid INI syntax generation
```

**Task T031: Integration Tests**
```bash
# File: tests/integration/test_ini_save_workflow.cpp
# - Load JSON → Save as INI → Load INI → Compare flattened structure
# - Mixed nested objects and arrays
```

---

## Key Code Patterns (Already Implemented)

### Result<T> for Error Handling
```cpp
Result<std::string> serialize(const SerializationContext& context) {
    // If success:
    return "serialized content";
    
    // If error:
    return Result<std::string>::error(
        SerializationError::InvalidData,
        "Specific error message"
    );
}

// Usage:
auto result = serializer->serialize(context);
if (result) {
    std::cout << result.value() << std::endl;
} else {
    std::cerr << error_message(result.error_code()) << std::endl;
}
```

### Factory Pattern
```cpp
// Creating serializer by type:
auto serializer = SerializerFactory::create_serializer(FormatType::JSON, context);

// Creating serializer from file path:
auto serializer = SerializerFactory::create_serializer_from_path(
    "config.ini", context
);
```

### Atomic File Write
```cpp
ConfigurationWriter writer;
auto result = writer.write_file(
    "output.json",
    json_content,
    FormatType::JSON
);
if (!result) {
    // Error message in result.error_code()
}
```

---

## Test Command Reference

```bash
# Full build
cmake --build build --parallel 4

# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
ctest --test-dir build -R test_json_serializer -V

# Build only core library
cmake --build build --target ConfigGUICore

# Check compilation without running
cmake --build build --target all --verbose
```

---

## Directory Layout

```
src/core/
├── models/
│   ├── format_type.h              ✅ Enum & conversion helpers
│   ├── format_type.cpp            ✅ Implementation
│   ├── serialization_context.h    ✅ Context struct
│   ├── serialization_result.h     ✅ Result<T> template
│   └── serialization_result.cpp   ✅ Error messages
├── serializers/
│   ├── format_serializer.h        ✅ Abstract base
│   ├── serializer_factory.h       ✅ Factory interface
│   ├── serializer_factory.cpp     ✅ Factory impl
│   ├── json_serializer.h          ✅ JSON contract
│   ├── json_serializer.cpp        ✅ JSON impl (READY)
│   ├── ini_serializer.h           ✅ INI contract
│   └── ini_serializer.cpp         ✅ INI impl (READY)
└── io/
    ├── configuration_reader.h     ✅ Read interface
    ├── configuration_reader.cpp   ✅ Read impl
    ├── configuration_writer.h     ✅ Write interface
    └── configuration_writer.cpp   ✅ Write impl

tests/
├── unit/                          🔄 Phase 3+4
├── integration/                   🔄 Phase 3+4
└── fixtures/
    ├── config_complex.json        ✅ Test data
    └── config_expected.ini        ✅ Expected output
```

---

## Debugging Tips

**If serializer doesn't instantiate**:
```bash
# Check factory in serializer_factory.cpp
# Verify FormatType case statement includes all formats
# Ensure return statement creates std::unique_ptr<JsonSerializer>
```

**If JSON serialization fails**:
```cpp
// Enable error messages:
if (auto result = serializer->serialize(context)) {
    std::cout << result.value() << std::endl;
} else {
    std::cerr << error_message(result.error_code()) << std::endl;
    // SerializationError will have specific error code
}
```

**If INI flattening looks wrong**:
```cpp
// Check flatten_json() in ini_serializer.cpp
// Verify dot-notation keys are formed correctly
// Ensure array indices are properly formatted (.0, .1, etc.)
```

**If tests don't compile**:
```bash
# Verify test file includes proper headers:
#include "core/serializers/json_serializer.h"
#include "core/models/serialization_result.h"

# Ensure CMakeLists.txt in tests/ adds test executable
add_executable(test_json_serializer test_json_serializer.cpp)
target_link_libraries(test_json_serializer ConfigGUICore)
```

---

## Important Notes

### Do NOT
- ❌ Add Qt includes to core library files
- ❌ Use exceptions in core library (use Result<T> instead)
- ❌ Add heap allocations without documenting them
- ❌ Modify Result<T> template without verification
- ❌ Change FormatType enum values (breaks serialization)

### DO
- ✅ Keep all business logic in core library
- ✅ Use factory pattern for creating serializers
- ✅ Test error cases as thoroughly as success cases
- ✅ Document all public APIs with comments
- ✅ Run full build and tests before committing

---

## Build Verification Checklist

Before starting Phase 3:

- [ ] Run: `cmake --build build --parallel 4`
- [ ] Verify: No compilation errors
- [ ] Verify: ConfigGUI executable exists
- [ ] Verify: ConfigGUIServer executable exists
- [ ] Run: `ctest --test-dir build --output-on-failure`
- [ ] Verify: Existing tests pass

---

## Phase 3 Starting Point

When ready to implement Phase 3 (JSON Serializer tests):

1. Create `tests/unit/test_json_serializer.cpp`
2. Add test function: `TEST(JsonSerializerTest, SerializeSimpleTypes)`
3. Build and verify test compilation: `cmake --build build --target test_json_serializer`
4. Run test: `ctest --test-dir build -R test_json_serializer`
5. Repeat for remaining test cases

---

## Contact Information for Implementation

**Repository**: ConfigGUI (003-multi-format-save branch)  
**Build System**: CMake with dual-target support  
**Languages**: C++17 core, Qt6, ES6 JavaScript  
**Status**: Ready for Phase 3 ✅

**Last Build**: October 27, 2025 (SUCCESS)  
**Next Phase**: User Story 1 - JSON Serializer Unit Tests

---

**Generated By**: Implementation Engine  
**Time to Complete Phase 1-2**: ~3-4 hours  
**Estimated Time for Phase 3-4**: ~6-8 hours  
**Total Expected Implementation**: ~15-18 hours (Phases 1-8)
