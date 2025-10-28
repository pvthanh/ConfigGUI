# Qt GUI INI Loading Implementation Plan

## Overview
Implement INI configuration file loading for the Qt GUI application, mirroring the JavaScript implementation that's already working for the HTML interface.

## Architecture Overview

### Current Structure
- **Core Library**: `src/core/io/` - JSON/YAML readers and writers
- **Qt Application**: Uses core library for I/O operations
- **Pattern**: Reader classes inherit from base, use Result<T> for error handling

### Implementation Strategy
1. Create `INIParser` class in core/io/ to parse INI strings/files
2. Create `INIReader` class following existing pattern (JsonReader, YamlReader)
3. Create `INIWriter` class for writing INI files (inverse operation)
4. Update Qt GUI to use INIReader for loading config files
5. Integrate into file dialogs and config loading flow

## File Structure to Create

```
src/core/io/
├── ini_parser.h          # NEW: Core INI parsing logic
├── ini_parser.cpp        # NEW: Implementation
├── ini_reader.h          # NEW: Reader following pattern
├── ini_reader.cpp        # NEW: Implementation
└── ini_writer.h          # NEW: Writer for symmetry
    ini_writer.cpp        # NEW: Implementation

src/qt/
├── config_loader.h       # NEW or UPDATE: Load config with INI support
├── config_loader.cpp     # NEW or UPDATE
└── main_window.cpp       # UPDATE: Add INI file dialog handling
```

## Implementation Phases

### Phase 1: Core INI Parser (Foundation)
**File**: `src/core/io/ini_parser.h` and `ini_parser.cpp`

**Responsibilities**:
- Parse INI text into nested JSON structure
- Handle section headers: `[section]`
- Handle nested sections: `[section.subsection]`
- Type inference: booleans, numbers, strings
- Array notation: `key[0]=value`
- Escape sequence handling: `\n`, `\r`, `\t`
- Comment handling: `;` and `#`

**API**:
```cpp
class INIParser {
public:
    // Parse INI content to JSON
    static Result<json> parse(const std::string& ini_content);
    
    // Parse INI file
    static Result<json> parseFile(const std::string& file_path);
    
private:
    struct ParseContext {
        std::string current_section;
        json result;
        int line_number = 0;
    };
    
    // Helper methods
    static bool is_section_header(const std::string& line);
    static std::string extract_section_name(const std::string& line);
    static bool is_key_value_pair(const std::string& line);
    static std::pair<std::string, std::string> parse_key_value(const std::string& line);
    static nlohmann::ordered_json parse_value(const std::string& value);
    static void set_nested_value(json& obj, const std::string& path, const nlohmann::ordered_json& value);
};
```

### Phase 2: INI Reader Class (Consistency)
**File**: `src/core/io/ini_reader.h` and `ini_reader.cpp`

**Follows existing pattern**:
- Inherits/follows JsonReader pattern
- Uses Result<T> for error handling
- Provides static methods for file/string reading
- Clear error messages

**API**:
```cpp
class INIReader {
public:
    // Read INI from file
    static JsonResult readFile(const std::string& file_path);
    
    // Read INI from string
    static JsonResult readString(const std::string& ini_content);
    
private:
    INIReader() = default;
};
```

### Phase 3: INI Writer Class (Save Support)
**File**: `src/core/io/ini_writer.h` and `ini_writer.cpp`

**Purpose**: Write JSON back to INI format (inverse of parser)

**API**:
```cpp
class INIWriter {
public:
    // Write JSON to INI file
    static Result<std::string> writeFile(const std::string& file_path, const json& config);
    
    // Convert JSON to INI string
    static Result<std::string> toString(const json& config);
    
private:
    INIWriter() = default;
    static std::string serialize_value(const nlohmann::ordered_json& value);
};
```

### Phase 4: Qt Application Integration
**Update**: Qt GUI main window and config loading logic

**Changes Needed**:
1. Add INI file filter to file open dialog
2. Update config loading to detect file type
3. Use INIReader for `.ini` files
4. Reuse existing form population logic

**Code Location**: 
- `src/qt/main_window.cpp` - Add file dialog filter
- `src/qt/config_loader.cpp` (or equivalent) - Add file type detection

## Detailed Algorithm: INI Parser

### Input INI Example
```ini
[PxPoint]
LicenseFile = C:/license.lic
RootDirectory = C:/root/

[PxData]
NavteqGdxFfp = C:/navteq.gdx
UspsGdxFfp = C:/usps.gdx

[Arrays]
items[0] = first
items[1] = second

[Config]
debug = true
retries = 5
```

### Output JSON
```json
{
  "PxPoint": {
    "LicenseFile": "C:/license.lic",
    "RootDirectory": "C:/root/"
  },
  "PxData": {
    "NavteqGdxFfp": "C:/navteq.gdx",
    "UspsGdxFfp": "C:/usps.gdx"
  },
  "Arrays": {
    "items": ["first", "second"]
  },
  "Config": {
    "debug": true,
    "retries": 5
  }
}
```

### Parsing Steps
1. **Split** input into lines
2. **For each line**:
   - Skip empty lines and comments (`#`, `;`)
   - Check if section header `[section]` → update `current_section`
   - Check if key=value pair → parse value, prefix with section
   - Type infer value: `true`/`false` → bool, `123`/`3.14` → number, else string
   - Handle array indices: `key[0]` → create array structure
   - Build nested path: `section.key` or `section.array[0].property`
3. **Set** value in nested structure
4. **Return** result object

## Value Type Inference

```cpp
// String to value conversion
json parse_value(const std::string& value_str) {
    // Boolean
    if (value_str == "true") return true;
    if (value_str == "false") return false;
    
    // Number (integer)
    if (all_digits(value_str) && no_dots(value_str)) {
        return std::stoi(value_str);
    }
    
    // Number (float)
    if (valid_float(value_str)) {
        return std::stof(value_str);
    }
    
    // String (unescape special sequences)
    std::string unescaped = value_str;
    replace(unescaped, "\\n", "\n");
    replace(unescaped, "\\r", "\r");
    replace(unescaped, "\\t", "\t");
    return unescaped;
}
```

## Nested Value Setting

```cpp
// Path: "PxPoint.LicenseFile"
// Results in: { PxPoint: { LicenseFile: "..." } }
void set_nested_value(json& obj, const std::string& path, const json& value) {
    std::vector<std::string> parts = split(path, '.');
    
    json* current = &obj;
    for (size_t i = 0; i < parts.size() - 1; ++i) {
        if (!current->contains(parts[i])) {
            (*current)[parts[i]] = json::object();
        }
        current = &((*current)[parts[i]]);
    }
    
    (*current)[parts.back()] = value;
}

// Path: "Arrays.items[0]"
// Results in: { Arrays: { items: ["first", ...] } }
// Handled by regex matching on array indices
```

## Error Handling

```cpp
Result<json> parse(const std::string& ini_content) {
    try {
        json result;
        std::string current_section;
        
        for (auto& line : split(ini_content, '\n')) {
            // Validate syntax
            if (has_syntax_error(line)) {
                return error("Invalid INI syntax at line X: " + line);
            }
            
            // Parse...
        }
        
        return success(result);
    } catch (const std::exception& e) {
        return error(std::string("INI parse failed: ") + e.what());
    }
}
```

## Qt Integration Points

### 1. File Dialog Update
```cpp
// In main_window.cpp
QString filter = "Config Files (*.json *.yaml *.yml *.ini);;JSON Files (*.json);;YAML Files (*.yaml *.yml);;INI Files (*.ini);;All Files (*)";
QString filename = QFileDialog::getOpenFileName(this, "Open Configuration", "", filter);
```

### 2. File Type Detection
```cpp
// Determine which reader to use
if (filename.endsWith(".ini", Qt::CaseInsensitive)) {
    auto result = INIReader::readFile(filename.toStdString());
} else if (filename.endsWith(".json")) {
    auto result = JsonReader::readFile(filename.toStdString());
} else if (filename.endsWith(".yaml") || filename.endsWith(".yml")) {
    auto result = YamlReader::readFile(filename.toStdString());
}
```

### 3. Form Population
```cpp
// Reuse existing form population logic
if (result.is_ok()) {
    json config = result.value();
    populate_form_from_json(config, form_widgets);
    show_success_message("Configuration loaded from " + filename);
} else {
    show_error_message(result.error());
}
```

## Testing Strategy

### Unit Tests
Create `tests/unit/ini_parser_test.cpp`:
- Test section parsing
- Test nested sections
- Test arrays
- Test type inference
- Test escape sequences
- Test comments
- Test error cases

### Integration Tests
Create `tests/integration/ini_loading_test.cpp`:
- Load real INI files (e.g., Parcels.sample.ini)
- Verify JSON output structure
- Compare with expected output
- Test with Qt forms

### Manual Testing
1. Build Qt application with INI support
2. Load schema (JSON)
3. Load INI config file
4. Verify form fields populate
5. Edit and save back to INI
6. Reload and verify

## Dependencies

### Already Available
- `nlohmann/json` - JSON manipulation
- Qt libraries - File dialogs, UI
- Standard C++ library - String processing

### No New Dependencies Required
- INI parsing uses only standard C++

## Implementation Order

1. **Week 1**:
   - [ ] Create `ini_parser.h` with core logic
   - [ ] Implement `INIParser::parse()` 
   - [ ] Test with unit tests

2. **Week 2**:
   - [ ] Create `ini_reader.h` wrapper
   - [ ] Create `ini_writer.h` for symmetry
   - [ ] Integration tests

3. **Week 3**:
   - [ ] Update Qt file dialogs
   - [ ] Add file type detection
   - [ ] Qt manual testing
   - [ ] Bug fixes

4. **Week 4**:
   - [ ] Documentation
   - [ ] Performance optimization
   - [ ] Release

## File Size Estimates

- `ini_parser.cpp`: ~300-400 lines (core logic)
- `ini_reader.cpp`: ~80-100 lines (wrapper)
- `ini_writer.cpp`: ~150-200 lines (inverse operation)
- Test files: ~400-500 lines
- **Total: ~1000-1200 lines of C++ code**

## Success Criteria

✅ INI files parse correctly with section headers  
✅ Nested structures created properly  
✅ Type inference works (bool, number, string)  
✅ Array notation handled  
✅ Qt file dialog accepts `.ini` files  
✅ Config loads and populates form  
✅ INI can be saved and reloaded  
✅ Error messages clear and helpful  
✅ All unit tests passing  
✅ Integration tests passing  

## Comparison with JavaScript Implementation

| Feature | JavaScript | C++ Qt |
|---------|-----------|--------|
| Section detection | Regex `/^\[(.+)\]$/` | Similar regex |
| Path prefixing | String concatenation | String manipulation |
| Type inference | `isNaN()`, manual checks | Similar checks |
| Nested structure | Object with dot notation | JSON with string paths |
| Array handling | String split on `[` `]` | Regex and string parsing |
| Error handling | try/catch | Result<T> pattern |

---

## Next Steps

1. Create `ini_parser.h` and implement core parsing logic
2. Create `ini_reader.h` wrapper for consistency
3. Write comprehensive unit tests
4. Update Qt GUI file dialogs
5. Integrate into config loading flow
6. Manual testing with real INI files

Ready to start? Let me know which component to implement first!

