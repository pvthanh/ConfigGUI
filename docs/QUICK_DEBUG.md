# Quick Debug Guide - AutoRules Save Issue

## One-Line Test
```bash
cd /home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui && \
./build/ConfigGUI 2>&1 | tee debug.log &
```

Then in the GUI:
1. **File → Open Schema** → `resources/schemas/config.schema.json`
2. **File → Open Configuration** → `resources/configs/config.json`
3. **File → Save Configuration** → save as `test_output.json`
4. Close app
5. Check log: `cat debug.log | grep -E "rdf|shapefiles" | head -50`

## What to Look For

### If Fields Are Found (GOOD):
```
[collectDataRecursive] Processing property: "rdf"
  - has properties: NO
  - looking for key: "rdf"
  - in field_widgets: YES
  - FOUND, attempting to get value
  - ArrayWidget, calling getValues()
  - ArrayWidget returned: ["RdfIntegrityRule", ...]
```

### If Fields Are NOT Found (BAD):
```
[collectDataRecursive] Processing property: "rdf"
  - has properties: NO
  - looking for key: "rdf"
  - in field_widgets: NO
  - NOT FOUND in field_widgets_
```

### If Wrong Widget Type (BAD):
```
[collectDataRecursive] Processing property: "rdf"
  - ...
  - widget type: QSpinBox  <-- WRONG! Should be ArrayWidget
```

## Three Likely Scenarios

### Scenario 1: Fields Not Registered
- **Evidence**: Debug shows "NOT FOUND" for rdf/shapefiles/3d-landmarks
- **Fix**: Check widget registration in addFieldToFormWithMetadata()

### Scenario 2: Widgets Exist But Not Collected
- **Evidence**: Debug shows fields found, ArrayWidget type, but getValues() returns size instead of array
- **Fix**: Check if there's a bug in ArrayWidget.getValues() or data type casting

### Scenario 3: Data Collected Correctly But Lost in Save
- **Evidence**: Debug shows correct arrays collected, but saved JSON has numbers
- **Fix**: Check if something overwrites data after collection

## Key Debug Output to Share
```bash
# Extract just the relevant lines
grep -E "(Processing property:|in field_widgets:|widget type:|ArrayWidget returned:|NOT FOUND)" debug.log | grep -E "(rdf|shapefiles|3d-landmarks)" > key_findings.txt

# Or get full section around autoRules
grep -A 100 'Processing property: "autoRules"' debug.log > autoRules_section.txt
```

## Files to Check Based on Findings

| Finding | File to Check |
|---------|---|
| Fields not in field_widgets_ | src/ui/form_generator.cpp (addFieldToFormWithMetadata) |
| Wrong widget type | src/ui/widget_factory.cpp (createWidget) |
| ArrayWidget returns number | src/ui/array_widget.cpp (getValues) |
| Data overwrites after collect | src/ui/main_window.cpp (onFileSave) |

## Verify Fix Worked
```bash
# After applying fix and rebuilding:
./build/ConfigGUI 2>&1 | tee debug_after_fix.log &

# In GUI: Load schema, load config, save config
# Then check:
cat debug_after_fix.log | grep "ArrayWidget returned"

# Should see arrays, not numbers:
# [collectDataRecursive] ArrayWidget returned: ["RdfIntegrityRule", "DataTypeValidationRule", ...]
```

## Last Saved Files
- Debug build: `build/ConfigGUI`
- Test schema: `resources/schemas/config.schema.json`
- Test config: `resources/configs/config.json`
