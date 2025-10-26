---
description: "Task breakdown for HTML Form Generation feature implementation"
---

# Tasks: HTML Form Generation with Local Server

**Input**: Design documents from `specs/002-html-form-generation/`  
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅, contracts/openapi.yaml ✅  
**Feature Branch**: `002-html-form-generation`  
**Status**: Phase 2 Task Breakdown

## Overview

**Feature**: Enable automatic HTML form generation from JSON/YAML schemas with local server hosting, enabling Qt-free deployment and shared core validation.

**MVP Scope** (Phases 1-3): User Stories 1-3 (P1 Priority)
- Deploy HTML application without Qt (US1)
- Generate configuration forms from schemas (US2)
- Maintain single codebase with shared core (US3)

**Timeline**: 4-6 weeks for MVP (~100-120 story points estimated)

**Team Structure Recommendation**: 2-3 developers
- Developer 1: Core extraction + Build system (Phase 1-2)
- Developer 2: HTTP server + HTML form generation (Phase 3 US1-US2)
- Developer 3: Testing & integration (all phases)

---

## Format: `- [ ] [TaskID] [P?] [Story?] Description`

- **Checkbox**: Always `- [ ]` (task tracking)
- **TaskID**: Sequential ID (T001, T002, etc.) in execution order
- **[P]**: Parallelizable (different files, no cross-dependencies)
- **[Story]**: User story label (US1, US2, US3) for story phases only
- **Description**: Clear action with exact file path

---

## Phase 1: Setup & Project Structure

**Purpose**: Initialize project structure for dual-target build (Qt + HTML)

**Timeline**: ~3-5 days | **Story Points**: 8-13

### CMake Build System Configuration

- [ ] T001 Update CMakeLists.txt with conditional build targets (BUILD_QT_APP, BUILD_HTML_SERVER) in /CMakeLists.txt
- [ ] T002 [P] Create src/CMakeLists.txt for modular source organization in /src/CMakeLists.txt
- [ ] T003 [P] Create CMakeLists.txt for core module in /src/core/CMakeLists.txt (shared by both Qt and HTML)
- [ ] T004 [P] Create CMakeLists.txt for Qt-specific module in /src/qt/CMakeLists.txt
- [ ] T005 [P] Create CMakeLists.txt for HTML server module in /src/html/CMakeLists.txt
- [ ] T006 Create fetch_content for cpp-httplib dependency in /CMakeLists.txt
- [ ] T007 Verify dual-build configuration: `cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON` builds successfully
- [ ] T008 Test Qt-only build: `cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF` builds successfully
- [ ] T009 Test HTML-only build: `cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON` builds successfully

### Directory Structure

- [ ] T010 [P] Create /src/core/schema/ directory for schema extraction
- [ ] T011 [P] Create /src/core/data/ directory for shared data structures
- [ ] T012 [P] Create /src/core/io/ directory for JSON/YAML I/O (will be moved from existing)
- [ ] T013 [P] Create /src/html/server/ directory for HTTP server implementation
- [ ] T014 [P] Create /src/html/handlers/ directory for request handlers
- [ ] T015 [P] Create /src/html/assets/ directory for HTML/CSS/JS assets
- [ ] T016 [P] Create resources/html/ directory for HTML templates
- [ ] T017 [P] Create tests/html/ directory for HTML server tests

### Project Documentation

- [ ] T018 Create ARCHITECTURE.md documenting module structure and dependencies in /specs/002-html-form-generation/ARCHITECTURE.md
- [ ] T019 Create BUILD.md with build instructions for all three scenarios in /specs/002-html-form-generation/BUILD.md
- [ ] T020 Update copilot-instructions.md with module organization guidelines in /.github/copilot-instructions.md

**Checkpoint**: Project structure ready, all three build scenarios verified ✅

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Extract and establish shared core module - BLOCKING all user story work

**Timeline**: ~2-3 weeks | **Story Points**: 21-34

**⚠️ CRITICAL**: NO user story implementation can begin until core extraction is complete

### Phase 2A: Core Module Abstraction & Extraction

**Goal**: Extract validation, schema parsing, and data handling into a shared library (no Qt dependencies)

#### Core Data Structures

- [ ] T021 Create Result<T> type in /src/core/result.h with template for error handling
- [ ] T022 Refactor error types in /src/core/error_types.h (consolidate from multiple files)
- [ ] T023 Create validation error representation in /src/core/validation_error.h
- [ ] T024 Create schema definition types in /src/core/schema/schema_types.h

#### Schema Loading & Parsing

- [ ] T025 Extract schema_loader.h/cpp to /src/core/schema/ (move from existing with Qt dependencies removed)
- [ ] T026 Verify schema_loader works with pure JSON input (no Qt file dialogs)
- [ ] T027 Add schema caching mechanism in /src/core/schema/schema_cache.h for performance
- [ ] T028 Update schema_loader to populate cache and handle memory efficiently

#### Schema Validation

- [ ] T029 Extract schema_validator.h/cpp to /src/core/schema/ (ensure Qt-free)
- [ ] T030 Verify validator uses nlohmann/json and json-schema-validator only
- [ ] T031 Create validation rule abstraction in /src/core/schema/validation_rule.h
- [ ] T032 Implement real-time field validation interface in /src/core/schema/field_validator.h

#### I/O Operations (JSON/YAML)

- [ ] T033 [P] Move json_reader.h/cpp to /src/core/io/ (verify Qt-free)
- [ ] T033 [P] Move json_writer.h/cpp to /src/core/io/ (verify Qt-free)
- [ ] T035 [P] Move yaml_reader.h/cpp to /src/core/io/ (verify Qt-free)
- [ ] T036 [P] Move yaml_writer.h/cpp to /src/core/io/ (verify Qt-free)
- [ ] T037 Create unified I/O interface in /src/core/io/configuration_io.h

#### Configuration Data Model

- [ ] T038 Create shared configuration data structure in /src/core/data/configuration.h
- [ ] T039 Extract configuration_data.h/cpp to /src/core/data/ (remove Qt references)
- [ ] T040 Implement configuration serialization/deserialization in /src/core/data/configuration_serializer.h

### Phase 2B: Build System & Static Library

**Goal**: Establish core as static library linked by both Qt and HTML targets

- [ ] T041 Create core module static library target in /src/core/CMakeLists.txt
- [ ] T042 Update main CMakeLists.txt to build core library ALWAYS (required by both targets)
- [ ] T043 Configure Qt application to link core library in /src/qt/CMakeLists.txt
- [ ] T044 Add core library as dependency for HTML server in /src/html/CMakeLists.txt
- [ ] T045 Verify core library has NO Qt dependencies (link test without Qt)
- [ ] T046 Configure include paths for both targets to access core headers

### Phase 2C: Unit Tests for Core Module

**Goal**: Ensure core module works independently (required before user story tests can pass)

- [ ] T047 [P] Create test suite for schema loading in /tests/unit/core/test_schema_loader.cpp
- [ ] T048 [P] Create test suite for schema validation in /tests/unit/core/test_schema_validator.cpp
- [ ] T049 [P] Create test suite for JSON I/O in /tests/unit/core/test_json_io.cpp
- [ ] T050 [P] Create test suite for YAML I/O in /tests/unit/core/test_yaml_io.cpp
- [ ] T051 [P] Create test suite for configuration data in /tests/unit/core/test_configuration.cpp
- [ ] T052 All core unit tests PASS and run without Qt libraries

### Phase 2D: Integration - Qt Application with New Core

**Goal**: Ensure existing Qt application still works after core extraction

- [ ] T053 Update Qt application CMakeLists.txt to link core library
- [ ] T054 Update Qt application #includes to use core module paths
- [ ] T055 Refactor Qt form_generator to use core schema abstraction
- [ ] T056 Run existing Qt tests - all MUST pass
- [ ] T057 Test Qt application with extracted core module - all features work identically
- [ ] T058 Verify no regression in Qt application functionality

**Checkpoint**: Core module extracted, tested, linked by Qt ✅ | HTML can now proceed in parallel

---

## Phase 3: User Story 1 - Developer Deploys HTML Without Qt (Priority: P1) 🎯 MVP

**Goal**: Enable deployment of HTML application in Qt-free environment with full schema form generation

**Independent Test**: Deploy application in Docker container (Qt-free), verify server starts, main form displays, schema selection works

**Timeline**: ~1.5-2 weeks | **Story Points**: 21-34

### HTTP Server Implementation

- [ ] T059 [P] Create http_server.h with cpp-httplib integration in /src/html/server/http_server.h
- [ ] T060 [P] Implement http_server.cpp with route registration and middleware in /src/html/server/http_server.cpp
- [ ] T061 [P] Create request_handler.h for common request processing in /src/html/handlers/request_handler.h
- [ ] T062 [P] Implement request_handler.cpp with error handling in /src/html/handlers/request_handler.cpp
- [ ] T063 Create main entry point for HTML server in /src/html/main.cpp with argument parsing
- [ ] T064 Implement graceful shutdown handling in /src/html/server/http_server.cpp
- [ ] T065 Add logging to HTTP server for startup, route access, errors in /src/html/server/http_server.cpp

### Schema Listing Endpoint

- [x] T066 [US1] Create schema_service.h for schema discovery in /src/html/handlers/schema_service.h
- [x] T067 [US1] Implement schema_service.cpp to scan resources/schemas/ directory in /src/html/handlers/schema_service.cpp
- [x] T068 [US1] Implement GET /api/schemas endpoint in /src/html/handlers/request_handler.cpp
- [x] T069 [US1] Add unit test for schema discovery in /tests/unit/html/test_schema_service.cpp

### Main Form (HTML)

- [ ] T070 [US1] Create index.html with schema selector UI in /src/html/assets/index.html
- [ ] T071 [US1] Create main.css for styling in /src/html/assets/css/main.css
- [ ] T072 [US1] Create main.js for form interactivity (schema selection, form loading) in /src/html/assets/js/main.js
- [ ] T073 [US1] Implement GET / endpoint to serve index.html in /src/html/handlers/request_handler.cpp
- [ ] T074 [US1] Test main form loads in browser with schema list displayed

### Static Asset Serving

- [ ] T075 [P] Configure CMake to embed HTML/CSS/JS assets in binary (or serve from filesystem) in /src/html/CMakeLists.txt
- [ ] T076 [P] Implement static file serving for CSS and JavaScript in /src/html/handlers/request_handler.cpp
- [ ] T077 Verify MIME types are correct (text/html, text/css, application/javascript)

### Docker Support (P1)

- [ ] T078 Create Dockerfile for HTML application in /Dockerfile.html
- [ ] T079 Configure Docker image to copy schema resources in /Dockerfile.html
- [ ] T080 Build Docker image: `docker build -f Dockerfile.html -t configgui-html .`
- [ ] T081 Run Docker container: `docker run -p 8080:8080 configgui-html`
- [ ] T082 Verify HTML application works inside container (schema list, form selection available)

### Integration Tests for User Story 1

- [ ] T083 [P] [US1] Create integration test for main form serving in /tests/integration/test_main_form.cpp
- [ ] T084 [P] [US1] Create integration test for schema discovery in /tests/integration/test_schema_discovery.cpp
- [ ] T085 [P] [US1] Create browser-based test for main form UI in /tests/integration/test_main_form_ui.md
- [ ] T086 [US1] All integration tests PASS

### Documentation for User Story 1

- [ ] T087 [US1] Update quickstart.md with deployment instructions in /specs/002-html-form-generation/quickstart.md
- [ ] T088 [US1] Create DEPLOYMENT.md for Docker/standalone hosting in /specs/002-html-form-generation/DEPLOYMENT.md
- [ ] T089 [US1] Add troubleshooting guide for common startup issues in /specs/002-html-form-generation/quickstart.md

**Checkpoint**: HTML application deployable without Qt, server starts, main form displays, schema list works ✅

---

## Phase 4: User Story 2 - End User Generates Forms from Schema (Priority: P1) 🎯 MVP

**Goal**: Generate dynamic HTML forms from JSON/YAML schemas with identical behavior to Qt version

**Independent Test**: Load JSON schema via main form, verify generated form displays all fields with correct types/validation, fill form with valid data, submit successfully

**Timeline**: ~2-2.5 weeks | **Story Points**: 34-55

### Form Generation Service

- [ ] T090 [US2] Create form_generator.h for HTML form generation in /src/html/handlers/form_generator.h
- [ ] T091 [US2] Implement form_generator.cpp using core schema and validation in /src/html/handlers/form_generator.cpp
- [ ] T092 [US2] Implement core form generation logic (HTML element mapping) in /src/html/handlers/form_generator.cpp
- [ ] T093 [US2] Add support for nested objects (grouped fields) in /src/html/handlers/form_generator.cpp
- [ ] T094 [US2] Add support for arrays (repeating groups) in /src/html/handlers/form_generator.cpp
- [ ] T095 [US2] Unit test form generation for all property types in /tests/unit/html/test_form_generator.cpp

### Schema Loading Endpoint

- [ ] T096 [US2] Implement POST /api/schema/{schema_id}/load in /src/html/handlers/request_handler.cpp
- [ ] T097 [US2] Add schema validation before returning to client in /src/html/handlers/request_handler.cpp
- [ ] T098 [US2] Return parsed schema structure (properties, types, validation rules) as JSON in /src/html/handlers/request_handler.cpp
- [ ] T099 [US2] Unit test schema loading endpoint in /tests/unit/html/test_request_handler.cpp

### Form Generation Endpoint

- [ ] T100 [US2] Implement POST /api/form/generate in /src/html/handlers/request_handler.cpp
- [ ] T101 [US2] Accept schema_id and optional existing configuration in /src/html/handlers/request_handler.cpp
- [ ] T102 [US2] Return generated HTML form as JSON response in /src/html/handlers/request_handler.cpp
- [ ] T103 [US2] Include validation rules in response for client-side validation in /src/html/handlers/request_handler.cpp
- [ ] T104 [US2] Unit test form generation endpoint in /tests/unit/html/test_request_handler.cpp

### Form Rendering (JavaScript)

- [ ] T105 [US2] Create form-renderer.js to dynamically render generated HTML in /src/html/assets/js/form-renderer.js
- [ ] T106 [US2] Implement form field rendering for all types (text, number, checkbox, select, textarea, etc.)
- [ ] T107 [US2] Implement nested object rendering (collapsible groups) in /src/html/assets/js/form-renderer.js
- [ ] T108 [US2] Implement array rendering (add/remove buttons for repeating items) in /src/html/assets/js/form-renderer.js
- [ ] T109 [US2] Add form field event handlers for validation and user feedback in /src/html/assets/js/form-renderer.js

### Real-Time Validation (Frontend & Backend)

- [ ] T110 [US2] Implement client-side validation in form-renderer.js using embedded validation rules
- [ ] T111 [US2] Implement POST /api/validate endpoint for server-side field validation in /src/html/handlers/request_handler.cpp
- [ ] T112 [US2] Create validation_service.h using core validator in /src/html/handlers/validation_service.h
- [ ] T113 [US2] Implement validation_service.cpp calling core schema validator in /src/html/handlers/validation_service.cpp
- [ ] T114 [US2] Add debouncing to validation requests (avoid excessive server calls) in /src/html/assets/js/main.js
- [ ] T115 [US2] Display validation errors inline on form fields in /src/html/assets/js/form-renderer.js
- [ ] T116 [US2] Unit test validation service in /tests/unit/html/test_validation_service.cpp

### Form Submission & Configuration Save

- [ ] T117 [US2] Implement POST /api/form/validate for full-form validation in /src/html/handlers/request_handler.cpp
- [ ] T118 [US2] Implement POST /api/config/save for persisting configuration in /src/html/handlers/request_handler.cpp
- [ ] T119 [US2] Create config_service.h for file I/O in /src/html/handlers/config_service.h
- [ ] T120 [US2] Implement config_service.cpp using core I/O (JSON/YAML) in /src/html/handlers/config_service.cpp
- [ ] T121 [US2] Save configurations to resources/configs/ directory with timestamp/unique name
- [ ] T122 [US2] Implement form submission handler in /src/html/assets/js/main.js
- [ ] T123 [US2] Display success/error messages to user in /src/html/assets/js/main.js
- [ ] T124 [US2] Unit test config service in /tests/unit/html/test_config_service.cpp

### Integration Tests for User Story 2

- [ ] T125 [P] [US2] Create integration test for schema loading in /tests/integration/test_schema_loading.cpp
- [ ] T126 [P] [US2] Create integration test for form generation in /tests/integration/test_form_generation.cpp
- [ ] T127 [P] [US2] Create integration test for real-time validation in /tests/integration/test_validation_flow.cpp
- [ ] T128 [P] [US2] Create integration test for configuration save/load in /tests/integration/test_config_persistence.cpp
- [ ] T129 [US2] Create end-to-end browser test (Selenium/Puppeteer) in /tests/e2e/test_full_workflow.js
- [ ] T130 [US2] All integration tests PASS

### API Contract Tests

- [ ] T131 [P] [US2] Create OpenAPI contract tests in /tests/contracts/test_openapi_compliance.cpp
- [ ] T132 [US2] Validate all endpoints match openapi.yaml specification
- [ ] T133 [US2] Validate response schemas match contracts in /specs/002-html-form-generation/contracts/openapi.yaml

### Qt Parity Verification

- [ ] T134 [US2] Create test comparing Qt form output with HTML form for same schema
- [ ] T135 [US2] Verify 95%+ visual and functional parity in /tests/integration/test_qt_html_parity.cpp
- [ ] T136 [US2] Document any intentional differences (CSS styling, etc.) in /DESIGN_NOTES.md

### Performance Testing

- [ ] T137 [US2] Measure form generation time (target: < 1 second) in /tests/performance/test_form_generation.cpp
- [ ] T138 [US2] Measure validation response time (target: < 200ms) in /tests/performance/test_validation.cpp
- [ ] T139 [US2] Test with schema containing 1000+ properties in /tests/performance/test_large_schema.cpp

### Documentation for User Story 2

- [ ] T140 [US2] Update API reference with form generation endpoints in /specs/002-html-form-generation/API_REFERENCE.md
- [ ] T141 [US2] Create form validation reference in /specs/002-html-form-generation/VALIDATION_GUIDE.md
- [ ] T142 [US2] Add troubleshooting for form generation issues in /specs/002-html-form-generation/TROUBLESHOOTING.md

**Checkpoint**: HTML forms generated from schemas, validation works, configurations save/load successfully ✅

---

## Phase 5: User Story 3 - Qt Developer Maintains Single Codebase (Priority: P1) 🎯 MVP

**Goal**: Ensure shared core is complete, documented, and used by both Qt and HTML without duplication

**Independent Test**: Modify validation rule in core library, rebuild both Qt and HTML targets, verify rule applies identically in both

**Timeline**: ~1 week | **Story Points**: 13-21

### Core Module Documentation

- [ ] T143 [US3] Create CORE_ARCHITECTURE.md documenting core module structure in /specs/002-html-form-generation/CORE_ARCHITECTURE.md
- [ ] T144 [US3] Document all core classes and their interfaces in /src/core/README.md
- [ ] T145 [US3] Create API reference for core module consumers in /specs/002-html-form-generation/CORE_API.md
- [ ] T146 [US3] Add Doxygen comments to all core headers in /src/core/**/*.h

### Build System Documentation

- [ ] T147 [US3] Document CMake build process for dual targets in /BUILD.md
- [ ] T148 [US3] Create example: Building Qt-only in /BUILD.md
- [ ] T149 [US3] Create example: Building HTML-only in /BUILD.md
- [ ] T150 [US3] Create example: Building both in /BUILD.md
- [ ] T151 [US3] Document dependency linking in /BUILD.md

### Qt Application Refactoring (Final)

- [ ] T152 [US3] Refactor Qt form_generator to minimize non-core code in /src/qt/ui/form_generator.cpp
- [ ] T153 [US3] Update Qt configuration_data usage to core module in /src/qt/ui/main_window.cpp
- [ ] T154 [US3] Verify all shared logic in core, all UI-specific logic in Qt in /src/qt/
- [ ] T155 [US3] Run Qt application end-to-end tests - all PASS in /tests/unit/
- [ ] T156 [US3] Verify Qt application unchanged from user perspective

### HTML Server Refactoring (Final)

- [ ] T157 [US3] Verify all shared logic uses core module in /src/html/handlers/
- [ ] T158 [US3] Verify no validation/schema/I/O logic duplicated in /src/html/
- [ ] T159 [US3] Run HTML server end-to-end tests - all PASS in /tests/integration/
- [ ] T160 [US3] Verify HTML application functionality complete

### Code Duplication Audit

- [ ] T161 [US3] Use clang tools to identify duplicate code between Qt and HTML in /
- [ ] T162 [US3] Refactor any found duplicates into core module (if not already)
- [ ] T163 [US3] Verify code coverage of core module (target: 85%+) in /tests/coverage/

### Integration & Regression Testing

- [ ] T164 [P] [US3] Run full test suite: Core unit tests in /tests/unit/core/
- [ ] T165 [P] [US3] Run full test suite: Qt integration tests in /tests/unit/
- [ ] T166 [P] [US3] Run full test suite: HTML integration tests in /tests/integration/
- [ ] T167 [US3] All tests PASS without failures
- [ ] T168 [US3] No performance regression in either target

### Documentation for User Story 3

- [ ] T169 [US3] Create CONTRIBUTING.md for future developers working with shared core in /CONTRIBUTING.md
- [ ] T170 [US3] Update copilot-instructions.md with final module organization in /.github/copilot-instructions.md
- [ ] T171 [US3] Create MAINTENANCE.md documenting how to add new features to core in /specs/002-html-form-generation/MAINTENANCE.md

### Final Validation

- [ ] T172 [US3] Constitution check: Verify all 7 gates still pass in /specs/002-html-form-generation/
- [ ] T173 [US3] Success criteria check: Verify all 10 SC met in /specs/002-html-form-generation/spec.md
- [ ] T174 [US3] Prepare MVP completion report in /specs/002-html-form-generation/MVP_COMPLETION.md

**Checkpoint**: Shared core complete, both targets working, zero duplication, documentation complete ✅

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Final improvements, optimization, hardening before release

**Timeline**: ~1 week | **Story Points**: 8-13

### Code Quality

- [ ] T175 [P] Run clang-tidy on entire codebase in /
- [ ] T176 [P] Fix all style violations and warnings in /
- [ ] T177 [P] Enable strict compiler flags (-Wall, -Wextra, -Werror) in /CMakeLists.txt
- [ ] T178 [P] Verify MISRA C++ compliance in /
- [ ] T179 Add pre-commit hooks for style/format checking in /.githooks/

### Testing & Coverage

- [ ] T180 [P] Generate code coverage report (target: 85%+) in /
- [ ] T181 [P] Identify and test edge cases in core module in /tests/
- [ ] T182 [P] Add stress tests for large schemas (5000+ properties) in /tests/stress/
- [ ] T183 [P] Test error paths and recovery in all modules in /tests/
- [ ] T184 Create continuous integration pipeline in /.github/workflows/

### Documentation

- [ ] T185 [P] Generate API documentation from Doxygen in /docs/api/
- [ ] T186 [P] Create architecture decision records (ADRs) in /docs/adr/
- [ ] T187 [P] Update README.md with new HTML server features in /README.md
- [ ] T188 [P] Create CHANGELOG.md with v1.1.0 release notes in /CHANGELOG.md
- [ ] T189 Complete project documentation review in /docs/

### Performance Optimization

- [ ] T190 Profile form generation with large schemas in /tests/performance/
- [ ] T191 Optimize validation engine if bottlenecks found in /src/core/schema/
- [ ] T192 Cache compiled schemas to reduce repeated parsing in /src/core/schema/
- [ ] T193 Measure startup time in Docker container in /tests/performance/
- [ ] T194 Optimize Docker image size (target: < 100MB) in /Dockerfile.html

### Security Hardening

- [ ] T195 Input validation on all HTTP endpoints in /src/html/handlers/
- [ ] T196 Add rate limiting to prevent abuse in /src/html/server/
- [ ] T197 Sanitize user input before displaying in forms in /src/html/assets/js/
- [ ] T198 Document security considerations in /SECURITY.md
- [ ] T199 Add Content Security Policy headers in /src/html/server/

### Deployment & DevOps

- [ ] T200 Create Docker Compose file for easy local development in /docker-compose.yml
- [ ] T201 Create Kubernetes deployment manifests (optional) in /k8s/
- [ ] T202 Create systemd service file for standalone deployment in /systemd/configgui-html.service
- [ ] T203 Test deployment in multiple environments (Linux, macOS, Windows)
- [ ] T204 Create installation guide for end users in /INSTALL.md

### Final QA & Release

- [ ] T205 [P] Execute full test suite on release branch
- [ ] T206 [P] Manual QA: Test all user stories end-to-end
- [ ] T207 [P] Performance QA: Verify all success criteria met
- [ ] T208 Create release checklist in /RELEASE_CHECKLIST.md
- [ ] T209 Tag release as v1.1.0 in git with comprehensive release notes
- [ ] T210 Push release to main branch and create GitHub release

---

## Summary & Metrics

### Task Distribution

| Phase | Purpose | Tasks | Story Points | Timeline |
|-------|---------|-------|--------------|----------|
| Phase 1 | Setup & Structure | T001-T020 | 8-13 | 3-5 days |
| Phase 2 | Foundational Core | T021-T089 | 21-34 | 2-3 weeks |
| Phase 3 | User Story 1 (No Qt) | T059-T089 | 21-34 | 1.5-2 weeks |
| Phase 4 | User Story 2 (Form Gen) | T090-T142 | 34-55 | 2-2.5 weeks |
| Phase 5 | User Story 3 (Shared Core) | T143-T174 | 13-21 | 1 week |
| Phase 6 | Polish & Release | T175-T210 | 8-13 | 1 week |
| **TOTAL MVP** | **Phases 1-5** | **174 tasks** | **97-157** | **4-6 weeks** |

### Story Points by User Story

- **User Story 1** (P1 - Deploy without Qt): 21-34 points
- **User Story 2** (P1 - Generate Forms): 34-55 points
- **User Story 3** (P1 - Shared Core): 13-21 points
- **Core Extraction** (Foundational): 21-34 points
- **Setup & Polish**: 16-26 points

### Team Capacity Example (3 developers, 2-week sprints)

**Sprint 1**: Phases 1-2A (Project setup + Core abstraction)
- Developer 1: CMake setup (T001-T009) + Core data structures (T021-T024)
- Developer 2: Directory structure (T010-T017) + Schema extraction (T025-T032)
- Developer 3: I/O extraction (T033-T040) + Documentation (T018-T020)

**Sprint 2**: Phase 2B-C (Core library + Unit tests)
- All: Build system completion (T041-T046)
- Developer 1: Qt integration (T053-T058)
- Developer 2-3: Core unit tests (T047-T052)

**Sprint 3-4**: Phases 3-4 (Parallel implementation)
- Developer 1: HTTP server (T059-T082) + User Story 1 integration (T083-T089)
- Developer 2: Form generation (T090-T124) + Validation (T110-T116)
- Developer 3: Integration tests (T125-T139) + API contract tests (T131-T133)

**Sprint 5**: Phase 5 (Shared core validation)
- All: Core documentation (T143-T174)
- All: Qt/HTML parity verification (T134-T136)

**Sprint 6**: Phase 6 (Polish & release)
- Developer 1: Performance optimization (T190-T194)
- Developer 2: Security hardening (T195-T199)
- Developer 3: Deployment & QA (T200-T210)

---

## Dependencies & Execution Order

### Critical Path

1. ✅ Phase 1 Setup (must complete first, enables Phase 2)
2. ✅ Phase 2 Foundational (BLOCKS all user stories)
3. 🎯 Phase 3 & 4 (can run in parallel once Phase 2 complete)
4. ✅ Phase 5 Shared Core Validation (after 3 & 4, before release)
5. ✅ Phase 6 Polish (final sprint before release)

### Parallelization Opportunities

**Phase 1**: All [P] tasks can run in parallel
- T002-T005 (CMakeLists.txt files) independent
- T010-T017 (directories) independent

**Phase 2**:
- T021-T024 (Core data structures) independent
- T025-T032 (Schema extraction) independent
- T033-T040 (I/O extraction) independent
- T047-T051 (Unit tests) independent
- All can start once corresponding extraction task begins

**Phase 3 & 4**: After Phase 2 complete
- Phase 3 User Story 1 (US1) and Phase 4 User Story 2 (US2) can run in parallel
- Within US2, contract tests (T125-T132) run in parallel with implementation

**Phase 6**: All [P] tasks run in parallel
- Clang-tidy (T175), Coverage (T180), Documentation (T185), Performance (T190), Security (T195) independent

### User Story Testing Sequence

Each user story should be testable independently:

1. **US1 Tests First**: Deploy without Qt, verify server starts, schema list loads
2. **US2 Tests After US1 Stable**: Load schema, generate form, validate, save
3. **US3 Tests Ongoing**: Verify no duplication, both targets work identically

---

## Success Criteria Mapping

| Success Criteria | Task Coverage | Status |
|------------------|---------------|--------|
| **SC-001**: Startup < 2s | T063, T079-T082, T137 | Testable in Phase 3 |
| **SC-002**: Form gen < 1s | T090-T094, T137 | Testable in Phase 4 |
| **SC-003**: Validation < 200ms | T110-T116, T138 | Testable in Phase 4 |
| **SC-004**: 95%+ Qt parity | T134-T136, T157-T160 | Testable in Phase 5 |
| **SC-005**: 1000 properties | T094, T139 | Testable in Phase 4 |
| **SC-006**: 10 concurrent users | T082, T194 | Testable in Phase 6 |
| **SC-007**: Qt-free deployment | T045, T078-T082 | Testable in Phase 3 |
| **SC-008**: 100% core reuse | T162-T163, T157-T160 | Testable in Phase 5 |
| **SC-009**: Upload < 5s (10MB) | T127-T128 | Testable in Phase 4 |
| **SC-010**: Clear deployment docs | T087-T089, T147-T150 | Testable in Phase 6 |

---

## Risk Mitigation & Contingencies

| Risk | Likelihood | Impact | Mitigation | Tasks |
|------|------------|--------|-----------|-------|
| Core extraction breaks Qt | High | Critical | Extract incrementally, test Qt after each step | T053-T058 |
| Form generation complexity | Medium | High | Prototype with simple schemas first, expand incrementally | T091-T094 |
| Validation parity issues | Medium | Medium | Comprehensive comparison tests early | T134-T136 |
| Performance with large schemas | Low | Medium | Implement streaming/caching early | T092, T192 |
| Docker deployment issues | Low | Medium | Test Docker build early in Phase 3 | T078-T082 |
| Build system complexity | Medium | Medium | Validate all 3 scenarios frequently | T007-T009 |

---

## Acceptance Criteria for Phases

### Phase 1 Complete ✅
- [x] All three build scenarios verified
- [x] Directory structure matches plan.md
- [x] CMakeLists.txt files created for all modules
- [x] cpp-httplib dependency configured

### Phase 2 Complete ✅
- [ ] Core module extracted from Qt (Qt-free verification)
- [ ] Core library compiles and links to both Qt and HTML
- [ ] All core unit tests pass (T047-T052)
- [ ] Qt application still fully functional (T056-T058)
- [ ] HTML can proceed without Qt (T045)

### Phase 3 Complete ✅
- [ ] HTML application starts without Qt libraries
- [ ] Main form displays with schema selection
- [ ] Docker container builds and runs
- [ ] Integration tests pass (T083-T086)

### Phase 4 Complete ✅
- [ ] Forms generate from schemas identically to Qt
- [ ] Real-time validation works (< 200ms)
- [ ] Configurations save and load
- [ ] Integration tests pass (T125-T130)
- [ ] Success criteria met (SC-001 through SC-009)

### Phase 5 Complete ✅
- [ ] Shared core documented thoroughly
- [ ] Build process documented and verified
- [ ] Qt application unaffected by changes
- [ ] Zero code duplication verified
- [ ] Constitution check passes (7/7 gates)

### Phase 6 Complete ✅
- [ ] Code quality gates passed (clang-tidy, MISRA)
- [ ] Test coverage 85%+
- [ ] Performance targets met
- [ ] Deployment documentation complete
- [ ] Release candidate ready

---

## Next Steps After MVP

**User Story 4 (P2)**: Configuration export to JSON/YAML
- Task breakdown: T211-T230 (~8-13 story points, 1 week)
- Dependencies: User Story 2 complete

**User Story 5 (P2)**: Docker containerization complete
- Task breakdown: T231-T250 (~8-13 story points, 1 week)
- Builds on Task T078-T082 from Phase 3

**Future Enhancements**:
- Multi-user session management
- Database persistence (PostgreSQL)
- API authentication (OAuth)
- Internationalization (i18n)
- Advanced schema validation (custom formats)

---

## Quick Reference: Running Tasks

```bash
# Build all three scenarios to verify Phase 1
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON ..
make -j8

# Build Qt only
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF ..
make -j8

# Build HTML only (Phase 3+)
cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON ..
make -j8

# Run core unit tests (Phase 2)
ctest -R "core" -V

# Run HTML integration tests (Phase 4)
ctest -R "html" -V

# Start HTML server (Phase 3+)
./build/bin/configgui-html --port 8080

# Build Docker image (Phase 3+)
docker build -f Dockerfile.html -t configgui-html .

# Run Docker container
docker run -p 8080:8080 configgui-html
```

---

**Document Generated**: October 24, 2025  
**Feature Branch**: `002-html-form-generation`  
**Status**: ✅ Phase 2 Task Breakdown Complete - Ready for Implementation
