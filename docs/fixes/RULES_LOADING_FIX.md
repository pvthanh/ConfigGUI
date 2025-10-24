# Rules Loading Bug Fix - Detailed Analysis

## Problem Statement
When loading a configuration file, the rules array inside `dataTypeValidationRule.rules` was not being repopulated in the GUI. Users could see the RuleEditorWidget section header but no rule items appeared.

## Root Cause Analysis

### Schema Definition
The schema (`config.schema.json`) defines `dataTypeValidationRule.rules` as an **array of objects**:
```json
"rules": {
  "type": "array",
  "items": {
    "type": "object",
    "properties": {
      "name": { "type": "string" },
      "type": { "type": "string", "enum": ["string", "integer", "float", "boolean"] },
      "allowEmpty": { "type": "boolean" },
      "pattern": { "type": "string" },
      "minimum": { "type": "number" },
      "maximum": { "type": "number" },
      "enum": { "type": "array", "items": { "type": "string" } }
    }
  }
}
```

### Config File Format
However, the actual `config.json` file stores rules in **shorthand dictionary format**:
```json
"dataTypeValidationRule": {
  "rules": {
    "type": "string?",
    "link id": "integer[0,]+",
    "latitude": "float[-90,90]",
    "longitude": "float[-180,180]",
    "node type": { "type": "string", "allowEmpty": false, "enum": ["intersection","termination of a link","change of attribution"] },
    ...
  }
}
```

Where:
- Keys are field names
- Values are either:
  - **Shorthand strings**: `"string?"`, `"integer[0,]+`, `"float[-90,90]"`
  - **Inline rule objects**: `{ "type": "string", "allowEmpty": false, "enum": [...] }`

### Data Loading Flow Issue

When `FormGenerator::setFormData()` is called with the config:

1. `setFormData(config)` calls `applyDataRecursive(config)`
2. `applyDataRecursive` iterates through `config` keys:
   - `configureRules` is an **object** → recurse into it
   - Inside, `dataTypeValidationRule` is an **object** → recurse into it
   - Inside that, `rules` is an **object** (not array!) → **RECURSE** instead of apply!
3. Recursing into the shorthand dictionary, it encounters:
   - `"type": "string?"` → tries `updateFieldValue("type", "string?")` → field not found, skipped
   - `"link id": "integer[0,]+"` → tries `updateFieldValue("link id", ...)` → field not found, skipped
   - etc.
4. The actual `"rules"` ObjectArrayWidget in `field_widgets_` is never populated!

### Root Cause
**The bug**: `applyDataRecursive` treats any object as a nested structure to recurse into. It doesn't know that `rules` in the config is in shorthand dictionary format (not a real nested object), and therefore never calls `updateFieldValue("rules", array_after_conversion)` to populate the ObjectArrayWidget.

## Solution

### Implementation
Modified `FormGenerator::applyDataRecursive()` to:

1. **Detect shorthand format**: When processing a key named `"rules"` that has an object value, check if it's the shorthand dictionary format by examining its structure:
   - If keys have values that are strings (shorthand like `"integer[0,]+"`) or objects with rule properties (`"type"`, `"allowEmpty"`), it's shorthand format
   
2. **Convert format**: Use the existing `RuleParser::convertNewFormatToOld()` to convert:
   - **From**: Shorthand dictionary `{ "field": "type[min,max]", ... }`
   - **To**: Array of rule objects `[{ "name": "field", "type": "type", "minimum": min, "maximum": max }, ...]`

3. **Apply to widget**: After conversion, the array is treated as a leaf value and passed to `ObjectArrayWidget::setValues()` which properly populates the rules items.

### Code Changes
File: `src/ui/form_generator.cpp`

```cpp
void FormGenerator::applyDataRecursive(const json& obj)
{
    for (auto it = obj.begin(); it != obj.end(); ++it)
    {
        const std::string key = it.key();
        json value = it.value();  // Make a copy so we can modify

        // Special handling for "rules" field: convert shorthand dictionary format to array format
        if (key == "rules" && value.is_object() && !value.is_array())
        {
            // Detect if this is shorthand dictionary format
            bool is_shorthand_format = false;
            for (auto rule_it = value.begin(); rule_it != value.end(); ++rule_it)
            {
                if (rule_it.value().is_string() || 
                    (rule_it.value().is_object() && (rule_it.value().contains("type") || rule_it.value().contains("allowEmpty"))))
                {
                    is_shorthand_format = true;
                    break;
                }
            }
            
            if (is_shorthand_format)
            {
                // Convert shorthand dictionary format to array format
                value = RuleParser::convertNewFormatToOld(value);
            }
        }

        if (value.is_object())
        {
            applyDataRecursive(value);
        }
        else
        {
            // Now "rules" array will be applied here after conversion
            updateFieldValue(QString::fromStdString(key), value);
        }
    }
}
```

### Dependencies
- **No new dependencies**: Uses existing `RuleParser::convertNewFormatToOld()` function already in the codebase
- **Backward compatible**: Only detects and converts the known shorthand format
- **Format agnostic**: Can handle both formats transparently

## Testing

### Expected Behavior After Fix
1. Load schema: `resources/schemas/config.schema.json`
2. Load config: `resources/configs/config.json`
3. Navigate to `configureRules` section → expand `dataTypeValidationRule`
4. **Result**: Rules array items should appear and be editable

### Validation Steps
1. Count the number of rule items in `dataTypeValidationRule.rules` (should match the count in config.json)
2. Verify each rule displays the correct field name, type, and constraints
3. Edit a rule and save - verify the shorthand format is properly serialized back to config.json

## Related Components

- **ObjectArrayWidget**: Handles array of complex objects (rules)
- **RuleEditorWidget**: Edits individual rules in shorthand format
- **RuleParser**: Converts between shorthand and verbose formats
- **DictionaryWidget**: Handles the `synonyms` field (similar structure)
- **FormGenerator**: Main form builder and data binding logic

## Commits
- **5aa97b9**: ui: Fix rules repopulation by converting shorthand dictionary format to array format when loading config
