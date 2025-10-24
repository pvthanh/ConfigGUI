# Field Name Collision Fix - COMPLETED

## Problem Identified
The autoRules array fields (rdf, shapefiles, 3d-landmarks) were saving as numbers (2, 3) instead of string arrays. The root cause was a **field name collision** in the `field_widgets_` map:

1. **Schema Definition**: The config schema had the same property names defined in multiple parent objects:
   - `autoRules.rdf` - String array of rules
   - `parallelismByMode.rdf` - Integer value (3)
   - Similar for `shapefiles` and `3d-landmarks`

2. **Collision Mechanism**: 
   - Form generation registered all fields using simple names as keys (e.g., "rdf", "shapefiles")
   - When `parallelismByMode.rdf` was processed AFTER `autoRules.rdf`, it overwrote the previous entry
   - Result: Field lookup returned the wrong widget (QSpinBox instead of ArrayWidget)
   - Saving autoRules.rdf collected data from parallelismByMode.rdf's QSpinBox (value=3)

## Solution Implemented
Changed the field registration system to use **qualified names** (full paths) instead of simple names:

### Example:
```
BEFORE (Collision):
  field_widgets_["rdf"] → overwritten 3 times
  field_widgets_["shapefiles"] → overwritten twice  
  field_widgets_["3d-landmarks"] → overwritten twice

AFTER (Fixed):
  field_widgets_["autoRules.rdf"] → ArrayWidget with 7 items
  field_widgets_["autoRules.shapefiles"] → ArrayWidget
  field_widgets_["autoRules.3d-landmarks"] → ArrayWidget
  field_widgets_["parallelismByMode.rdf"] → QSpinBox with value 3
  field_widgets_["parallelismByMode.shapefiles"] → QSpinBox
  field_widgets_["parallelismByMode.3d-landmarks"] → QSpinBox
```

## Code Changes Made

### 1. Form Generation (src/ui/form_generator.cpp)

**addFieldToForm()**: Modified to call new `addFieldToFormWithPath()` helper
```cpp
void FormGenerator::addFieldToForm(const QString& field_name, const json& field_schema)
{
    // ... setup code ...
    addFieldToFormWithPath(layout_, field_name, field_schema, "");
    // ... remaining code ...
}
```

**addFieldToFormWithPath()**: New function that passes qualified_name through nested field creation
```cpp
void FormGenerator::addFieldToFormWithPath(QVBoxLayout* parent_layout, const QString& field_name, 
                                            const json& field_schema, const QString& qualified_name)
{
    // Builds qualified_name as it recurses through nested structures
    // Passes to addFieldToFormWithMetadata with full qualified name
}
```

**addFieldToFormWithMetadata()**: Updated to accept and use qualified_name
```cpp
void FormGenerator::addFieldToFormWithMetadata(QVBoxLayout* parent_layout, const QString& field_name, 
                                                const json& field_schema, const QString& qualified_name = "")
{
    // Use qualified name for field registration
    QString use_key = qualified_name.isEmpty() ? field_name : qualified_name;
    
    // ... create widget ...
    
    // Register with qualified name
    field_widgets_[use_key] = fw;
    
    // Pass qualified names to nested calls
    addFieldToFormWithPath(nested_layout, child_name, child_schema, use_key + "." + child_name);
}
```

### 2. Data Collection (src/ui/form_generator.cpp)

**getFormData()**: Updated call signature
```cpp
json FormGenerator::getFormData() const
{
    auto result = collectDataRecursive(schema_, "");  // Start with empty parent path
    return result;
}
```

**collectDataRecursive()**: Updated to track and use parent paths
```cpp
json FormGenerator::collectDataRecursive(const json& schema, const QString& parent_path) const
{
    // Build qualified_key for field lookup
    QString qualified_key = parent_path.isEmpty() ? q_key : (parent_path + "." + q_key);
    
    // Look up field using qualified key
    if (field_widgets_.contains(qualified_key))
    {
        const FieldWidget& fw = field_widgets_[qualified_key];
        // Extract values using correct widget type
    }
    
    // Pass qualified keys to nested calls
    data[key] = collectDataRecursive(prop_schema, qualified_key);
}
```

### 3. Data Loading (src/ui/form_generator.cpp)

**applyDataRecursive()**: Updated to track parent_path
```cpp
void FormGenerator::applyDataRecursive(const json& obj, const QString& parent_path)
{
    // Build qualified_key for each field
    QString qualified_key = parent_path.isEmpty() ? q_key : (parent_path + "." + q_key);
    
    // Recurse with new parent_path
    applyDataRecursive(value, qualified_key);
    
    // Pass parent_path to field updates
    updateFieldValue(q_key, value, parent_path);
}
```

**updateFieldValue()**: Updated to use qualified names for lookups
```cpp
void FormGenerator::updateFieldValue(const QString& field_name, const json& value, const QString& parent_path)
{
    // Build qualified key for lookup
    QString qualified_key = parent_path.isEmpty() ? field_name : (parent_path + "." + field_name);
    
    // Look up field using qualified key
    if (!field_widgets_.contains(qualified_key))
        return;
    
    const FieldWidget& fw = field_widgets_[qualified_key];
    // Apply value using correct widget type
}
```

### 4. Header File Updates (src/ui/form_generator.h)

```cpp
private:
    void addFieldToForm(const QString& field_name, const json& field_schema);
    void addFieldToFormWithPath(QVBoxLayout* parent_layout, const QString& field_name, 
                                 const json& field_schema, const QString& qualified_name);
    void addFieldToFormWithMetadata(QVBoxLayout* parent_layout, const QString& field_name, 
                                     const json& field_schema, const QString& qualified_name = "");
    void addSimpleFieldToForm(const QString& field_name, const json& field_schema);
    void updateFieldValue(const QString& field_name, const json& value, const QString& parent_path = "");
    void applyDataRecursive(const json& obj, const QString& parent_path = "");
    json collectDataRecursive(const json& schema, const QString& parent_path = "") const;
```

## Compilation Status
✅ **PASSED** - Application compiled successfully without errors
- Main ConfigGUI target: Built
- Test target: Has unrelated gtest dependency issue (not caused by this fix)

## Expected Behavior After Fix

### Loading a Config File:
1. Form generates fields with qualified names: "autoRules.rdf", "autoRules.shapefiles", etc.
2. User expands autoRules section - sees 7 items in rdf dropdown (✓ correct)
3. User expands parallelismByMode section - sees value 3 for rdf field (✓ correct)

### Saving a Config File:
1. Data collection uses qualified names to look up correct widgets
2. `autoRules.rdf` returns ArrayWidget with all 7 items (✓ correct, not 3)
3. `parallelismByMode.rdf` returns QSpinBox with value 3 (✓ correct)
4. Saved JSON has:
   ```json
   {
     "autoRules": {
       "rdf": ["RdfIntegrityRule", "...6 more items..."],
       "shapefiles": [...],
       "3d-landmarks": [...]
     },
     "parallelismByMode": {
       "rdf": 3,
       "shapefiles": 2,
       "3d-landmarks": 2
     }
   }
   ```

## Data Flow Diagram

### BEFORE (Broken):
```
Form Generation:
  autoRules.rdf → field_widgets_["rdf"] = ArrayWidget
  parallelismByMode.rdf → field_widgets_["rdf"] = QSpinBox  [OVERWRITES!]

Data Collection:
  collectDataRecursive looks for "rdf" → gets QSpinBox
  Saves autoRules.rdf as integer 3 ❌ WRONG

Data Loading:
  updateFieldValue("rdf") → finds QSpinBox
  Sets autoRules.rdf widget to QSpinBox (should be ArrayWidget) ❌ WRONG
```

### AFTER (Fixed):
```
Form Generation:
  autoRules.rdf → field_widgets_["autoRules.rdf"] = ArrayWidget
  parallelismByMode.rdf → field_widgets_["parallelismByMode.rdf"] = QSpinBox

Data Collection:
  collectDataRecursive with parent_path="autoRules"
  looks for "autoRules.rdf" → gets correct ArrayWidget
  Saves autoRules.rdf as array [item1, item2, ...] ✓ CORRECT
  
  collectDataRecursive with parent_path="parallelismByMode"
  looks for "parallelismByMode.rdf" → gets correct QSpinBox
  Saves parallelismByMode.rdf as integer 3 ✓ CORRECT

Data Loading:
  applyDataRecursive with parent_path="autoRules"
  updateFieldValue("rdf", [...], "autoRules")
  builds "autoRules.rdf" → finds correct ArrayWidget
  Sets values correctly ✓ CORRECT
  
  applyDataRecursive with parent_path="parallelismByMode"
  updateFieldValue("rdf", 3, "parallelismByMode")
  builds "parallelismByMode.rdf" → finds correct QSpinBox
  Sets value 3 correctly ✓ CORRECT
```

## Testing Recommendation

To verify this fix works correctly:

1. **Load original config.json**
   - Expand autoRules section → rdf dropdown should show 7 items
   - Expand parallelismByMode section → rdf field should show value 3

2. **Modify and Save**
   - Add/remove item from autoRules.rdf
   - Save configuration

3. **Verify Saved File**
   - autoRules.rdf should have array with correct items
   - parallelismByMode.rdf should still show 3
   - No corruption (numbers appearing where arrays should be)

4. **Reload and Verify**
   - Load saved config again
   - All values should load correctly in their respective widgets

## Secondary Issue Pending
**Save Order Fix**: User reported that save order doesn't match GUI display order. This should be addressed after verifying this collision fix works correctly.

## Files Modified
1. `src/ui/form_generator.h` - Added function signatures with qualified_name and parent_path parameters
2. `src/ui/form_generator.cpp` - Implemented qualified name tracking through all three phases:
   - Form generation (addFieldToForm, addFieldToFormWithPath, addFieldToFormWithMetadata)
   - Data collection (collectDataRecursive)
   - Data loading (applyDataRecursive, updateFieldValue)

## Root Cause Analysis Summary
The fundamental issue was that nested objects in the schema use simple property names that can collide. The fix recognizes that field names must be globally unique by including their full path context, turning a flat namespace into a hierarchical one. This is a common pattern in form generators that handle deeply nested JSON structures.
