# AutoRules Save Debug Guide

## Issue
When saving a configuration, the autoRules array fields (rdf, shapefiles, 3d-landmarks) are being saved as numbers (e.g., `"rdf": 3`) instead of string arrays.

## Investigation Status
Added comprehensive debug logging to `collectDataRecursive()` to trace:
1. All properties being processed
2. All fields registered in field_widgets_
3. Which fields are found/not found in field_widgets_
4. The widget type for each field  
5. The actual values being collected

## How to Test

### 1. Build the Application
```bash
cd /home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui/build
cmake --build . --target ConfigGUI -j4
```

### 2. Run with Debug Output
```bash
./build/ConfigGUI 2>&1 | tee debug.log
```

### 3. Steps in GUI
1. Click "Load Schema" → select `resources/schemas/config.schema.json`
2. Click "Load Configuration" → select `resources/configs/config.json`
3. Click "Save Configuration" → save as `config_debug.json`
4. Exit the application

### 4. Examine Debug Output
```bash
# Check the debug output
cat debug.log | grep -A 100 "Field registration"
cat debug.log | grep -A 100 "[collectDataRecursive]"
```

## Expected Debug Output

### Field Registration (During Form Creation)
Should see something like:
```
[addFieldToFormWithMetadata] Registered field: rdf, widget type: ArrayWidget
[addFieldToFormWithMetadata] Registered field: shapefiles, widget type: ArrayWidget
[addFieldToFormWithMetadata] Registered field: 3d-landmarks, widget type: ArrayWidget
[addFieldToFormWithMetadata] Registered field: default, widget type: ArrayWidget
[addFieldToFormWithMetadata] Registered field: rootRules, widget type: ArrayWidget
```

### Data Collection (During Save)
Should see something like:
```
[getFormData] Starting form data collection...
[collectDataRecursive] Processing 8 properties
[collectDataRecursive] field_widgets_ has X entries:
  - propertyName1
  - propertyName2
  ...
[collectDataRecursive] Processing property: "autoRules"
  - has properties: YES
  - looking for key: "autoRules"
  - in field_widgets: NO
  - Found nested object, type: "object"
  - Recursing into nested object
  
[collectDataRecursive] Processing 5 properties
[collectDataRecursive] field_widgets_ has X entries:
  - ...
  - rdf
  - shapefiles
  - 3d-landmarks
  - ...
  
[collectDataRecursive] Processing property: "rdf"
  - has properties: NO
  - looking for key: "rdf"
  - in field_widgets: YES
  - FOUND, attempting to get value
  - ArrayWidget, calling getValues()
  - ArrayWidget returned: ["RdfIntegrityRule", "DataTypeValidationRule", ...]
  - Result stored: ["RdfIntegrityRule", "DataTypeValidationRule", ...]

[collectDataRecursive] Processing property: "shapefiles"
  ... similar to rdf ...

[collectDataRecursive] Processing property: "3d-landmarks"
  ... similar to rdf ...

[getFormData] Completed. Result: {...autoRules with correct arrays...}
```

## Possible Issues Found

### Issue 1: Fields Not Registered
If debug shows "NOT FOUND" for rdf/shapefiles/3d-landmarks:
- The widgets aren't being created during form generation
- Check if `addFieldToFormWithMetadata` is being called for nested fields
- Check if widget registration is happening properly

### Issue 2: Wrong Widget Type
If debug shows rdf as QComboBox or QSpinBox instead of ArrayWidget:
- The widget factory is creating the wrong type
- Check widget_factory.cpp to see why arrays of strings aren't getting ArrayWidget

### Issue 3: Fields Are Found But Value Is Wrong
If debug shows ArrayWidget returning numbers instead of arrays:
- ArrayWidget::getValues() is broken
- Check array_widget.cpp getValues() implementation

### Issue 4: Data Gets Overwritten After Collection
If debug shows correct array collection but saved file has numbers:
- Some code is overwriting the collected data after getFormData() returns
- Check main_window.cpp save logic

## Debug Logging Added

File: `src/ui/form_generator.cpp`

### In `getFormData()`:
- Logs when data collection starts and completes
- Dumps full result at end

### In `collectDataRecursive()`:
- Logs all properties being processed
- Lists all entries in field_widgets_ map
- For each property:
  - Logs if it has nested properties
  - Logs if found in field_widgets_
  - Logs widget type when found
  - For ArrayWidget: logs getValues() result
  - Logs final value stored

## Next Steps After Testing

Once you see the debug output, please share:
1. The complete output from test run
2. Or specifically the section from "[getFormData]" onwards
3. The resulting config_debug.json file

This will pinpoint exactly where the data is going wrong.
