# Schema Properties Order Fix - COMPLETE

## Problem Statement
When loading and saving configuration, the `configureRules` section was not preserving the correct property order. The issue occurred both when:
1. **Loading**: Form displayed properties in wrong order
2. **Saving**: Saved JSON had properties in wrong order

You clarified that the order should follow the **schema properties definition order** (the order they appear in the schema JSON), NOT the order in the `required` array.

## Root Cause
The code was prioritizing the `required` array for ordering instead of using the schema properties definition order:
- `collectDataRecursive()` checked for `required` array first
- `generateFromSchema()` checked for `required` array first
- Schema properties themselves were also in the wrong order

## Solution - Three Parts

### Part 1: Fix collectDataRecursive() Logic
**File**: `src/ui/form_generator.cpp` (lines 615-625)

**Before**:
```cpp
if (schema.contains("required") && schema["required"].is_array())
{
    // Use required array order
    for (const auto& required_field : schema["required"]) { ... }
}
else
{
    // Fallback to properties order
    for (auto it = properties.begin(); it != properties.end(); ++it) { ... }
}
```

**After**:
```cpp
// Always use properties definition order (not required array order)
for (auto it = properties.begin(); it != properties.end(); ++it)
{
    field_order.push_back(it.key());
}
```

### Part 2: Fix generateFromSchema() Logic  
**File**: `src/ui/form_generator.cpp` (lines 64-80)

**Before**:
```cpp
if (schema.contains("required") && schema["required"].is_array())
{
    // Use required array order
}
else
{
    // Fallback to properties order
}
```

**After**:
```cpp
// Always use properties definition order (order in schema JSON)
for (auto it = properties.begin(); it != properties.end(); ++it)
{
    field_order.push_back(it.key());
}
```

### Part 3: Reorder Schema Properties
**File**: `resources/schemas/config.schema.json`

Used Python script `reorder_schema.py` to reorganize the `configureRules.properties` to the correct order:

```
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

This matches:
- ✓ config_new.json order
- ✓ Desired display order
- ✓ Required array order (as a nice side effect)

## How It Works Now

### Form Generation (Load)
```
generateFromSchema()
  ↓
  Iterates through schema.properties (definition order)
  ↓
  Displays form fields in schema order
  ↓
  Result: Form shows 3DFileCompletenessRule first, then BuildingTableRule, etc.
```

### Data Collection (Save)
```
collectDataRecursive()
  ↓
  Iterates through schema.properties (definition order)
  ↓
  Collects data in schema order
  ↓
  Result: JSON output has properties in schema order
```

## Verification Results
✅ All tests passed:
- Schema JSON is valid
- Schema properties order matches config_new.json
- Form generation uses properties order
- Data collection uses properties order
- Application builds successfully
- Executable is functional

## Expected Behavior After Fix

### When Loading Configuration
1. Open config_new.json in ConfigGUI
2. Form displays `configureRules` section with this order:
   - 3DFileCompletenessRule (expanded first)
   - BuildingTableRule
   - GeometryEquivalenceRule
   - ... (in the order from schema)

### When Saving Configuration
1. Modify any field in configureRules
2. Save the file (Ctrl+S)
3. Saved JSON will have this structure:
```json
{
  "configureRules": {
    "3DFileCompletenessRule": { ... },
    "BuildingTableRule": { ... },
    "GeometryEquivalenceRule": { ... },
    ...
  }
}
```

The order is **preserved** and matches the schema properties definition order.

## Files Changed
1. **`src/ui/form_generator.cpp`**
   - Modified `generateFromSchema()` to use schema properties order (1 change)
   - Modified `collectDataRecursive()` to use schema properties order (1 change)

2. **`resources/schemas/config.schema.json`**
   - Reordered `configureRules.properties` to correct order (12 properties reorganized)

3. **`reorder_schema.py`** (helper script, optional to keep)
   - Python script used to reorganize schema properties

## Build Status
✅ Application compiles successfully with all changes.

## Key Insight
The fix ensures **perfect synchronization** between:
- **Form display order** (what user sees)
- **Data collection order** (what gets saved)
- **Schema definition order** (the source of truth)

All three now use the same ordering mechanism: the order properties appear in the schema JSON file.
