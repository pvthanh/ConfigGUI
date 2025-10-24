# Implementation Plan: HTML Form Generation with Local Server

**Branch**: `002-html-form-generation` | **Date**: October 24, 2025 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/002-html-form-generation/spec.md`

## Summary

The HTML Form Generation feature extends ConfigGUI with a web-based interface enabling users to generate configuration forms from JSON/YAML schemas without requiring Qt libraries. The implementation maintains a shared C++ core (validation, schema parsing, data handling) while isolating Qt-specific UI code, enabling deployment in non-Qt environments including Docker containers and embedded systems. This feature delivers architectural separation ensuring long-term maintainability and code reuse across both desktop and web interfaces.

## Technical Context

**Language/Version**: C++17 with MISRA C++ compliance  
**Primary Dependencies**: 
- Core: nlohmann/json, json-schema-validator, libyaml
- Web Server: C++ HTTP library (NEEDS CLARIFICATION: embedded HTTP library vs standalone server)
- Frontend: Modern HTML5/ES6 JavaScript (no framework initially)
- Qt: Qt 6.x (for Qt application only, NOT required for HTML deployment)

**Storage**: File system (local JSON/YAML configurations)  
**Testing**: CMake test framework (unit + integration tests for C++ core); browser-based testing for HTML UI  
**Target Platform**: Linux, macOS, Windows (desktop Qt); any OS with C++ compiler (HTML server); modern web browsers (Chrome, Firefox, Safari, Edge)  
**Project Type**: Hybrid - extends single C++ project with dual UI components  
**Performance Goals**: 
- Form startup: < 2 seconds
- Form generation: < 1 second  
- Validation feedback: < 200ms latency
- Support 1000+ schema properties
- Handle 10 concurrent users

**Constraints**: 
- No Qt dependencies in HTML deployment
- 95%+ UI behavior parity between Qt and HTML versions
- Zero code duplication in validation/core logic
- Localhost-only deployment (HTTPS not required for MVP)
- MISRA C++ compliance mandatory

**Scale/Scope**: 
- MVP: 3 P1 user stories (non-Qt deployment, web form generation, shared core)
- Phase 2: 2 P2 stories (export/import, Docker containerization)
- ~3-4 weeks development for MVP

## Constitution Check

**Gate 1: Schema-Driven Dynamic UI** ✅ PASS
- Requirement: All UI components generated from JSON Schema
- Implementation: HTML forms generated from same schema as Qt application
- Compliance: Both UI layers treat schema as single source of truth

**Gate 2: JSON as Primary Interface** ✅ PASS
- Requirement: JSON/YAML configuration validation mandatory; bidirectional sync
- Implementation: Shared core handles all JSON/YAML I/O and validation
- Compliance: Web server receives/returns JSON; validation occurs in shared core

**Gate 3: MISRA C++ Compliance (NON-NEGOTIABLE)** ✅ PASS
- Requirement: All C++ code follows MISRA C++ rules; static analysis mandatory
- Implementation: Shared core code already MISRA-compliant; HTML server C++ code follows same standards
- Compliance: Static analysis (clang-tidy) enforced in CI; no exceptions without justification

**Gate 4: Real-Time Validation & User Guidance** ✅ PASS
- Requirement: JSON Schema validation on every edit; errors prevent save
- Implementation: Shared validator called from both Qt and HTML on each field change
- Compliance: Both interfaces show validation feedback identically; save blocked on validation failure

**Gate 5: Multi-Schema Support & Extensibility** ✅ PASS
- Requirement: Plugin architecture for custom validators; support arbitrary JSON Schema drafts
- Implementation: HTML server uses same schema validator as Qt (supports JSON Schema drafts via core library)
- Compliance: No core modifications required for new schemas; extensible through schema x-* extensions

**Gate 6: Memory Management** ✅ PASS
- Requirement: Smart pointers only; no raw new/delete
- Implementation: Shared core uses smart pointers; HTTP server uses smart pointers for request/response handling
- Compliance: Code review enforces smart pointer usage

**Gate 7: Type Safety & Error Handling** ✅ PASS
- Requirement: const-correctness; strong typing; no void*; Result<T> patterns for fallible ops
- Implementation: C++17 optional<T> and custom Result type; const-correct function signatures
- Compliance: Compiler enforces; static analysis verifies

## Constitution Compliance Summary

✅ All 7 Constitution gates PASS  
✅ No violations requiring justification  
✅ Feature architecture aligns with ConfigGUI principles  
✅ Shared core ensures consistency across UI implementations  

**Compliance Status**: READY FOR PHASE 0 RESEARCH

## Project Structure

### Documentation (this feature)

```
specs/002-html-form-generation/
├── spec.md              # Feature specification (COMPLETE)
├── plan.md              # This file (PHASE 0 PLANNING)
├── research.md          # PENDING: Phase 0 research tasks
├── data-model.md        # PENDING: Phase 1 entity models
├── quickstart.md        # PENDING: Phase 1 developer guide
├── contracts/           # PENDING: Phase 1 API contracts
│   ├── openapi.yaml     # HTTP API specification
│   └── schema-load-contract.md  # Schema loading contract
├── tasks.md             # PENDING: Phase 2 task breakdown
└── checklists/
    └── requirements.md  # Quality validation (COMPLETE)
```

### Source Code (repository root)

**Current Structure (Qt Application)**:
```
src/
├── main.cpp             # Qt application entry point
├── core/                # SHARED CORE - to be extracted/enhanced
│   ├── configuration_data.h/cpp
│   ├── schema_loader.h/cpp
│   ├── schema_validator.h/cpp
│   ├── validation_error.h/cpp
│   └── result.h
├── io/                  # SHARED I/O - to be reused
│   ├── json_reader.h/cpp
│   ├── json_writer.h/cpp
│   ├── yaml_reader.h/cpp
│   └── yaml_writer.h/cpp
├── ui/                  # QT-SPECIFIC - isolated from HTML
│   ├── form_generator.h/cpp
│   ├── main_window.h/cpp
│   ├── rule_editor_widget.h/cpp
│   └── ... (other Qt widgets)
├── utils/               # POTENTIALLY SHARED
└── validators/          # POTENTIALLY SHARED

tests/
├── unit/                # Qt-specific tests
├── integration/         # Qt-specific tests
└── common/              # Shared test utilities
```

**New Structure (With HTML Support)**:
```
src/
├── core/                # SHARED CORE (extracted, enhanced)
│   ├── schema/
│   │   ├── schema_loader.h/cpp
│   │   ├── schema_validator.h/cpp
│   │   └── rule_definition.h
│   ├── io/
│   │   ├── json_reader.h/cpp
│   │   ├── json_writer.h/cpp
│   │   ├── yaml_reader.h/cpp
│   │   └── yaml_writer.h/cpp
│   ├── data/
│   │   ├── configuration_data.h/cpp
│   │   └── validation_error.h/cpp
│   └── types.h          # Common types and Result<T> pattern
│
├── qt/                  # QT-SPECIFIC - isolated
│   ├── main.cpp
│   ├── ui/
│   │   ├── form_generator.h/cpp
│   │   ├── main_window.h/cpp
│   │   └── ... (Qt widgets)
│   └── app_main_window.h/cpp
│
├── html/                # NEW: HTML SERVER - independent
│   ├── main.cpp         # HTTP server entry point
│   ├── server.h/cpp     # HTTP server implementation
│   ├── request_handler.h/cpp  # Request routing
│   ├── schema_service.h/cpp  # Schema loading logic
│   ├── form_service.h/cpp    # Form generation logic
│   └── config_service.h/cpp  # Configuration save/load
│
└── shared/              # UTILITY CODE (both Qt and HTML)
    ├── logging.h/cpp
    └── file_utils.h/cpp

tests/
├── unit/
│   ├── core/            # Core validation, schema tests (both use)
│   ├── io/              # I/O tests (both use)
│   ├── qt/              # Qt-specific tests
│   └── html/            # HTML server tests
├── integration/
│   ├── core_io_integration/  # Shared
│   ├── qt_integration/       # Qt-specific
│   └── html_integration/     # HTML-specific
└── contract/            # API contract tests
    └── html_api_contract.cpp
```

**Structure Decision**: Modular extraction pattern - extract shared code into `src/core/` and `src/shared/`, isolate Qt code in `src/qt/`, add new HTML server in `src/html/`. Each module independently compilable and testable. This enables:
- HTML deployment without Qt
- Shared validation logic preventing bugs
- Clear architectural boundaries
- Easier future maintenance

## Complexity Tracking

| Item | Status | Justification |
|------|--------|---------------|
| New HTTP Server Component | Required | Core feature - no existing HTTP server in ConfigGUI |
| Source Code Reorganization | Required | Enables shared core; current structure mixes Qt and core code |
| Dual Build Targets | Required | Qt app + HTML server both needed; can't merge into single binary |

All required complexity justified by architectural goals (non-Qt deployability, code reuse, maintainability).

## Key Decisions Requiring Research

### NEEDS CLARIFICATION (Phase 0 Research Required)

**Technical Decision 1: HTTP Server Library**
- Status: NEEDS CLARIFICATION
- Impact: Core dependency for HTML deployment
- Options to research:
  - Embedded C++ HTTP library (cpp-httplib, crow, beast)
  - Standalone server (Node.js with HTTP wrapper)
  - Framework approach (embedded Flask/Werkzeug via SWIG)

**Technical Decision 2: Frontend Framework**
- Status: NEEDS CLARIFICATION  
- Impact: HTML form generation and validation
- Options to research:
  - Vanilla JavaScript with HTML templates
  - Lightweight framework (Alpine.js, htmx)
  - Full framework (Vue, React, Angular)
- Constraint: Must maintain 95%+ parity with Qt validation behavior

**Technical Decision 3: Build System Enhancements**
- Status: NEEDS CLARIFICATION
- Impact: Compilation flow for dual targets
- Options to research:
  - CMake conditional compilation (add_executable with if)
  - Separate CMakeLists.txt in each module
  - Build flags (--enable-html, --enable-qt)

**Technical Decision 4: Shared Core Extraction Strategy**
- Status: NEEDS CLARIFICATION
- Impact: How to refactor existing code without breaking Qt build
- Options to research:
  - Create new src/core/ with new headers, gradually move implementations
  - Use CMake object libraries to share compiled objects
  - Linker-time reuse vs source-level reuse

## Next Steps

1. **Phase 0 Research** (output: research.md)
   - Investigate HTTP server library options
   - Evaluate frontend technology stack
   - Document build system approach
   - Analyze shared code extraction strategy

2. **Phase 1 Design** (output: data-model.md, contracts/, quickstart.md)
   - Design data model for schemas, configurations, validation
   - Define HTTP API contracts (OpenAPI spec)
   - Create developer quickstart guide
   - Plan HTML template structure

3. **Phase 2 Planning** (output: tasks.md)
   - Detailed task breakdown with subtasks
   - Dependency graph for parallel work
   - Sprint planning with story points

