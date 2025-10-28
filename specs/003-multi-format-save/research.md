# Phase 0 Research: Multi-Format Configuration Save

**Date**: October 27, 2025  
**Status**: Complete  
**Objective**: Resolve technical decisions and document best practices for implementation

---

## Research Topics

### 1. INI Parser/Serializer Library Selection

**Research Question**: Which C++ INI parser library is best suited for ConfigGUI's requirements?

**Evaluation Criteria**:
- MISRA C++ compliance (non-negotiable)
- Header-only or easily integrated
- Round-trip capable (read and write)
- Cross-platform support (Linux, Windows, macOS)
- Active maintenance and community
- Permissive license (Apache 2.0, MIT, BSD)

**Candidates Evaluated**:

| Library | Type | MISRA Compliant | Features | License | Notes |
|---------|------|-----------------|----------|---------|-------|
| **mINI** | Header-only | ✅ Mostly | Read/Write, simple API | MIT | Minimal deps, clean code |
| **INIReader** (boost-like) | Single header | ⚠️ Partial | Read-only, basic | Boost license | Solid for reading |
| **SimpleIni** | Header-only | ✅ | Read/Write, sections | MIT | Well-tested, compact |
| **libconfig** | Library | ⚠️ | Rich features | LGPL | Heavy, over-featured |

**Decision**: **mINI** (header-only INI library)
- Rationale: Header-only avoids external dependencies; simple API fits ConfigGUI's design; clean, MISRA-compatible code; MIT license
- Integration: Add `src/core/third_party/mINI/` as header-only library
- Risk: Verify serialization output matches standard Windows INI format

**Alternatives Considered**:
- Building custom INI serializer: Rejected - duplicates effort, adds maintenance burden
- Using libconfig: Rejected - LGPL license, over-complex for simple key-value format
- Using Boost Property Tree: Rejected - Boost dependency too heavy for single feature

---

### 2. JSON to INI Data Type Mapping Strategy

**Research Question**: How should complex JSON data types be represented in INI format?

**Challenge**: INI format is flat (sections and key-value pairs), but JSON supports nested objects and arrays.

**Mapping Strategy**:

#### Standard JSON-to-INI Conventions

| JSON Type | INI Representation | Example |
|-----------|-------------------|---------|
| **String** | Direct value | `name=John` |
| **Number** | String (parseable as number) | `count=42`, `ratio=3.14` |
| **Boolean** | `true`/`false` or `yes`/`no` | `enabled=true` |
| **Null** | Empty value or omitted | `optional=` |
| **Array** | Comma-separated values OR indexed keys | `colors=red,green,blue` OR `color.0=red, color.1=green` |
| **Object** | Dot-notation flattened keys | `server.host=localhost`, `server.port=8080` |
| **Nested Object** | Hierarchical dot notation | `db.primary.host=localhost`, `db.replica.host=remote` |

#### Implementation Approach (Dot-Notation Flattening)

```
JSON Input:
{
  "database": {
    "host": "localhost",
    "port": 5432,
    "credentials": {
      "user": "admin",
      "password": "secret"
    }
  },
  "features": ["auth", "logging", "caching"]
}

INI Output:
[database]
database.host=localhost
database.port=5432
database.credentials.user=admin
database.credentials.password=secret

[features]
features.0=auth
features.1=logging
features.2=caching
```

**Decision**: Use dot-notation flattening for nested structures and indexed notation for arrays.

- Rationale: Standard approach recognized by INI tools; preserves structure; reversible for round-trip conversion
- Implementation: Recursive flattening function in `ini_serializer.cpp`
- Round-trip: JSON → INI → JSON with identical structure validation

**Alternatives Considered**:
- Comma-separated arrays: Rejected - ambiguous with nested structures
- Multiple sections for objects: Rejected - loses hierarchy information
- Custom section prefixes: Rejected - non-standard, reduces compatibility

---

### 3. Error Handling & Filesystem Operations

**Research Question**: What error handling strategy is needed for file save operations?

**Common Failure Modes**:
1. File path invalid or inaccessible
2. Insufficient disk space
3. Permission denied (write access)
4. File already exists (overwrite confirmation)
5. Invalid configuration data (validation failure)

**Error Handling Strategy**:

```cpp
// Result<T> pattern used throughout (existing ConfigGUI convention)
Result<std::string> serialize_to_json(const ConfigData& config);
Result<std::string> serialize_to_ini(const ConfigData& config);

Result<void> write_file(const std::string& path, const std::string& content);
Result<void> save_configuration(const ConfigData& config, 
                                const std::string& path,
                                FormatType format);
```

**User-Facing Error Messages** (for display in UI):

| Error | Qt Message | HTML Message |
|-------|-----------|--------------|
| Invalid path | "File path is not accessible. Check permissions." | "File path is not accessible." |
| No disk space | "Insufficient disk space. Free up space and try again." | "Insufficient disk space." |
| Permission denied | "Permission denied. You don't have write access to this location." | "Permission denied." |
| File exists (overwrite) | Dialog: "File already exists. Overwrite?" | JS confirmation: "File exists. Overwrite?" |
| Validation failed | "Configuration contains errors and cannot be saved." | "Configuration is invalid." |

**Decision**: Use Result<T> for internal error propagation; user-friendly messages in UI layer

- Rationale: Consistent with ConfigGUI patterns; clear error taxonomy; UI handles presentation
- Implementation: Error variants for each failure mode with context
- Logging: All errors logged with severity levels (error level for user-visible issues)

---

### 4. Round-Trip Data Preservation Testing

**Research Question**: How should round-trip conversions be validated?

**Test Scenarios**:

1. **JSON → JSON** (baseline, should be 100% preservation)
2. **JSON → INI → JSON** (complex mapping verification)
3. **Various data types**: strings, numbers, booleans, nested objects, arrays
4. **Edge cases**: empty values, special characters, unicode, very long strings

**Validation Approach**:

```cpp
// Unit test pattern
TEST(RoundTripSerialization, JsonToIniToJson) {
    ConfigData original = create_complex_config();
    
    // JSON → INI
    auto ini_str = serialize_to_ini(original);
    auto parsed_ini = parse_ini_string(ini_str);
    
    // INI → JSON (reconstruct ConfigData)
    auto reconstructed = ini_to_config(parsed_ini);
    
    // Validation
    ASSERT_TRUE(validate_against_schema(reconstructed));
    ASSERT_EQ(original, reconstructed);  // Deep equality check
}
```

**Acceptance Criteria**:
- ✅ All primitive types preserved exactly
- ✅ Nested objects flatten and unflatten correctly
- ✅ Arrays map to indexed notation and back
- ✅ Round-trip JSON equals original JSON (not just equivalent)

**Decision**: Implement comprehensive round-trip tests in `tests/unit/test_serialization_round_trip.cpp`

---

### 5. Integration with Existing ConfigGUI Serialization

**Research Question**: How does new format support integrate with existing JSON serialization?

**Current Architecture** (from codebase review):
- Configurations loaded as JSON → validated against schema → populated in UI
- Save operation currently writes JSON only
- Validation layer is schema-aware and enforces constraints

**Integration Points**:

```
User loads config
  ↓
Parse JSON file
  ↓
Validate against schema ← Existing pipeline
  ↓
Populate GUI
  ↓
User modifies config (in-memory representation)
  ↓
User clicks Save
  ↓
[NEW] Format Selection Dialog ← Added
  ↓
JSON format?     INI format?
  ↓                 ↓
Use existing      [NEW] Serialize to INI
JSON serializer   (using new serializer)
  ↓                 ↓
Write JSON file  Write INI file
```

**Key Points**:
- In-memory representation remains unchanged (JSON-based)
- Format selection happens only at save time (no validation changes)
- Both formats write valid, loadable configurations
- Existing JSON load/validate/save path unaffected

**Decision**: New format serialization hooks into existing save workflow at output stage

---

### 6. Qt and HTML GUI Implementation Patterns

**Research Question**: What's the best UI pattern for format selection across both frontends?

#### Qt Desktop Application

**Pattern**: Modal dialog (blocking)
```cpp
FormatSelectionDialog dialog;
if (dialog.exec() == QDialog::Accepted) {
    FormatType format = dialog.selectedFormat();
    // Proceed with file save dialog
}
```

**Pros**:
- Standard Qt pattern, familiar to desktop users
- Clear user intent (explicit choice before action)
- Can include format descriptions/help text

**Cons**:
- Adds one more step in save workflow (acceptable per UX analysis)

#### HTML Web Application

**Pattern**: Modal dialog with radio buttons or dropdown
```html
<div id="formatModal" class="modal">
  <h3>Select Save Format</h3>
  <label>
    <input type="radio" name="format" value="json" checked> JSON (recommended)
  </label>
  <label>
    <input type="radio" name="format" value="ini"> INI Format
  </label>
  <button>Save</button>
  <button>Cancel</button>
</div>
```

**Pros**:
- Browser-native, consistent with web UX
- Can show format descriptions with icons
- Integrates with existing HTML UI framework

**Cons**:
- File download behavior differs from desktop save dialogs
- Browser may prompt for download location (user expected flow)

**Decision**: Modal dialog for Qt; modal dialog for HTML (with descriptive text explaining each format)

---

## Technology Stack Summary

### Core Library (No Qt)
- **Language**: C++17
- **INI Library**: mINI (header-only)
- **JSON**: nlohmann_json (existing)
- **Schema Validation**: json-schema-validator (existing)
- **Pattern**: Result<T> for error handling
- **Compliance**: MISRA C++ enforced

### Qt Application
- **Framework**: Qt 6.x (Core, Gui, Widgets)
- **Dialog Pattern**: QDialog modal for format selection
- **UI Integration**: Connect format dialog to existing save workflow

### HTML Application
- **Server**: cpp-httplib (existing)
- **Frontend**: ES6 JavaScript + HTML5
- **Format Modal**: Standard HTML dialog with form
- **Download**: Trigger browser download with appropriate MIME type

---

## Decisions Summary

| Decision | Choice | Rationale |
|----------|--------|-----------|
| **INI Library** | mINI (header-only) | MISRA-compliant, minimal dependencies, MIT license |
| **Type Mapping** | Dot-notation flattening + indexed arrays | Standard, reversible, preserves structure |
| **Error Handling** | Result<T> pattern + user-friendly UI messages | Consistent with ConfigGUI, clear error taxonomy |
| **Round-trip Validation** | Comprehensive unit tests + schema validation | Ensures data integrity per SC-001 and SC-002 |
| **Integration** | Hook into existing save workflow | Non-breaking, minimal changes to core |
| **Qt UI Pattern** | Modal dialog (QDialog) | Standard desktop pattern |
| **HTML UI Pattern** | Modal dialog (HTML + JS) | Browser-native, familiar web pattern |

---

## Implementation Readiness

**Phase 0 Complete**: ✅ All research questions answered
**Open Questions**: 0  
**Architecture Decisions**: 7  
**Technology Selections**: 7  
**Risk Assessment**: Low (well-understood problem domain, proven patterns)

**Ready for Phase 1**: ✅ YES - All NEEDS CLARIFICATION resolved, technical approach finalized

---

## References & Resources

- **mINI Library**: https://github.com/pulzed/mINI
- **INI Format Spec**: https://en.wikipedia.org/wiki/INI_file (Windows INI file format)
- **Dot-Notation Flattening**: https://docs.aws.amazon.com/systems-manager/latest/userguide/parameter-store-parameter-types.html (AWS reference implementation)
- **ConfigGUI Architecture**: See docs/SOFTWARE_ARCHITECTURE.md in repository
- **MISRA C++ Standard**: https://www.misra.org.uk/
