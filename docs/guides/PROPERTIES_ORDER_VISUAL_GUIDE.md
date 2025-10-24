# Properties Order Fix - Visual Summary

## The Fix at a Glance

```
BEFORE:
  - collectDataRecursive() used "required" array order → WRONG
  - generateFromSchema() used "required" array order → WRONG  
  - Schema properties were in random order → WRONG
  Result: Form, save, and schema all had different orders ❌

AFTER:
  - collectDataRecursive() uses schema properties order → CORRECT
  - generateFromSchema() uses schema properties order → CORRECT
  - Schema properties in correct order → CORRECT
  Result: Form, save, and schema all synchronized ✅
```

## Data Flow - Before vs After

### BEFORE (Broken)
```
                    config_new.json                config.schema.json
                         ↓                              ↓
                    3DFileCompletenessRule    GeometryEquivalenceRule
                    BuildingTableRule         ShapefilePrjRule
                    GeometryEquivalenceRule   3DFileCompletenessRule
                    ...                       ...
                         ↓                              ↓
            ┌─────────────────────────────────────────┐
            │  MISMATCH! Different order               │
            └─────────────────────────────────────────┘
                         ↓
        generateFromSchema()  collectDataRecursive()
        (displays form)       (saves JSON)
                ↓                    ↓
        User sees:             Saved as:
        3D first              GeometryEquivalence first
        Different order! ❌
```

### AFTER (Fixed)
```
                    config_new.json                config.schema.json
                         ↓                              ↓
                    3DFileCompletenessRule    3DFileCompletenessRule
                    BuildingTableRule         BuildingTableRule
                    GeometryEquivalenceRule   GeometryEquivalenceRule
                    ...                       ...
                         ↓                              ↓
            ┌─────────────────────────────────────────┐
            │  SYNCHRONIZED! Same order                │
            └─────────────────────────────────────────┘
                         ↓
        generateFromSchema()  collectDataRecursive()
        (displays form)       (saves JSON)
                ↓                    ↓
        User sees:             Saved as:
        3D first              3D first
        Same order! ✅
```

## Code Changes Summary

### Change 1: collectDataRecursive() 
**Location**: `src/ui/form_generator.cpp`, lines 615-625

```cpp
// OLD: Prioritize required array
if (schema.contains("required") && schema["required"].is_array()) {
    // Use required array order
} else {
    // Use properties order
}

// NEW: Always use properties order
for (auto it = properties.begin(); it != properties.end(); ++it) {
    field_order.push_back(it.key());
}
```

### Change 2: generateFromSchema()
**Location**: `src/ui/form_generator.cpp`, lines 64-80

```cpp
// OLD: Prioritize required array
if (schema.contains("required") && schema["required"].is_array()) {
    // Use required array order
} else {
    // Use properties order
}

// NEW: Always use properties order
for (auto it = properties.begin(); it != properties.end(); ++it) {
    field_order.push_back(it.key());
}
```

### Change 3: Schema Properties Reordering
**File**: `resources/schemas/config.schema.json`

```
Before: GeometryEquivalenceRule, ShapefilePrjRule, 3DFileCompletenessRule, ...
After:  3DFileCompletenessRule, BuildingTableRule, GeometryEquivalenceRule, ...
        (matching config_new.json order)
```

## Testing Checklist

- ✅ Schema JSON is valid
- ✅ Schema properties order matches config file
- ✅ Code uses schema properties order (not required array)
- ✅ Application builds successfully
- ✅ Executable is functional
- ✅ Form displays in correct order
- ✅ Saved JSON preserves correct order

## How Users Will See the Fix

### Step 1: Load Configuration
```
File → Open → config_new.json
```

### Step 2: View configureRules Section
The form now shows:
```
▶ configureRules: type nest object that contain individual rule configurations
  ▼ 3DFileCompletenessRule
    ├ requiredSuffixes: [_L.OBJ, _L.MTL, ...]
    └ searchRecursively: true
  ▼ BuildingTableRule
    ├ require_sdo_or_wkt: true
    └ expectedHeaders: [BUILDING_ID, ...]
  ▼ GeometryEquivalenceRule
    └ ...
  (order continues as defined in schema)
```

### Step 3: Modify and Save
When user presses Ctrl+S, the saved JSON will have:
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

**Order is preserved!** ✅

## Performance Impact
- **None** - No performance change (same algorithmic complexity)
- Code is actually **simpler** now (removed conditional logic)

## Backward Compatibility
- ✅ Existing configuration files load correctly
- ✅ All field data is preserved
- ✅ Only the JSON property order changes in output
- ✅ No schema validation changes

## Summary
This fix ensures that **form display order, data collection order, and schema order are perfectly synchronized**. Users see properties in the same order as they will be saved, providing a consistent and predictable user experience.
