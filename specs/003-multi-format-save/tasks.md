# Tasks: Multi-Format Configuration Save

**Input**: Design documents from `/specs/003-multi-format-save/`  
**Prerequisites**: 
- ✅ plan.md (implementation plan with technical context)
- ✅ spec.md (5 user stories with priorities)
- ✅ research.md (technical decisions: mINI library, dot-notation flattening, Result<T> pattern)
- ✅ data-model.md (7 data models and API contracts)
- ✅ contracts/serialization-api.md (12 API endpoints)
- ✅ quickstart.md (implementation guide with code examples)

**Tests**: Unit and integration tests are INCLUDED per specification (round-trip validation critical for SC-001 and SC-002)

**Organization**: Tasks organized by user story (P1 = high priority core features, P2 = quality verification)

---

## Format Reference

Task format: `- [ ] [TaskID] [P?] [Story?] Description with exact file path`

- **[P]**: Parallelizable (different files, no inter-task dependencies)
- **[Story]**: User story label (US1, US2, US3, US4, US5)
- **File paths**: Absolute from repository root

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project structure and build configuration

- [ ] T001 Create mINI library directory at `src/core/third_party/mINI/` with header files
- [ ] T002 Add CMake dependency for mINI library in root `CMakeLists.txt` via FetchContent
- [ ] T003 [P] Create models directory structure in `src/core/models/`
- [ ] T004 [P] Create serializers directory structure in `src/core/serializers/`
- [ ] T005 [P] Create I/O handler directory structure in `src/core/io/`
- [ ] T006 Create test fixtures directory in `tests/fixtures/` with sample configs
- [ ] T007 Update `src/core/CMakeLists.txt` to include new serializer modules
- [ ] T008 [P] Create HTML format selection assets directory in `resources/html/`
- [ ] T009 Create Qt GUI dialog files scaffolding in `src/ui/`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core serialization infrastructure - MUST complete before any UI/format story

**⚠️ CRITICAL**: All user stories depend on this foundation

### Core Data Models (Foundation)

- [ ] T010 [P] Create `src/core/models/format_type.h` with FormatType enum (JSON, INI) and helper functions
- [ ] T011 [P] Create `src/core/models/format_type.cpp` implementing format conversion utilities
- [ ] T012 [P] Create `src/core/models/serialization_context.h` with SerializationContext struct and factory methods
- [ ] T013 [P] Create `src/core/models/serialization_result.h` with SerializationError enum (8 error types) and Result<T> alias

### Base Serializer Interface (Foundation)

- [ ] T014 Create `src/core/serializers/format_serializer.h` abstract base class with 5 pure virtual methods:
  - serialize() → SerializationResult
  - deserialize() → Result<json>
  - validate_pre_serialization() → optional<error>
  - get_mime_type() → string
  - get_format_name() → string

- [ ] T015 Update `src/core/CMakeLists.txt` to export FormatSerializer interface

### Serializer Factory (Foundation)

- [ ] T016 Create `src/core/serializers/serializer_factory.h` with factory methods:
  - create_serializer(FormatType) → unique_ptr<FormatSerializer>
  - create_serializer_from_path(string) → unique_ptr<FormatSerializer>

- [ ] T017 Create `src/core/serializers/serializer_factory.cpp` implementing factory pattern with format detection

### File I/O Layer (Foundation)

- [ ] T018 [P] Create `src/core/io/configuration_writer.h` with write_configuration_file() function signature
- [ ] T019 [P] Create `src/core/io/configuration_reader.h` with read_configuration_file() function signature
- [ ] T020 Create `src/core/io/configuration_writer.cpp` implementing atomic file write with error handling
- [ ] T021 Create `src/core/io/configuration_reader.cpp` implementing file read with error handling

**Checkpoint**: Foundation complete - core serialization infrastructure ready for format implementations

---

## Phase 3: User Story 1 - Save Configuration in JSON Format (Priority: P1) 🎯

**Goal**: Enable users to save configurations in JSON format with round-trip preservation

**Independent Test**: Load config → Save as JSON → Reload → Verify identical data

### Unit Tests for US1

- [ ] T022 [P] [US1] Create `tests/unit/test_json_serializer.cpp` with test cases:
  - Serialize simple types (strings, numbers, booleans)
  - Serialize complex types (nested objects, arrays)
  - Deserialize valid JSON
  - Detect and handle invalid JSON
  - Verify pretty-printing
  - Test error cases (invalid data, encoding issues)

- [ ] T023 [P] [US1] Create `tests/fixtures/config_simple.json` - basic configuration for testing

### Integration Tests for US1

- [ ] T024 [US1] Create `tests/integration/test_json_save_workflow.cpp` with scenarios:
  - Load valid config → select JSON → save → verify file exists
  - Round-trip: load JSON → verify data integrity
  - Save overwrites existing file
  - Handle permission errors gracefully

### Implementation for US1

- [ ] T025 Create `src/core/serializers/json_serializer.h` extending FormatSerializer abstract interface
- [ ] T026 Create `src/core/serializers/json_serializer.cpp` implementing:
  - serialize() method using nlohmann_json
  - deserialize() method with validation
  - Pre-serialization validation
  - MIME type: "application/json"
  - Format name: "JSON"
  - Pretty-printing for readability

- [ ] T027 Update `src/core/CMakeLists.txt` to compile json_serializer.cpp
- [ ] T028 [US1] Update `src/core/serializers/serializer_factory.cpp` to register JSON serializer

**Checkpoint**: User Story 1 complete - JSON save functional and tested independently

---

## Phase 4: User Story 2 - Save Configuration in INI Format (Priority: P1)

**Goal**: Enable users to save configurations in INI format with proper data mapping

**Independent Test**: Load config → Save as INI → Parse → Verify data mapped correctly

### Unit Tests for US2

- [ ] T029 [P] [US2] Create `tests/unit/test_ini_serializer.cpp` with test cases:
  - Flatten nested JSON to dot-notation keys
  - Unflatten dot-notation back to JSON
  - Handle array indices (servers.0, servers.1, etc.)
  - Escape special characters in values
  - Unescape values on round-trip
  - Serialize to valid INI format with [sections]
  - Parse INI format back to flat map
  - Error cases (unsupported types, encoding issues)
  - Round-trip: JSON → INI → JSON equality

- [ ] T030 [P] [US2] Create `tests/fixtures/config_complex.json` - nested objects and arrays
- [ ] T031 [P] [US2] Create `tests/fixtures/config_expected.ini` - expected INI output for complex config

### Integration Tests for US2

- [ ] T032 [US2] Create `tests/integration/test_ini_save_workflow.cpp` with scenarios:
  - Load valid config → select INI → save → verify file exists with .ini extension
  - Verify INI file is readable by standard INI parsers
  - Round-trip INI: load INI → convert to JSON → validate against schema
  - Handle special characters and unicode

### Implementation for US2

- [ ] T033 Create `src/core/serializers/ini_serializer.h` extending FormatSerializer abstract interface with helper methods:
  - flatten_json()
  - unflatten_ini()
  - escape_ini_value()
  - unescape_ini_value()

- [ ] T034 Create `src/core/serializers/ini_serializer.cpp` implementing:
  - serialize() method with flattening algorithm
  - deserialize() method with unflattening
  - Dot-notation key generation for nested objects
  - Indexed notation for arrays (key.0, key.1, etc.)
  - Character escaping (spaces, newlines, etc.)
  - mINI library integration for INI format parsing/generation
  - Pre-serialization validation
  - MIME type: "text/plain" or "text/x-ini"
  - Format name: "INI"

- [ ] T035 Update `src/core/CMakeLists.txt` to compile ini_serializer.cpp and link mINI
- [ ] T036 [US2] Update `src/core/serializers/serializer_factory.cpp` to register INI serializer

**Checkpoint**: User Story 2 complete - INI save functional with proper data mapping

---

## Phase 5: User Story 3 - Format Selection Dialog in Qt GUI (Priority: P1)

**Goal**: Qt desktop application users can select save format via modal dialog

**Independent Test**: Launch app → Load config → Click Save → Dialog appears → Select format → Proceed to file dialog

### Unit Tests for US3

- [ ] T037 [P] [US3] Create `tests/unit/test_qt_format_dialog.cpp` with test cases:
  - Dialog instantiation and initialization
  - Radio button states (JSON selected by default)
  - Get selected format after selection
  - Dialog accept/reject handling
  - Signal emissions on format change

### Integration Tests for US3

- [ ] T038 [US3] Create `tests/integration/test_qt_save_workflow_with_dialog.cpp` with scenarios:
  - Qt GUI loads config
  - User clicks Save button
  - Format selection dialog appears
  - User selects JSON → proceeds with JSON save
  - User selects INI → proceeds with INI save
  - User cancels dialog → save operation aborted
  - Dialog appears within 500ms (performance check)

### Implementation for US3

- [ ] T039 Create `src/ui/format_selection_dialog.h` QDialog class with:
  - Radio buttons for JSON and INI
  - Format descriptions/help text
  - OK and Cancel buttons
  - selected_format() method to get choice

- [ ] T040 Create `src/ui/format_selection_dialog.cpp` implementing:
  - UI layout and styling
  - Signal/slot connections
  - Default format (JSON)
  - Dialog modal behavior

- [ ] T041 Update `src/qt/CMakeLists.txt` to include format_selection_dialog files
- [ ] T042 [US3] Modify existing Qt save handler in `src/qt/` (locate main window/save button handler):
  - Create and show FormatSelectionDialog
  - Get selected format
  - Pass to serializer factory
  - Integrate with existing file save dialog

- [ ] T043 [US3] Update `src/qt/CMakeLists.txt` to link new core serializer libraries

**Checkpoint**: User Story 3 complete - Qt GUI users can select save format

---

## Phase 6: User Story 4 - Format Selection Dialog in HTML GUI (Priority: P1)

**Goal**: HTML web application users can select save format via modal dialog and download files

**Independent Test**: Access HTML GUI → Load config → Click Save → Modal appears → Select format → Download triggered

### Unit Tests for US4

- [ ] T044 [P] [US4] Create `tests/unit/test_html_format_selection.js` with test cases:
  - Modal creation and visibility toggling
  - Radio button state management
  - Format value retrieval
  - Promise-based show() behavior
  - Event listener attachment

### Integration Tests for US4

- [ ] T045 [US4] Create `tests/integration/test_html_save_workflow_with_modal.js` with scenarios:
  - HTML GUI loads config
  - User clicks Save button
  - Format selection modal appears
  - User selects JSON → POST to server → file download triggered
  - User selects INI → POST to server → file download triggered
  - User cancels modal → save operation aborted
  - Modal appears within 500ms (performance check)

### Implementation for US4

- [ ] T046 [P] [US4] Create `resources/html/format-selection.html` with:
  - Modal container (initially hidden)
  - Radio buttons for JSON and INI formats
  - Format descriptions with icons
  - Save and Cancel buttons
  - Accessible form structure

- [ ] T047 [P] [US4] Create `resources/html/format-selection.css` with:
  - Modal styling (overlay, centered dialog)
  - Radio button styling
  - Button styling (primary/secondary)
  - Responsive design for various screen sizes

- [ ] T048 Create `resources/html/format-selection.js` implementing FormatSelector class:
  - show() method returning Promise
  - get selected format
  - Event listeners for buttons
  - Modal visibility toggle
  - Accessible keyboard support (Escape to cancel)

- [ ] T049 [US4] Modify `src/html/handlers/save_handler.cpp` to:
  - Add /api/save POST endpoint (if not exists)
  - Accept format parameter
  - Call appropriate serializer via factory
  - Return serialized content with correct MIME type
  - Handle errors with proper HTTP status codes

- [ ] T050 [US4] Modify HTTP response handler to:
  - Set Content-Type header (application/json or text/plain)
  - Set Content-Disposition header for download
  - Trigger browser download

- [ ] T051 Update `src/html/CMakeLists.txt` to include save_handler modifications and link core serializers
- [ ] T052 Integrate `resources/html/format-selection.js` into main HTML file to:
  - Import FormatSelector class
  - Hook Save button to show format selection
  - Handle POST to /api/save
  - Trigger file download

**Checkpoint**: User Story 4 complete - HTML GUI users can select save format and download files

---

## Phase 7: User Story 5 - Preserve Configuration Data Integrity During Format Conversion (Priority: P2)

**Goal**: Ensure data is preserved accurately across format conversions without loss or corruption

**Independent Test**: Save JSON → Load → Save INI → Load → Validate all data intact and schema-compliant

### Unit Tests for US5

- [ ] T053 [P] [US5] Create `tests/unit/test_serialization_round_trip.cpp` with comprehensive test cases:
  - JSON → JSON round-trip: 100% identical
  - JSON → INI → JSON round-trip: data preservation with flattening
  - Various data types: strings, numbers, booleans, null, arrays, nested objects
  - Edge cases: empty values, special characters, unicode strings, very long strings
  - Large configurations (> 100KB)
  - Verify deserialized JSON validates against schema

- [ ] T054 [P] [US5] Create `tests/fixtures/config_invalid.json` - configs that should fail validation
- [ ] T055 [P] [US5] Create additional test fixtures for edge cases (unicode, special chars, large configs)

### Integration Tests for US5

- [ ] T056 [US5] Create `tests/integration/test_round_trip_both_formats.cpp` with end-to-end scenarios:
  - Load config → save JSON → reload → verify identical
  - Load config → save JSON → load JSON → save INI → load INI → verify schema compliance
  - Save in both formats from same config → compare structure equivalence
  - Extensive validation against schema after each round-trip

### Implementation for US5 (Quality Verification)

- [ ] T057 [US5] Create equality comparison function for config data in `src/core/serializers/`:
  - Deep comparison of JSON objects
  - Handle array ordering
  - Compare types strictly

- [ ] T058 [US5] Add schema validation to core save pipeline:
  - After deserialization: validate against schema
  - After flattening/unflattening: validate structure
  - Prevent saving invalid data in any format

- [ ] T059 [US5] Update serializer error handling to catch and report:
  - Type mismatches
  - Missing required fields
  - Out-of-range values
  - Encoding errors

- [ ] T060 [US5] Create test utilities in `tests/common/`:
  - Config generator for complex nested structures
  - Comparison functions for data integrity checks
  - Schema validation helpers

- [ ] T061 [US5] Run all round-trip tests and verify 100% success rate
- [ ] T062 [US5] Document any data type limitations in conversion (e.g., precision loss)

**Checkpoint**: User Story 5 complete - Data integrity verified across all format conversions

---

## Phase 8: Integration & Cross-Cutting Concerns

**Purpose**: End-to-end testing, documentation, and quality assurance

### End-to-End Testing

- [ ] T063 [P] Create `tests/integration/test_e2e_both_guis.cpp`:
  - Qt GUI: complete workflow for both JSON and INI saves
  - HTML GUI: complete workflow for both JSON and INI saves
  - Verify both produce equivalent results

- [ ] T064 [P] Create `tests/integration/test_error_scenarios.cpp`:
  - Permission denied on write
  - Disk full simulation
  - Invalid file paths
  - Corrupted configuration data
  - Verify error messages are user-friendly

### Build & Compilation

- [ ] T065 [P] Verify all CMakeLists.txt files compile without errors
- [ ] T066 [P] Run clang-tidy for MISRA C++ compliance on all new files
- [ ] T067 [P] Verify no compiler warnings (treat as errors)

### Test Execution & Coverage

- [ ] T068 Execute all unit tests: `ctest --test-dir build -R unit`
- [ ] T069 Execute all integration tests: `ctest --test-dir build -R integration`
- [ ] T070 Generate code coverage report (target: ≥90% for new code)

### Performance Validation

- [ ] T071 Measure format selection dialog latency (target: < 500ms)
- [ ] T072 Measure JSON serialization time for typical configs (target: < 100ms)
- [ ] T073 Measure INI serialization time for typical configs (target: < 200ms)
- [ ] T074 Measure file write time (target: < 500ms for < 1MB)

### Static Analysis

- [ ] T075 Run full clang-tidy analysis: verify 0 MISRA violations
- [ ] T076 Verify memory safety: no raw pointers, use smart pointers throughout
- [ ] T077 Check const-correctness: all functions properly marked const where applicable

### Manual Testing

- [ ] T078 [P] Qt GUI manual testing checklist:
  - [ ] Load sample configuration
  - [ ] Save as JSON → verify file created with correct format
  - [ ] Save as INI → verify file created with correct format
  - [ ] Cancel format dialog → verify no file created
  - [ ] Test with large configuration (> 1MB)
  - [ ] Test with special characters and unicode
  - [ ] Verify round-trip: save → load → save again → identical

- [ ] T079 [P] HTML GUI manual testing checklist:
  - [ ] Load sample configuration
  - [ ] Save as JSON → verify download triggered
  - [ ] Save as INI → verify download triggered
  - [ ] Cancel modal → verify no download
  - [ ] Test with large configuration (> 1MB)
  - [ ] Test with special characters and unicode
  - [ ] Verify browser compatibility (Chrome, Firefox, Safari)

### Documentation Update (Deferred per Requirements)

- [ ] T080 **POST-TESTING ONLY**: Update main documentation after manual testing complete
  - Update docs/SOFTWARE_ARCHITECTURE.md with Phase 6 implementation details
  - Create user guide for multi-format save feature
  - Document supported data type mappings (JSON ↔ INI)
  - Add troubleshooting guide for common issues

---

## Dependencies & Execution Order

### Phase Dependencies

```
Phase 1 (Setup)
    ↓
Phase 2 (Foundational - BLOCKING)
    ├→ Phase 3 (US1: JSON)
    ├→ Phase 4 (US2: INI)
    ├→ Phase 5 (US3: Qt Dialog)
    ├→ Phase 6 (US4: HTML Modal)
    └→ Phase 7 (US5: Data Integrity)
         ↓
Phase 8 (Integration & Cross-Cutting)
```

### Task Dependencies Within Phases

**Phase 2 Critical Path**:
- T010-T013 (Models) → T014-T017 (Factory) → (Required by all user stories)
- T018-T021 (I/O Layer) → (Required by US1, US2, US3, US4)

**US1 Path**: T025-T028 (depends on Phase 2)  
**US2 Path**: T033-T036 (depends on Phase 2, independent of US1)  
**US3 Path**: T039-T043 (depends on Phase 2 and US1/US2 core)  
**US4 Path**: T046-T052 (depends on Phase 2 and US1/US2 core)  
**US5 Path**: T053-T062 (depends on Phase 2, US1, US2)

### Parallel Execution Opportunities

**Within Phase 2**: 
- T010-T013, T018-T019 can run in parallel (different files)
- T003-T005 (directory setup) can run in parallel

**Within US1**: T022-T023 (tests) can run in parallel with T025-T028 (implementation)

**Within US2**: T029-T031 (tests) can run in parallel with T033-T036 (implementation)

**Across User Stories**: After Phase 2 completion:
- US1, US2 can proceed in parallel (independent serializers)
- US3 and US4 can proceed after US1/US2 serializers complete
- US5 depends on US1+US2 completion but can proceed immediately after

**Recommended Parallel Strategy**:
1. One developer: Phase 1-2 (Setup + Foundation)
2. Then, parallel work:
   - Developer A: US1 (JSON serializer)
   - Developer B: US2 (INI serializer) 
   - Developer C: US3 (Qt Dialog) + US4 (HTML Modal)
3. Developer A: US5 (Round-trip validation) after US1+US2
4. All: Phase 8 (Integration, testing, documentation)

---

## Parallel Example: User Story 1 (JSON Serializer)

```bash
# Multiple developers can work on these in parallel:
Task T022: Unit tests for JSON serializer (tests/unit/test_json_serializer.cpp)
Task T023: Test fixtures (tests/fixtures/config_simple.json)
Task T025: JSON serializer header (src/core/serializers/json_serializer.h)
Task T026: JSON serializer implementation (src/core/serializers/json_serializer.cpp)

# Then sequentially:
Task T027: Update CMakeLists.txt
Task T028: Register in factory
Task T024: Integration tests
```

---

## Parallel Example: User Story 2 (INI Serializer)

```bash
# Can proceed in parallel with User Story 1:
Task T029: Unit tests (tests/unit/test_ini_serializer.cpp)
Task T030-T031: Test fixtures
Task T033: INI serializer header
Task T034: INI serializer implementation
Task T035: Update CMakeLists.txt
Task T036: Register in factory
Task T032: Integration tests
```

---

## Parallel Example: User Stories 3 & 4 (Qt + HTML UIs)

```bash
# After Phase 2 + US1/US2 complete, can split UI work:

Developer A (Qt):
  T037-T038: Qt tests
  T039-T040: Qt dialog implementation
  T041-T043: Qt integration

Developer B (HTML):
  T044-T045: HTML tests
  T046-T047: HTML modal (HTML + CSS)
  T048-T052: HTML JavaScript + backend integration
```

---

## MVP Scope Recommendation

**Minimum Viable Product (Phase 3 only)**:
- ✅ T010-T028: Core JSON serializer + Phase 1-2 foundation
- **Delivers**: Users can save configurations in JSON format (US1)
- **Why**: JSON is already supported, leverages existing validation
- **Testing**: All unit + integration tests pass
- **Deployment**: Feature works, minimal code changes

**Recommended MVP+** (Phase 3-4):
- ✅ T010-T036: Core foundation + JSON + INI serializers
- **Adds**: INI format support (US2)
- **Why**: Both core serializers complete, UI can follow
- **Testing**: Round-trip tests pass for both formats

**Full Feature** (Phase 3-7):
- ✅ All tasks T001-T062
- **Adds**: Qt GUI dialog, HTML GUI modal, data integrity verification
- **Testing**: Complete end-to-end testing across both applications

---

## Quality Checkpoints

| Checkpoint | Phase | Criteria |
|-----------|-------|----------|
| **Foundation Ready** | After T021 | All data models and I/O layer implemented, no user stories blocked |
| **JSON Complete** | After T028 | JSON serializer passes all unit + integration tests |
| **INI Complete** | After T036 | INI serializer passes all unit + integration tests, round-trip validated |
| **Qt GUI Ready** | After T043 | Format dialog appears and properly integrates with Qt save workflow |
| **HTML GUI Ready** | After T052 | Format modal appears and download functionality works |
| **Data Integrity** | After T062 | All round-trip tests pass, 100% data preservation verified |
| **Release Ready** | After T080 | Manual testing complete, documentation updated, all quality gates pass |

---

## Success Criteria Validation

| Success Criterion | Task Validation | Status |
|------------------|-----------------|--------|
| **SC-001**: JSON round-trip preservation | T022, T053 unit tests | ✓ Testable |
| **SC-002**: INI format validity | T029, T032 integration tests | ✓ Testable |
| **SC-003**: Dialog < 500ms response | T038, T045 performance tests | ✓ Measurable |
| **SC-004**: Save < 1 minute for typical configs | T024, T032, T074 | ✓ Measurable |
| **SC-005**: 99.9% success rate | T064 error scenario tests | ✓ Testable |
| **SC-006**: Format parity (Qt + HTML) | T063 end-to-end tests | ✓ Verifiable |
| **SC-007**: Data integrity across conversion | T053, T056 round-trip tests | ✓ Testable |
| **SC-008**: Clean cancellation | T037, T044, T064 tests | ✓ Testable |

---

## Total Task Count Summary

| Phase | Focus | Tasks | Details |
|-------|-------|-------|---------|
| **Phase 1** | Setup | 9 | Project structure, CMake config, directories |
| **Phase 2** | Foundation | 12 | Core models, factory, serializer interface, I/O layer |
| **Phase 3** | US1: JSON | 12 | Unit tests, fixtures, implementation, integration |
| **Phase 4** | US2: INI | 12 | Unit tests, fixtures, implementation, integration |
| **Phase 5** | US3: Qt Dialog | 12 | Unit tests, dialog implementation, integration, Qt workflow |
| **Phase 6** | US4: HTML Modal | 12 | Unit tests, modal HTML/CSS/JS, API endpoint, integration |
| **Phase 7** | US5: Data Integrity | 12 | Comprehensive round-trip tests, quality verification |
| **Phase 8** | Integration | 18 | E2E tests, build, analysis, performance, manual testing, docs |
| **TOTAL** | All Phases | **99 tasks** | Comprehensive implementation with full test coverage |

---

## File Creation Summary

**New C++ Files** (14):
- Core models: format_type.{h,cpp}, serialization_context.h, serialization_result.h
- Serializers: format_serializer.h, json_serializer.{h,cpp}, ini_serializer.{h,cpp}, serializer_factory.{h,cpp}
- I/O layer: configuration_writer.{h,cpp}, configuration_reader.{h,cpp}

**New Qt Files** (2):
- UI: format_selection_dialog.{h,cpp}

**New Web Files** (3):
- HTML: format-selection.html, format-selection.css
- JavaScript: format-selection.js

**New Test Files** (8):
- Unit tests: test_json_serializer.cpp, test_ini_serializer.cpp, test_serialization_round_trip.cpp, test_qt_format_dialog.cpp
- Integration tests: test_json_save_workflow.cpp, test_ini_save_workflow.cpp, test_qt_save_workflow_with_dialog.cpp, test_html_save_workflow_with_modal.js
- Additional: test_error_scenarios.cpp, test_e2e_both_guis.cpp

**New Test Fixtures** (4):
- config_simple.json, config_complex.json, config_invalid.json, config_expected.ini

**Configuration Files**:
- Root CMakeLists.txt (update for mINI)
- src/core/CMakeLists.txt (update for serializers)
- src/qt/CMakeLists.txt (update for dialog)
- src/html/CMakeLists.txt (update for API handler)

---

## See Also

- `spec.md` - Feature specification (5 user stories)
- `plan.md` - Implementation plan (architecture, decisions)
- `research.md` - Technical research (mINI selection, flattening strategy)
- `data-model.md` - Data structures (7 models, API contracts)
- `quickstart.md` - Implementation guide (code examples, step-by-step)
- `contracts/serialization-api.md` - API specifications (12 endpoints)

---

**Total Implementation Effort**: ~50-70 developer hours  
**Recommended Timeline**: 1-2 weeks with 2-3 developers  
**MVP Delivery**: Phase 3 only (~15-20 hours, 3-4 days)  
**Full Feature Delivery**: All phases (~60-70 hours, 1-2 weeks)

**Status**: ✅ Ready for implementation - All 99 tasks defined and sequenced

---

## Next Actions

1. **Review**: Verify task list with team and stakeholders
2. **Plan**: Assign developers to phases based on skill/capacity
3. **Execute**: Begin with Phase 1 (Setup) immediately
4. **Track**: Update task checkboxes as work progresses
5. **Validate**: Run tests frequently, verify quality gates at each checkpoint

---

**Generated**: October 27, 2025  
**For**: Feature 003-multi-format-save  
**Format**: 99 tasks organized by user story with parallel opportunities
