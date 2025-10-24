# ConfigureRules Save Order Fix - Complete

## Problem
When saving configuration, the `configureRules` section had incorrect property ordering in the JSON output, even though the top-level properties were now fixed. The order was not matching the GUI display order.

## Root Cause
The `configureRules` object in the schema (`config.schema.json`) did not have a `required` array defined. The `collectDataRecursive()` function checks for a `required` array to preserve order:
- If `required` array exists → use that order (deterministic, matches form generation)
- If no `required` array → fall back to arbitrary iteration order (non-deterministic)

Since `configureRules` had no `required` array, properties were being collected in arbitrary order.

## Solution
Added a `required` array to the `configureRules` object definition in `resources/schemas/config.schema.json`:

```json
"configureRules": {
  "type": "object",
  "description": "type nest object that contain individual rule configurations",
  "additionalProperties": false,
  "properties": {
    // ... all the rules ...
  },
  "required": [
    "3DFileCompletenessRule",
    "BuildingTableRule",
    "GeometryEquivalenceRule",
    "LinksGeometryRule",
    "LinksTableRule",
    "ObjMtlReferenceRule",
    "PoiTableRule",
    "RdfIntegrityRule",
    "RootStructureRule",
    "ShapefileCompletenessRule",
    "ShapefilePrjRule",
    "dataTypeValidationRule"
  ]
}
```

This order matches the order defined in the original `config_new.json` file.

## How It Works
The `collectDataRecursive()` function in `src/ui/form_generator.cpp` (lines 601-750):

1. **For each schema level**, checks if `schema["required"]` exists
2. **If YES**: Iterates through properties in the order specified by the `required` array
3. **If NO**: Falls back to arbitrary iteration through properties

Since we added `required` to `configureRules`, its properties are now collected in the correct order.

## Implementation Details

### File Modified
- `resources/schemas/config.schema.json` - Added `required` array to `configureRules`

### Code Path
```
getFormData()
  → collectDataRecursive(schema, "")
    → For "configureRules" property:
      → Checks: schema["required"] exists? YES ✓
      → Iterates: 3DFileCompletenessRule, BuildingTableRule, ...
      → For each nested rule:
        → collectDataRecursive(rule_schema, "configureRules.RuleName")
        → Uses rule's own "required" array if it exists
```

## Verification

### Test 1: Schema Validation
The schema now correctly includes the `required` array:
```
✓ configureRules has 'required' array
✓ configureRules required fields order:
  1. 3DFileCompletenessRule
  2. BuildingTableRule
  3. GeometryEquivalenceRule
  4. LinksGeometryRule
  5. LinksTableRule
  6. ObjMtlReferenceRule
  7. PoiTableRule
  8. RdfIntegrityRule
  9. RootStructureRule
  10. ShapefileCompletenessRule
  11. ShapefilePrjRule
  12. dataTypeValidationRule
```

### Test 2: Order Logic
The `collectDataRecursive()` logic correctly uses the `required` array when present:
```
✓ configureRules properties order (using 'required' array):
  1. 3DFileCompletenessRule
  2. BuildingTableRule
  3. GeometryEquivalenceRule
  4. LinksGeometryRule
  5. LinksTableRule
  6. ObjMtlReferenceRule
  7. PoiTableRule
  8. RdfIntegrityRule
  9. RootStructureRule
  10. ShapefileCompletenessRule
  11. ShapefilePrjRule
  12. dataTypeValidationRule
```

## Expected Behavior After Fix
When you:
1. Load the configuration
2. Modify some `configureRules` properties (e.g., change `requiredSuffixes` in `3DFileCompletenessRule`)
3. Save the configuration

The saved JSON will now preserve this order:
```json
{
  "configureRules": {
    "3DFileCompletenessRule": { ... },
    "BuildingTableRule": { ... },
    "GeometryEquivalenceRule": { ... },
    "LinksGeometryRule": { ... },
    "LinksTableRule": { ... },
    "ObjMtlReferenceRule": { ... },
    "PoiTableRule": { ... },
    "RdfIntegrityRule": { ... },
    "RootStructureRule": { ... },
    "ShapefileCompletenessRule": { ... },
    "ShapefilePrjRule": { ... },
    "dataTypeValidationRule": { ... }
  }
}
```

This matches the GUI display order and the original config order.

## Build Status
✅ Application builds successfully with the fix.

## Files Changed
1. `resources/schemas/config.schema.json` - Added `required` array to `configureRules` (1 change)
2. No code changes needed - the existing `collectDataRecursive()` logic already handles this correctly

## Summary
The fix is **minimal and targeted**: by adding the `required` array to the schema, the existing `collectDataRecursive()` function now preserves the correct property order for nested objects within `configureRules`. This applies the same ordering fix that was already implemented for root-level properties to nested objects.
