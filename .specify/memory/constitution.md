# ConfigGUI Constitution
<!-- Qt C++ Configuration GUI Tool -->


## Core Principles

### I. Schema-Driven Dynamic UI
<!-- Schema is the Single Source of Truth -->
All UI components are generated dynamically from JSON Schema; Schema defines data types, constraints, validation rules; No hardcoded UI layouts; Schema changes automatically reflect in GUI without code recompilation -->

### II. JSON as Primary Interface
<!-- JSON Everywhere, Configuration at Core -->
All configuration loaded from JSON files; JSON Schema validation mandatory before UI generation; Bidirectional JSON ↔ GUI sync; Error messages routed to stderr; Valid configurations to stdout/file -->

### III. MISRA C++ Compliance (NON-NEGOTIABLE)
<!-- Safety-Critical Code Quality -->
All C++ code strictly follows MISRA C++ rules; Static analysis mandatory (clang-tidy with MISRA checks); No C-style casts, unions, variable-length arrays; Strong type checking enforced; Rule violations require documented exceptions with justification -->

### IV. Real-Time Validation & User Guidance
<!-- Prevent Invalid State -->
JSON Schema validation occurs on every edit (real-time feedback); Validation errors prevent save; Suggestions/constraints shown before user input; Type mismatches caught immediately; Validation rules included in UI tooltips -->

### V. Multi-Schema Support & Extensibility
<!-- One Tool, Many Configurations -->
Plugin architecture for custom validators; Support arbitrary JSON Schema drafts; Schema-specific UI hints (via x-* JSON Schema extensions); No modification of core required for new schema support -->

## Code Quality & Architecture Standards
<!-- MISRA C++ + Qt Best Practices -->

- **Memory Management**: Smart pointers (std::unique_ptr, std::shared_ptr) only; No raw `new`/`delete` in application code
- **Type Safety**: const-correctness mandatory; Strong typing, avoid void*; Enum class for type-safe enumerations
- **Error Handling**: Qt signals/slots for async operations; No bare exceptions in callbacks; Use Result<T> patterns for fallible operations
- **Qt Integration**: Model-View-Controller pattern for data/UI separation; QAbstractItemModel for dynamic lists/trees; Connect signals before loading config
- **Logging**: Structured logging with severity levels; Debug logs disabled in Release builds; Config change history logged
- **Build Configuration**: CMake or qmake; Static analysis in CI/CD pipeline (clang-tidy -checks=* -header-filter=.*)


## Development Workflow
<!-- Schema-First, Test-Driven -->

1. **Schema Definition**: JSON Schema written first for each config type; Schema reviewed before UI implementation
2. **UI Generation**: Automatic widget generation from schema (text fields, comboboxes, spinners, etc.); Custom renderers for complex types
3. **Validation Layer**: Schema validator instantiated; Real-time validation during editing; Validation feedback immediate
4. **Testing**: Unit tests for schema validators; Integration tests for JSON load/save cycles; UI tests for dynamic widget creation
5. **Documentation**: Schema documentation auto-generated; Validator constraints documented in code; Configuration examples provided per schema


## Governance & Compliance

**Constitution Priority**: Constitution supersedes all design discussions; MISRA C++ compliance is non-negotiable and enforced automatically in CI

**Amendments**: Changes to core principles require schema compatibility verification; Breaking schema changes require major version bump and migration guide

**Compliance Verification**: 
- All PRs must pass clang-tidy static analysis (0 MISRA violations)
- Schema changes validated against existing test configurations
- Code review checklist includes MISRA C++ compliance and type safety verification

**Use [GUIDANCE_FILE]**: For runtime development decisions, schema generation patterns, and Qt integration examples

**Version**: 1.0.0 | **Ratified**: 2025-10-18 | **Last Amended**: 2025-10-18
<!-- Qt C++ ConfigGUI Tool - Dynamic Configuration Management System -->

