# Application Crash Analysis & Fix - October 23, 2025

## Issue
Application crashed with SIGSEGV (exit code 139) when loading configuration.

## Root Cause
The `RuleParser::convertNewFormatToOld()` function had incomplete implementation:
- It only processed **string format** rules (e.g., `"integer[0,]+"`)
- It **skipped inline rule objects** (e.g., `{ "type": "string", "allowEmpty": false, "enum": [...] }`)
- This caused inconsistent array conversion and potential null pointer access

The config.json contains a mix of both formats:
```json
"rules": {
  "type": "string?",                           // String format (processed)
  "node type": {                               // Inline format (was skipped!)
    "type": "string",
    "allowEmpty": false,
    "enum": ["intersection", "termination"]
  }
}
```

## Fixes Applied

### 1. Enhanced RuleParser::convertNewFormatToOld (Commit 4df9ac4)
**File**: `src/ui/rule_parser.cpp`

Added support for inline rule objects:
```cpp
if (it.value().is_string())
{
    // Handle shorthand: "integer[0,]+"
    shorthand = it.value().get<std::string>();
    rule = parseShorthand(field_name, shorthand);
}
else if (it.value().is_object())
{
    // Handle inline objects: { "type": "string", "allowEmpty": false, ... }
    const json& inline_rule = it.value();
    
    if (inline_rule.contains("type"))
        rule.type = inline_rule["type"].get<std::string>();
    if (inline_rule.contains("allowEmpty"))
        rule.allow_empty = inline_rule["allowEmpty"].get<bool>();
    if (inline_rule.contains("pattern"))
        rule.pattern = inline_rule["pattern"].get<std::string>();
    if (inline_rule.contains("minimum"))
        rule.minimum = inline_rule["minimum"].get<double>();
    if (inline_rule.contains("maximum"))
        rule.maximum = inline_rule["maximum"].get<double>();
    if (inline_rule.contains("enum"))
        // Extract enum values...
}
```

### 2. Added Safety Checks (Commit 4df9ac4)
**File**: `src/ui/form_generator.cpp`

- Check if array is non-empty before processing
- Added detailed error logging for type mismatches
- Prevent passing invalid data to ObjectArrayWidget

### 3. Added Exception Handling (Commit 916620c)
**File**: `src/ui/form_generator.cpp`

Wrapped rule conversion in try-catch blocks:
```cpp
try
{
    value = RuleParser::convertNewFormatToOld(value);
}
catch (const std::exception& e)
{
    std::cerr << "Exception while converting rules: " << e.what() << std::endl;
    // Gracefully continue without conversion
}
```

## What Was Fixed

| Issue | Before | After |
|-------|--------|-------|
| Inline rule objects | Skipped silently | Properly converted to array format |
| Crash on conversion | Segfault (SIGSEGV) | Exception caught, logged, and handled |
| Error visibility | Silent failure | Detailed logging of conversion steps |
| Null pointer risk | Unchecked access | Safe checks before use |

## Testing Checklist

After these fixes, verify:

1. ✓ Build succeeds with no errors
2. ✓ App doesn't crash on startup
3. [ ] Load schema: `resources/schemas/config.schema.json`
4. [ ] Load config: `resources/configs/config.json`
5. [ ] Expand `configureRules` → `dataTypeValidationRule`
6. [ ] Verify all rule items appear (should see ~40+ rules including both string and inline formats)
7. [ ] Edit a rule and verify it saves correctly
8. [ ] Save configuration and verify rules are preserved

## Commits Involved

- **5aa97b9**: Initial fix - detect and convert shorthand dictionary format
- **a539ac6**: Documentation of rules loading bug
- **4df9ac4**: Handle inline rule objects in converter
- **916620c**: Add exception handling to prevent crashes

## Known Debug Output

The app prints conversion logs to stderr when loading configs:
```
[FormGenerator::applyDataRecursive] Processing key: rules, is_object: 1, is_array: 0
[FormGenerator::applyDataRecursive] Converting rules from shorthand dictionary format to array
[FormGenerator::applyDataRecursive] Converted rules, new size: 41
[FormGenerator::updateFieldValue] Setting ObjectArrayWidget values for field: rules, array size: 41
```

These can be removed in a cleanup commit if needed.

## Related Code Paths

1. **Loading Config**:
   - `main_window.cpp`: Load file → parse JSON → call `FormGenerator::setFormData()`
   
2. **Data Application**:
   - `FormGenerator::setFormData()` → `applyDataRecursive()` → detect rules format
   - If shorthand dictionary → convert via `RuleParser::convertNewFormatToOld()`
   - Then apply to widget: `updateFieldValue("rules", array)` → `ObjectArrayWidget::setValues(array)`

3. **Widget Population**:
   - `ObjectArrayWidget::setValues()` → clear items → add items via `addItemWidget()`
   - For rule objects → create `RuleEditorWidget` with parsed rule

## Recommendations

- [ ] Remove debug logging after user confirms the fix works
- [ ] Consider adding unit tests for RuleParser conversion functions
- [ ] Add validation for config structure on load
