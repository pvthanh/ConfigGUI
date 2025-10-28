# Implementation Quickstart: Multi-Format Configuration Save

**Date**: October 27, 2025  
**Target**: Developers starting Phase 2 implementation  

---

## Quick Overview

This feature adds support for saving configurations in JSON or INI format. Users select their preferred format via a dialog when saving.

**Key Deliverables**:
1. Core serialization library (format-agnostic)
2. Qt GUI format selection dialog
3. HTML GUI format selection UI
4. Comprehensive tests for both formats

---

## Architecture at a Glance

```
┌─────────────────────────────────────────────────────────┐
│                  Configuration Data                      │
│           (JSON format, schema-validated)                │
└──────────────────────┬──────────────────────────────────┘
                       │
                ┌──────▼──────┐
                │ Format Dialog│
                │ (Qt or HTML) │
                └──────┬──────┘
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
    Serializer    Serializer    File Writer
    (JSON)        (INI)         (I/O)
        │              │              │
        └──────────────┼──────────────┘
                       │
                ┌──────▼──────┐
                │ Saved File  │
                │ (.json or   │
                │  .ini)      │
                └─────────────┘
```

---

## File Layout Reference

### New Files to Create

```
src/core/
├── models/
│   ├── format_type.h
│   ├── format_type.cpp
│   ├── serialization_context.h
│   └── serialization_result.h
│
├── serializers/
│   ├── format_serializer.h
│   ├── json_serializer.h
│   ├── json_serializer.cpp
│   ├── ini_serializer.h
│   ├── ini_serializer.cpp
│   ├── serializer_factory.h
│   └── serializer_factory.cpp
│
└── io/
    ├── configuration_writer.h
    ├── configuration_writer.cpp
    ├── configuration_reader.h
    └── configuration_reader.cpp

src/ui/
├── format_selection_dialog.h
└── format_selection_dialog.cpp

src/qt/
└── [modify existing main window to integrate dialog]

src/html/
├── handlers/
│   └── [modify save_handler to support formats]
└── [modify server.cpp for API integration]

resources/html/
├── format-selection.html
├── format-selection.js
└── format-selection.css

tests/unit/
├── test_json_serializer.cpp
├── test_ini_serializer.cpp
└── test_serialization_round_trip.cpp

tests/integration/
├── test_qt_format_dialog.cpp
├── test_html_format_selection.js
└── test_save_workflow_both_formats.cpp

tests/fixtures/
├── config_simple.json          # Test data
├── config_complex.json         # With nested objects/arrays
├── config_invalid.json         # For error testing
└── config_expected.ini         # Expected INI output
```

### Modified Files

```
CMakeLists.txt              # Add INI library dependency
src/CMakeLists.txt          # Add new source files
src/core/CMakeLists.txt     # Add serializer modules
src/qt/CMakeLists.txt       # Link new core modules
src/html/CMakeLists.txt     # Link new core modules
```

---

## Step-by-Step Implementation Path

### Phase 2A: Core Serialization Library

**Goal**: Implement JSON and INI serializers, factory pattern

**Tasks** (recommended order):

1. **Model Definitions**
   - Create `format_type.h` with FormatType enum
   - Create `serialization_context.h` with context struct
   - Create `serialization_result.h` with error types

2. **Base Class**
   - Create `format_serializer.h` (abstract interface)
   - Define all pure virtual methods

3. **JSON Serializer**
   - Create `json_serializer.h/cpp`
   - Implement serialize() using nlohmann_json (already available)
   - Implement deserialize()
   - Implement validation methods

4. **INI Serializer**
   - Create `ini_serializer.h/cpp`
   - Implement flattening algorithm (JSON → flat key-value map)
   - Implement unflattening algorithm (flat map → JSON)
   - Integrate mINI library for INI format
   - Handle escaping for special characters

5. **Factory**
   - Create `serializer_factory.h/cpp`
   - Implement factory pattern for creating serializers

6. **I/O Layer**
   - Create `configuration_writer.h/cpp` for file writing
   - Create `configuration_reader.h/cpp` for file reading
   - Implement error handling (permissions, disk space, etc.)

---

### Phase 2B: Qt GUI Integration

**Goal**: Add format selection dialog to Qt application

**Tasks**:

1. **Format Selection Dialog**
   - Create `src/ui/format_selection_dialog.h/cpp`
   - Implement QDialog with radio buttons (JSON/INI)
   - Add format descriptions/help text

2. **Integration with Save**
   - Modify `src/qt/main_window.cpp` (or appropriate save handler)
   - Show format dialog when Save is clicked
   - Get selected format from dialog
   - Call serializer and write file

3. **Error Handling UI**
   - Display error dialogs for common failures
   - Provide user-friendly error messages

---

### Phase 2C: HTML GUI Integration

**Goal**: Add format selection UI to web application

**Tasks**:

1. **Format Selection Modal**
   - Create `resources/html/format-selection.html`
   - Create `resources/html/format-selection.js`
   - Create `resources/html/format-selection.css`
   - Implement modal dialog with format options

2. **Save Handler Integration**
   - Modify `src/html/handlers/save_handler.cpp`
   - Add /api/save endpoint (or modify existing)
   - Accept format parameter
   - Return serialized content for download

3. **Client-Side Integration**
   - Call format selection modal when user clicks Save
   - POST selected format + config to server
   - Trigger download when response received

---

### Phase 2D: Testing

**Goal**: Comprehensive tests for all scenarios

**Unit Tests**:
- JSON serialization (with complex types)
- INI serialization (with flattening)
- INI deserialization (with unflattening)
- Round-trip conversions
- Error handling (invalid data, encoding issues)

**Integration Tests**:
- Qt dialog integration (show/cancel/select format)
- HTML modal integration (show/cancel/select format)
- End-to-end save workflows (both formats, both GUIs)
- Round-trip file save/load

**Test Fixtures**:
- Simple config (basic types)
- Complex config (nested objects, arrays)
- Invalid configs (missing required fields)
- Edge cases (unicode, special characters)

---

## Common Patterns & Examples

### Creating a Serializer

```cpp
// Get serializer for INI format
auto serializer = SerializerFactory::create_serializer(FormatType::INI);

// Create context
SerializationContext context;
context.config_data = current_config;  // nlohmann::json object
context.target_format = FormatType::INI;
context.file_path = "/path/to/config.ini";

// Serialize
auto result = serializer->serialize(context);

if (result.is_ok()) {
    std::cout << "Serialized: " << result.value() << std::endl;
} else {
    std::cerr << "Error: " << result.error().message << std::endl;
}
```

### Saving to File

```cpp
// Serialize first
auto serialization_result = serializer->serialize(context);
if (!serialization_result.is_ok()) {
    return serialization_result.error();
}

// Then write file
auto write_result = write_configuration_file(
    file_path,
    serialization_result.value(),
    true  // overwrite if exists
);

if (!write_result.is_ok()) {
    show_error(write_result.error().message);
}
```

### Qt Dialog Integration

```cpp
// In save button handler:
FormatSelectionDialog dialog(this);
if (dialog.exec() == QDialog::Accepted) {
    FormatType format = dialog.selected_format();
    
    // Open file save dialog
    QString file_path = QFileDialog::getSaveFileName(
        this,
        "Save Configuration",
        "",
        (format == FormatType::JSON ? "JSON files (*.json)" : "INI files (*.ini)")
    );
    
    if (!file_path.isEmpty()) {
        // Perform serialization and save
        save_configuration(
            current_config,
            file_path.toStdString(),
            format
        );
    }
}
```

### HTML Format Selection

```javascript
// In save button handler:
const formatter = new FormatSelector();
const format = await formatter.show();

if (format) {
    // POST to server
    const response = await fetch('/api/save', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            config: currentConfig,
            format: format,
            filename: `config.${format}`
        })
    });
    
    if (response.ok) {
        const blob = await response.blob();
        download(blob, `config.${format}`);
    }
}
```

---

## Key Technical Decisions

### 1. INI Flattening Approach

**Decision**: Use dot-notation for nested keys

```
JSON:  { "db": { "host": "localhost" } }
INI:   db.host=localhost
```

**Rationale**: Standard, reversible, preserves structure

### 2. Error Handling

**Decision**: Use Result<T> pattern (existing ConfigGUI style)

**Example**:
```cpp
Result<std::string> serialize(...);
Result<void> write_file(...);
```

### 3. Factory Pattern

**Decision**: SerializerFactory for creating format-specific instances

**Benefit**: Adding new formats doesn't require modifying client code

### 4. Dual Implementations

**Decision**: Separate Qt and HTML implementations, shared core library

**Benefit**: Format logic shared, UI implementations independent

---

## Testing Checklist

### Unit Tests

- [ ] JSON serializer produces valid JSON
- [ ] INI serializer produces valid INI
- [ ] Flattening preserves all data
- [ ] Unflattening reconstructs JSON correctly
- [ ] Round-trip JSON→INI→JSON identical
- [ ] Error handling for invalid inputs
- [ ] Special character escaping/unescaping
- [ ] Large configuration handling (> 1MB)
- [ ] Empty configuration handling

### Integration Tests

- [ ] Qt dialog appears and responds to selections
- [ ] Qt file save dialog integration
- [ ] HTML modal appears and responds
- [ ] HTML file download triggered
- [ ] Error dialogs displayed (Qt)
- [ ] Error messages returned (HTML)
- [ ] Full save workflow (dialog → serialize → write → success)
- [ ] Cancel at dialog step
- [ ] Overwrite existing file handling

### End-to-End Tests

- [ ] Load config → modify → save as JSON → load → validate
- [ ] Load config → modify → save as INI → load → validate
- [ ] Save same config in both formats → compare structures
- [ ] Very large config (stress test)
- [ ] Special characters and unicode
- [ ] Permissions errors (mock)
- [ ] Disk full errors (mock)

---

## Dependencies & Setup

### Required Dependencies

- **mINI Library**: Header-only, add to `src/core/third_party/mINI/`
- **nlohmann_json**: Already available
- **json-schema-validator**: Already available
- **Qt6**: Already available (for Qt GUI)
- **cpp-httplib**: Already available (for HTML server)

### CMakeLists Setup

```cmake
# In src/core/CMakeLists.txt:
add_library(ConfigGUICore STATIC
    # ... existing sources ...
    serializers/format_serializer.cpp
    serializers/json_serializer.cpp
    serializers/ini_serializer.cpp
    serializers/serializer_factory.cpp
    io/configuration_writer.cpp
    io/configuration_reader.cpp
)

# No new external dependencies needed (mINI is header-only)
```

---

## Common Pitfalls & Solutions

| Pitfall | Solution |
|---------|----------|
| Forgetting to validate before serializing | Always call validate_pre_serialization() first |
| INI special character issues | Use escape/unescape helpers for values |
| Circular references in JSON | Schema validation prevents these |
| File write failures not propagated | Always check Result<T>.is_ok() |
| Round-trip data loss | Use indexed arrays (not comma-separated) |
| MISRA C++ compliance | Use clang-tidy: `clang-tidy -checks=* src/core/` |

---

## Performance Tips

1. **Serialization**: Pre-allocate strings for large configs
2. **INI Parsing**: Use mINI's streaming capabilities for large files
3. **File I/O**: Write to temp file, then atomic rename
4. **Caching**: Cache serializers (they're stateless and expensive to create)
5. **Validation**: Skip redundant validation if config already validated

---

## Debugging Hints

### Debug JSON Serialization
```cpp
std::cout << serialized_content << std::endl;
// Pretty-print to inspect structure
```

### Debug INI Flattening
```cpp
for (const auto& [key, value] : flattened_map) {
    std::cout << key << "=" << value << std::endl;
}
```

### Debug Round-Trip
```cpp
// Serialize to INI
auto ini_str = serialize_to_ini(original_json);

// Deserialize back to JSON
auto reconstructed = parse_ini_to_json(ini_str);

// Compare
if (original_json != reconstructed) {
    std::cout << "Mismatch!" << std::endl;
    std::cout << "Original: " << original_json.dump() << std::endl;
    std::cout << "Reconstructed: " << reconstructed.dump() << std::endl;
}
```

---

## Next Steps

1. **Review** this quickstart and `data-model.md`
2. **Setup** CMakeLists with mINI dependency
3. **Implement** core serializers (Phase 2A)
4. **Test** serialization layer thoroughly
5. **Add** Qt GUI dialog (Phase 2B)
6. **Add** HTML GUI modal (Phase 2C)
7. **Integration** test both frontends
8. **Documentation** update (post-testing per requirements)

---

## See Also

- `spec.md` - Feature specification
- `research.md` - Technical research
- `data-model.md` - Data structures
- `contracts/serialization-api.md` - API contracts
- `plan.md` - Implementation plan overview
