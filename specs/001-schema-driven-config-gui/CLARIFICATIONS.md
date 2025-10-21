# Specification Clarifications: Schema-Driven Configuration GUI

**Status**: ✅ ALL CLARIFICATIONS RESOLVED

**Created**: 2025-10-20  
**Resolved**: 2025-10-20  
**Feature**: [spec.md](../spec.md)

---

## Resolved Clarifications

### Question 1: JSON Schema Draft Support ✅

**Decision**: **JSON Schema Draft 7**

**Rationale**:
- Most widely supported version in validator libraries and tools
- Stable specification with comprehensive feature coverage
- Supports 99% of real-world configuration scenarios
- Good balance between features and library ecosystem maturity
- Clear upgrade path to newer drafts in future releases

**Implementation Impact**: Use JSON Schema Draft 7 validator library; verify compatibility with common schema tools

**Future Path**: v1.1 can add support for draft 2020-12 if needed

---

### Question 2: Schema References and Imports ✅

**Decision**: **No $ref Support in v1.0 - Standalone Schemas Only**

**Rationale**:
- Simplifies initial implementation and testing
- Reduces scope and accelerates MVP delivery
- Eliminates complexity of resolving external references
- Most common use case is standalone application-specific schemas
- Clear upgrade path: add $ref support in v1.1

**Scope**: v1.0 supports only self-contained JSON Schema files

**Future Feature**: v1.1 can add support for local file references ($ref with local paths)

**Edge Case Handling**:
- If user loads schema with $ref, display error: "Schema references are not supported in v1.0. Please use a self-contained schema or flatten your schema manually."

---

### Question 3: File Format Support ✅

**Decision**: **JSON and YAML Both Supported**

**Rationale**:
- YAML is industry-standard for configuration files alongside JSON
- Many teams prefer YAML for human readability
- Supporting both maximizes tool applicability
- YAML can be converted to JSON internally for validation
- Users can choose their preferred format for input/output

**Implementation**:
- FR-015: Load both JSON and YAML config files
- FR-016: Allow user to select output format (JSON or YAML) at save time
- Internal validation uses JSON Schema against normalized data structure

**Format Detection**:
- Auto-detect format from file extension (.json, .yaml, .yml)
- Or explicit user selection if format ambiguous

---

## Summary Table

| Question | Decision | Complexity | Future |
|----------|----------|-----------|--------|
| JSON Schema Draft | Draft 7 | Low | Draft 2020-12 in v1.1+ |
| Schema References | None (v1.0) | Low | $ref support in v1.1 |
| File Formats | JSON + YAML | Medium | TOML in v1.2+ (optional) |

---

## Implementation Notes

### JSON Schema Draft 7 Validator
- Libraries: nlohmann/json-schema, rapidjson, or Qt-based validator
- Ensure library supports Draft 7 compliance
- Plan upgrade path for newer drafts

### Standalone Schema Validation
- Add schema validation step on load
- Error message for schemas containing $ref
- Clear documentation: "v1.0 requires self-contained schemas"

### YAML Support
- YAML parser library: libyaml or similar
- Conversion pipeline: YAML → intermediate format → JSON structure → validation
- Output format selector in Save dialog
- File format persistence in recent files

---

## Specification Status

✅ **COMPLETE** - All clarifications resolved  
✅ **READY FOR PLANNING** - All NEEDS CLARIFICATION markers removed  
✅ **QUALITY VERIFIED** - All requirements testable and unambiguous  

**Next Step**: Run `/speckit.plan` to generate detailed architecture and implementation planning



