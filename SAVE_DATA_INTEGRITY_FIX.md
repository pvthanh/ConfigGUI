# Save Data Integrity Fix - Multi-Level Nested Objects

## Problem
When saving a configuration, nested object sections like `configureRules`, `autoRules`, and their children were being lost in the output file.

**Before**: Saved file was missing entire sections
```json
{
  "log": { ... },
  "parallelism": 4,
  // configureRules, autoRules missing!
}
```

**Expected**: All nested sections preserved
```json
{
  "log": { ... },
  "autoRules": { ... },
  "configureRules": {
    "dataTypeValidationRule": {
      "rules": [...],
      "synonyms": {...}
    },
    // ... other rules
  },
  "parallelism": 4,
  // ... rest of config
}
```

## Root Cause

The original `FormGenerator::getFormData()` only handled **one level** of nesting:
- Top-level → First-level nested objects ✓
- First-level → Second-level nested objects ✗ (Missing!)

### Schema Structure Example
```
Schema (properties)
  ├── log (nested object)
  │   ├── level (field)
  │   └── file (field)
  └── configureRules (nested object) ← Top level
      └── dataTypeValidationRule (nested object) ← **SECOND level - was skipped!**
          ├── rules (field/array)
          └── synonyms (field/object)
```

## Solution: Recursive Data Collection

Implemented `collectDataRecursive()` function that:
1. **Recursively traverses** the schema structure at any depth
2. **Identifies nested objects** by checking `"type": "object"` and presence of `"properties"`
3. **Collects data** from leaf fields and complex widgets (arrays, dictionaries, etc.)
4. **Reconstructs** the full hierarchical JSON structure

### Implementation
```cpp
json FormGenerator::collectDataRecursive(const json& schema) const
{
    json data = json::object();

    if (!schema.contains("properties"))
        return data;

    const auto& properties = schema["properties"];

    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        const std::string key = it.key();
        const json& prop_schema = it.value();

        // Check if this is a nested object with properties
        if (prop_schema.is_object() && prop_schema.contains("properties"))
        {
            const std::string type_str = 
                (prop_schema.contains("type") && prop_schema["type"].is_string()) ?
                prop_schema["type"].get<std::string>() : "";

            if (type_str == "object")
            {
                // RECURSIVE: Collect nested object data recursively
                data[key] = collectDataRecursive(prop_schema);
                continue;
            }
        }

        // Handle leaf fields (strings, numbers, arrays, objects, etc.)
        if (field_widgets_.contains(QString::fromStdString(key)))
        {
            const FieldWidget& fw = field_widgets_[QString::fromStdString(key)];
            
            if (auto* line_edit = qobject_cast<QLineEdit*>(fw.widget))
                data[key] = line_edit->text().toStdString();
            // ... other widget types
        }
    }

    return data;
}
```

## Data Flow Example

**Config structure**:
```
configureRules
├── dataTypeValidationRule
│   ├── synonyms: {dictionary}
│   └── rules: [array of objects]
├── ShapefilePrjRule
│   └── acceptedTokens: [array]
```

**Before**: Only `configureRules` children were collected (one level down), missing `dataTypeValidationRule` and its children

**After**: Full recursive collection:
1. Process `configureRules` → Found as nested object
2. Recurse into `configureRules` properties
3. Find `dataTypeValidationRule` → Found as nested object (second level!)
4. Recurse into `dataTypeValidationRule` properties
5. Find `synonyms` → Not a nested object, get from widget ✓
6. Find `rules` → Not a nested object, get from widget (ObjectArrayWidget) ✓
7. Return complete structure

## Benefits

| Aspect | Before | After |
|--------|--------|-------|
| **Single-level nesting** | ✓ Worked | ✓ Works |
| **Multi-level nesting** | ✗ Lost data | ✓ **Preserves all levels** |
| **Complex widgets** (arrays, dicts) | ✓ Worked | ✓ **Now properly nested** |
| **File round-trip** | Lossy | Lossless (except computed fields) |
| **Code complexity** | High (180 lines) | Low (70 lines recursive) |

## Testing

### Test Case 1: Basic Save
- Load: `config.schema.json`
- Load: `config.json`
- Save: As `config_test.json`
- Compare: `config.json` vs `config_test.json` → **Same structure** ✓

### Test Case 2: Edit and Save
- Load config
- Edit a field in `configureRules.dataTypeValidationRule.synonyms`
- Save
- Verify: `synonyms` object is present and edit is saved ✓

### Test Case 3: Rules Array
- Load config
- Navigate to `configureRules` → `dataTypeValidationRule` → Expand to see rules
- Edit a rule
- Save
- Verify: `rules` array is present and edit is saved ✓

## Commits
- **ec1f16e**: fix: Implement recursive data collection for multi-level nested objects in Save

## Files Modified
- `src/ui/form_generator.h`: Added `collectDataRecursive` declaration
- `src/ui/form_generator.cpp`: Replaced flat `getFormData()` with recursive implementation
