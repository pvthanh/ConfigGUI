# Auto Rules Save Issue - Code Analysis

## Problem Statement
When saving a configuration, autoRules array fields (rdf, shapefiles, 3d-landmarks) are saved as numbers instead of string arrays:
- Expected: `"rdf": ["RdfIntegrityRule", "DataTypeValidationRule", ...]`
- Actual: `"rdf": 3` or `"rdf": 2`

## Data Flow Analysis

### Loading Configuration (Working Correctly)
1. User loads config via GUI → `MainWindow::loadConfiguration(file_path)`
2. File parsed into JSON object
3. `FormGenerator::setFormData(config)` called
4. `FormGenerator::applyDataRecursive(config)` iterates all properties
5. When encountering "autoRules" object, recurses into it
6. For each child property ("rdf", "shapefiles", etc.):
   - Calls `updateFieldValue(field_name, array_value)`
   - Finds widget in `field_widgets_` map using just the field name
   - Casts to ArrayWidget and calls `setValues(array_value)`
   - ArrayWidget renders the array elements in the UI

### Saving Configuration (Bug Location)
1. User clicks Save → `MainWindow::onFileSave()`
2. `FormGenerator::getFormData()` called
3. Returns `collectDataRecursive(schema_)` where schema_ is the top-level schema
4. In `collectDataRecursive()`:
   - Iterates through all top-level properties
   - When encountering "autoRules": {type: "object", properties: {...}}
   - Detects it's a nested object
   - **Recursively calls `collectDataRecursive(autoRules_schema)`**
5. In recursive call for autoRules:
   - Iterates through its properties: rdf, shapefiles, 3d-landmarks, etc.
   - For each property, tries to find it in `field_widgets_`
   - Should find it and call `array_widget->getValues()`
   - Returns collected data back to parent

## Hypothesis Testing (In Order of Likelihood)

### HYPOTHESIS 1: Fields Not Found In Map (MOST LIKELY)
**Scenario**: When `collectDataRecursive()` recursively processes autoRules, it tries to look up "rdf" but `field_widgets_.contains(q_key)` returns false.

**Evidence**:
- Debug logging will show "NOT FOUND" for these fields
- If true, data[key] would never get set, leading to default values
- The saved number (2, 3) might be leftover data or uninitialized values

**Root Cause Possibilities**:
- Fields registered with different names (e.g., "autoRules.rdf" instead of "rdf")
- Fields registered in a local map that isn't `field_widgets_`
- Field registration fails silently during form generation
- Widget creation returns nullptr, preventing registration

**Test**: Look for "NOT FOUND" in debug output for rdf, shapefiles, 3d-landmarks

---

### HYPOTHESIS 2: Wrong Widget Type Stored
**Scenario**: The field is found, but it's storing a QSpinBox or QComboBox instead of ArrayWidget.

**Evidence**:
- Debug output would show widget type is not ArrayWidget
- The number values (2, 3) would be the count from a spin box or index from a combo box
- QComboBox::currentIndex() or QSpinBox::value() called instead of ArrayWidget::getValues()

**Root Cause Possibilities**:
- Widget factory creating wrong type
- Multiple widgets registered with same key (second one overwrites)
- Schema property detection logic wrong (thinking array is integer/enum)

**Test**: Check debug output widget type for these fields

---

### HYPOTHESIS 3: ArrayWidget::getValues() Returns Numbers
**Scenario**: The ArrayWidget exists and is called, but getValues() returns a number instead of array.

**Evidence**:
- Debug output would show "ArrayWidget" for widget type
- ArrayWidget.getValues() debug log would show number instead of array
- This seems very unlikely given the code review

**Root Cause Possibilities**:
- getValues() has a bug where it returns count instead of array
- Cast fails silently and falls through to next type check (QSpinBox?) which returns count

**Test**: Check if debug logs show ArrayWidget called but returns number

---

### HYPOTHESIS 4: Field-Widgets Map Not Shared
**Scenario**: The `field_widgets_` map is cleared or different between form generation and data collection.

**Evidence**:
- Debug output in registration would show all fields registered
- But debug output during collection would show different/fewer entries in map
- Collection would see empty or different map

**Root Cause Possibilities**:
- `clearForm()` called before `getFormData()` (clears field_widgets_)
- Multiple FormGenerator instances created
- field_widgets_ is thread-local or scoped differently than expected

**Test**: Compare field_widgets_ contents in registration logs vs collection logs

---

### HYPOTHESIS 5: JSON Serialization Issue
**Scenario**: Array is collected correctly but somehow serializes to number.

**Evidence**:
- Debug logs would show correct array collected
- But saved JSON file has number
- JSON serialization code has bug

**Root Cause Possibilities**:
- Array cast to int somewhere after collection
- QJsonObject/nlohmann::json conversion bug
- Save function corrupts data

**Test**: Compare collected data in getFormData() logs vs actual saved file

---

## Test Execution Plan

1. **Build with debug logging**
   ```bash
   cd build && cmake --build . --target ConfigGUI
   ```

2. **Run with output capture**
   ```bash
   ./ConfigGUI 2>&1 | tee test_output.log
   ```

3. **In GUI**:
   - Load Schema: `resources/schemas/config.schema.json`
   - Load Config: `resources/configs/config.json`
   - Save Config: As `config_debug.json`

4. **Analyze**:
   ```bash
   # Find field registration messages
   grep "Registered field:" test_output.log | head -20
   
   # Find specific rdf registration
   grep -i "rdf.*widget" test_output.log
   
   # Find data collection messages
   grep "Processing property:" test_output.log | grep -E "(rdf|shapefile|3d-land)"
   
   # Find field lookup results
   grep "in field_widgets:" test_output.log | tail -20
   
   # Full collection output
   grep -A5 "Processing property: \"rdf\"" test_output.log
   ```

## Expected Debug Flow for Correct Execution

```
[Form Generation - During Load Schema]
[addFieldToFormWithMetadata] Registered field: rdf, widget type: ArrayWidget
[addFieldToFormWithMetadata] Registered field: shapefiles, widget type: ArrayWidget
[addFieldToFormWithMetadata] Registered field: 3d-landmarks, widget type: ArrayWidget
[addFieldToFormWithMetadata] Registered field: default, widget type: ArrayWidget
[addFieldToFormWithMetadata] Registered field: rootRules, widget type: ArrayWidget

[Data Loading - During Load Config]
[FormGenerator::applyDataRecursive] Processing key: autoRules, is_object: true, is_array: false
[FormGenerator::applyDataRecursive] Recursing into object: autoRules
[FormGenerator::applyDataRecursive] Processing key: rdf, is_object: false, is_array: true
[FormGenerator::updateFieldValue] Attempting to update field: rdf, field exists: YES
[FormGenerator::updateFieldValue] Setting ArrayWidget values for field: rdf, array size: 7

[Data Saving - During Save Config]
[getFormData] Starting form data collection...
[collectDataRecursive] Processing 8 properties
[collectDataRecursive] field_widgets_ has 25 entries:
  - rdf
  - shapefiles  
  - 3d-landmarks
  - ...
[collectDataRecursive] Processing property: "autoRules"
  - has properties: YES
  - in field_widgets: NO (correct, autoRules itself is not a widget, it's an object container)
  - Found nested object, type: "object"
  - Recursing into nested object
[collectDataRecursive] Processing 5 properties
[collectDataRecursive] Processing property: "rdf"
  - has properties: NO
  - looking for key: "rdf"
  - in field_widgets: YES
  - FOUND, attempting to get value
  - ArrayWidget, calling getValues()
  - ArrayWidget returned: ["RdfIntegrityRule", "DataTypeValidationRule", ...]
  - Result stored: ["RdfIntegrityRule", "DataTypeValidationRule", ...]
[getFormData] Completed. Result: {...proper autoRules with arrays...}
```

## Next Steps

1. Run test with debug guide (see DEBUGGING_AUTORUN_SAVE.md)
2. Share debug log output
3. Based on which hypothesis matches the output, implement fix:
   - Hypothesis 1 → Check field registration path
   - Hypothesis 2 → Check widget factory logic
   - Hypothesis 3 → Check ArrayWidget.getValues()
   - Hypothesis 4 → Check field_widgets_ lifecycle
   - Hypothesis 5 → Check JSON serialization in save

## Files Potentially Involved

- `src/ui/form_generator.cpp` - Main form generation and data collection logic
- `src/ui/widget_factory.cpp` - Widget type selection
- `src/ui/array_widget.cpp` - Array widget value getting/setting
- `src/ui/main_window.cpp` - Save logic that calls getFormData()
- `src/core/schema_loader.cpp` - Schema parsing (unlikely to be issue)
