# Implementation Plan: Multi-Format Configuration Save

**Branch**: `003-multi-format-save` | **Date**: October 27, 2025 | **Spec**: [specs/003-multi-format-save/spec.md](spec.md)
**Input**: Feature specification from `/specs/003-multi-format-save/spec.md`

**Note**: This implementation plan defines the technical approach for adding JSON and INI format support to ConfigGUI's save functionality across both Qt and HTML GUIs.

## Summary

The Multi-Format Configuration Save feature extends ConfigGUI's existing configuration persistence capability by supporting both JSON and INI output formats. Users will be presented with a format selection dialog when saving, enabling them to choose their preferred format. JSON support leverages the existing schema validation system, while INI support provides backward compatibility and enterprise integration. Implementation requires dual-interface changes (Qt desktop and HTML web), serialization logic in the core library, and comprehensive testing of round-trip data preservation.

## Technical Context

**Language/Version**: C++17 (core) + C++ with Qt6 (Qt GUI) + ES6 JavaScript/HTML5 (HTML GUI)  
**Primary Dependencies**: 
- Core: nlohmann_json, json-schema-validator, yaml-cpp (existing)
- New: INI parser/serializer (to be selected)
- Qt: Qt6 Core, Gui, Widgets (existing)
- HTML: cpp-httplib (embedded server, existing)

**Storage**: File system (JSON and INI files)  
**Testing**: Google Test (unit), Qt Test Framework (Qt integration), JavaScript/HTML5 tests (HTML integration)  
**Target Platform**: Cross-platform (Linux, Windows, macOS via Qt; Web via HTTP server)  
**Project Type**: Dual application (Qt desktop + HTTP server with HTML frontend)  
**Performance Goals**: 
- Format selection dialog appears within 500ms
- Save operation completes within 1 minute for typical configs (< 1MB)
- 99.9% success rate under normal filesystem conditions

**Constraints**: 
- Must maintain 100% JSON round-trip data preservation
- Must respect INI format limitations (flattening of nested structures)
- Must not break existing JSON save/load functionality
- Must pass MISRA C++ compliance for core library changes

**Scale/Scope**: 
- 2 formats (JSON, INI)
- 2 UI implementations (Qt, HTML)
- 5 user stories (4 P1, 1 P2)
- Estimated 12-15 functional components


## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### ConfigGUI Constitution Compliance Verification

#### I. Schema-Driven Dynamic UI ✅
- **Requirement**: All UI components generated dynamically from JSON Schema
- **Status**: COMPLIANT
- **Rationale**: Format selection dialog is schema-agnostic; configuration data remains schema-validated in JSON form internally; INI is export-only format that doesn't affect schema validation pipeline

#### II. JSON as Primary Interface ✅
- **Requirement**: JSON everywhere, schema validation mandatory, bidirectional JSON ↔ GUI sync
- **Status**: COMPLIANT
- **Rationale**: Internal representation remains JSON; JSON save path uses existing validation; INI is secondary export format; No breaking changes to JSON validation workflow

#### III. MISRA C++ Compliance ✅
- **Requirement**: All C++ code follows MISRA C++ rules, clang-tidy enforcement mandatory
- **Status**: COMPLIANT - TO BE ENFORCED
- **Rationale**: New serialization code in core library will be subject to same MISRA checks as existing code; No exceptions or non-compliant patterns in design

#### IV. Real-Time Validation & User Guidance ✅
- **Requirement**: JSON Schema validation on every edit, errors prevent save
- **Status**: COMPLIANT
- **Rationale**: JSON save uses existing validation pathway; INI export happens after JSON validation passes; User cannot save invalid configuration in either format

#### V. Multi-Schema Support & Extensibility ✅
- **Requirement**: Plugin architecture for custom validators, support arbitrary JSON Schema
- **Status**: COMPLIANT
- **Rationale**: Format serialization is independent of schema; INI mapping is format-level, not schema-specific; Existing plugin architecture unaffected

### Code Quality & Architecture Compliance

| Standard | Requirement | Status | Notes |
|----------|-------------|--------|-------|
| Memory Management | Smart pointers only, no raw new/delete | ✅ | Core serializers will use unique_ptr/shared_ptr |
| Type Safety | const-correctness, strong typing | ✅ | Result<T> for fallible serialization operations |
| Error Handling | Qt signals/slots for async, no bare exceptions | ✅ | Format dialogs use Qt signals; File I/O errors propagated via Result<T> |
| Qt Integration | MVC pattern, QAbstractItemModel for lists | ✅ | Format dialog integrated with existing save workflow |
| Logging | Structured logging, disabled in Release | ✅ | Serialization events logged with severity levels |
| Build | CMake with clang-tidy in CI | ✅ | No changes to build system required; existing CI applies |

### Gate Assessment

**GATE STATUS**: ✅ **PASS - All constitution principles compliant**

**Justification**: 
- No fundamental architecture changes
- Feature is strictly additive (new serialization formats)
- Existing JSON validation pipeline unchanged
- MISRA C++ compliance enforced via existing CI/CD
- Dual-format support is internal implementation detail; does not violate schema-driven principle

**Conditions for Phase 1**:
1. INI parser selection must be MISRA C++ compliant or have clear interface layer
2. Format selection logic must not bypass validation
3. All new code must pass clang-tidy checks


## Project Structure

### Documentation (this feature)

```
specs/003-multi-format-save/
├── spec.md              # Feature specification (completed)
├── plan.md              # This file (Phase 0/1 output)
├── research.md          # Phase 0 output (NEEDS CLARIFICATION resolutions)
├── data-model.md        # Phase 1 output (data structures, serialization contracts)
├── quickstart.md        # Phase 1 output (implementation guide)
├── contracts/           # Phase 1 output (API interfaces)
│   ├── format-serializer.md
│   ├── json-serializer.md
│   ├── ini-serializer.md
│   └── format-selection-ui.md
├── checklists/
│   └── requirements.md  # Specification validation checklist
└── tasks.md             # Phase 2 output (NOT created by /speckit.plan)
```

### Source Code (ConfigGUI repository)

```
# Core Library - Format Serialization (NO Qt DEPENDENCY)
src/core/
├── serializers/
│   ├── format_serializer.{h,cpp}           # Base interface for all format serializers
│   ├── json_serializer.{h,cpp}             # JSON serialization (wraps existing nlohmann_json)
│   ├── ini_serializer.{h,cpp}              # INI serialization (new)
│   └── serializer_factory.{h,cpp}          # Factory pattern for format selection
├── models/
│   ├── format_type.h                       # Enum: JSON, INI
│   └── serialization_context.h             # Configuration data + metadata for serialization
└── [existing core files unchanged]

# Qt GUI Application - Format Selection Dialog
src/ui/
├── format_selection_dialog.{h,cpp}         # Modal dialog: JSON vs INI selection
└── [existing Qt UI files]

src/qt/
├── CMakeLists.txt                          # Link new core serializers
└── [existing Qt app files - modified to use new serializers]

# HTML/Web Server - Format Selection UI
src/html/
├── handlers/
│   └── save_handler.cpp                    # Modified to support format selection
└── [existing HTML server files]

resources/html/
├── format-selection.html                   # Format selection UI (modal or dropdown)
├── format-selection.js                     # Format selection logic and file handling
└── [existing HTML assets]

# Tests - Round-trip Serialization
tests/unit/
├── test_json_serializer.cpp                # JSON serialization unit tests
├── test_ini_serializer.cpp                 # INI serialization unit tests
└── test_serialization_round_trip.cpp       # JSON ↔ INI ↔ JSON conversion tests

tests/integration/
├── test_qt_format_dialog.cpp               # Qt GUI dialog integration
├── test_html_format_selection.js           # HTML GUI format selection tests
└── test_save_workflow_both_formats.cpp     # End-to-end save scenarios

# Build Configuration (CMakeLists.txt)
CMakeLists.txt                              # Modified to include INI parser dependency
src/CMakeLists.txt                          # Updated with new serializer modules
src/core/CMakeLists.txt                     # Links new serializer libraries
```

**Structure Decision**: Dual-application structure (Qt + HTTP server) maintained. New serialization logic placed in core library (no Qt dependencies) to support both applications. Format selection UI implemented separately in each frontend (Qt dialog, HTML form) but uses shared core serialization interfaces. Testing pyramid: unit tests for serializers, integration tests for UI workflows, end-to-end tests for round-trip data preservation.


## Phases Overview

### Phase 0: Research & Analysis (CURRENT)

**Objective**: Resolve all NEEDS CLARIFICATION items and finalize technical decisions

**Tasks**:
1. Research INI parser options (C++ libraries that are MISRA C++ compliant)
2. Analyze data type mapping strategy for JSON → INI conversion
3. Review existing serialization patterns in ConfigGUI codebase
4. Define error handling strategy for filesystem operations

**Deliverable**: `research.md` with all decisions documented

### Phase 1: Design & Contracts (PENDING /speckit.plan execution)

**Objective**: Define data models, APIs, and integration contracts

**Tasks**:
1. Create `data-model.md` with serialization data structures
2. Generate API contracts in `/contracts/` directory
3. Create `quickstart.md` for implementation reference
4. Update agent context with new technologies

**Deliverables**: `data-model.md`, `/contracts/`, `quickstart.md`, updated agent context

### Phase 2: Task Breakdown (PENDING /speckit.tasks command)

**Objective**: Generate detailed implementation tasks and dependencies

**Tasks** (to be generated by /speckit.tasks):
1. Core serializer interfaces
2. JSON serializer implementation
3. INI serializer implementation + parser integration
4. Qt GUI format selection dialog
5. HTML GUI format selection UI
6. Unit tests for all serializers
7. Integration tests for both GUIs
8. End-to-end round-trip tests
9. Documentation updates (post-testing)

**Deliverable**: `tasks.md` with task breakdown, dependencies, and time estimates

## Next Steps

1. **If continuing with Phase 0**: `/speckit.plan` will generate `research.md` with all NEEDS CLARIFICATION items resolved
2. **After Phase 0 complete**: Run `/speckit.plan` again to proceed to Phase 1 design documents
3. **After Phase 1 complete**: Run `/speckit.tasks` to generate detailed implementation task breakdown

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |

