# Save Order Issue - Analysis & Solution

## Problem Definition

User reported: "The save order does not match with order in GUI"

## Root Cause Analysis

### How Form is Generated (from `generateFromSchema`)
1. Checks if schema has "required" array
2. If yes: Uses "required" array order to display fields
3. If no: Uses properties iteration order (which in nlohmann/json IS ordered)
4. Result: Fields displayed in either required array order or properties definition order

### How Data is Collected for Save (from `collectDataRecursive`)
1. Iterates through `schema["properties"]` using iterator
2. Since nlohmann/json preserves insertion order, this SHOULD match properties definition order
3. However, depends on JSON parsing order

### Potential Issues
1. **Schema "required" array order ≠ properties definition order**
   - If schema defines "required": ["field3", "field1", "field2"]
   - But properties are defined as: "field1", "field2", "field3"
   - GUI displays: field3, field1, field2 (required order)
   - Save outputs: field1, field2, field3 (properties order)
   - **MISMATCH!**

2. **JSON parsing inconsistency**
   - If properties aren't in the same order as they appear in JSON file
   - Different iteration order could occur (unlikely with nlohmann/json but possible)

## Solution

The fix is to make `collectDataRecursive` follow the SAME order as `generateFromSchema`:

### Implementation Strategy

```
collectDataRecursive() should:
1. Build field_order the same way generateFromSchema does
   - Check for schema["required"] array first
   - Otherwise, iterate through properties normally
2. Iterate through field_order instead of schema["properties"]
3. This ensures save order matches GUI display order

Before:
  for (auto it = properties.begin(); it != properties.end(); ++it)  // Arbitrary order
    data[key] = getFieldValue(key);

After:
  // Build field_order (same as generateFromSchema)
  std::vector<std::string> field_order;
  if (schema.contains("required") && schema["required"].is_array()) {
    for (const auto& required_field : schema["required"]) {
      if (required_field.is_string()) {
        field_order.push_back(required_field.get<std::string>());
      }
    }
  } else {
    for (auto it = properties.begin(); it != properties.end(); ++it) {
      field_order.push_back(it.key());
    }
  }
  
  // Iterate in same order
  for (const auto& field_name : field_order) {
    data[field_name] = getFieldValue(field_name);
  }
```

### Code Changes Required

**File**: `src/ui/form_generator.cpp`
**Function**: `collectDataRecursive()`
**Lines**: ~615-700

Replace the current loop:
```cpp
for (auto it = properties.begin(); it != properties.end(); ++it)
{
    const std::string key = it.key();
    ...
}
```

With order-preserving loop that matches `generateFromSchema`.

## Benefits

1. ✅ **Save order matches GUI display order**
   - User sees fields in one order on screen
   - Config file saves in same order
   - Easier to read and compare manually

2. ✅ **Consistent with schema intent**
   - If schema specifies "required" order, that's intentional
   - Should be preserved in output

3. ✅ **No performance impact**
   - Same number of operations
   - Just different iteration source

## Testing

After implementation:
1. Load config with specific field order in schema
2. Edit a field in GUI
3. Save config file
4. Verify JSON output has fields in same order as GUI display

## Files Affected

- `src/ui/form_generator.cpp` - `collectDataRecursive()` function
- No header changes needed
- No other files affected

## Implementation Priority

- **Category**: UI/Output order correctness
- **Severity**: Medium (cosmetic but important for usability)
- **Complexity**: Low (straightforward code change)
- **Risk**: Low (order change doesn't affect functionality)
