# Quick Testing Guide - Field Collision Fix

## What Was Fixed
✅ **Field name collision** where autoRules arrays (rdf, shapefiles, 3d-landmarks) were saving as numbers instead of arrays.

**Root Cause**: Both `autoRules.rdf` and `parallelismByMode.rdf` registered with key "rdf" in the field_widgets map, causing the second to overwrite the first.

**Solution**: Use qualified names as keys (e.g., "autoRules.rdf", "parallelismByMode.rdf").

## How to Test

### Step 1: Build and Run
```bash
cd /home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui
./build/ConfigGUI
```

### Step 2: Load Configuration
- Click "Load Configuration" or File → Open
- Select the original `resources/configs/config.json`

### Step 3: Verify autoRules Section
1. Find and expand the "autoRules" section
2. Look for the "rdf" field (should be a dropdown or multi-select)
3. **CHECK**: Should show **7 items** (RdfIntegrityRule, etc.)
4. Do the same for "shapefiles" and "3d-landmarks"

### Step 4: Verify parallelismByMode Section  
1. Find and expand the "parallelismByMode" section
2. Look for the "rdf" field (should be a number input)
3. **CHECK**: Should show **3** (or the value from original config)
4. Do the same for "shapefiles" and "3d-landmarks"

### Step 5: Modify autoRules
1. Click on the "rdf" field in autoRules
2. Try to add or remove an item
3. Verify you can edit the array properly

### Step 6: Save Configuration
1. Click "Save Configuration" or File → Save
2. Save as a new file (e.g., `config_test_new.json`)

### Step 7: Verify Saved File
Open the saved JSON file in a text editor and check:

#### autoRules section should have ARRAYS:
```json
"autoRules": {
  "3d-landmarks": ["3DFileCompletenessRule"],
  "default": ["3DFileCompletenessRule"],
  "rdf": ["RdfIntegrityRule", ...],
  "rootRules": ["RootStructureRule"],
  "shapefiles": ["ShapefileCompletenessRule", ...]
}
```

#### parallelismByMode section should have NUMBERS:
```json
"parallelismByMode": {
  "3d-landmarks": 2,
  "auto": 4,
  "rdf": 3,
  "shapefiles": 2
}
```

### Step 8: Reload and Verify
1. Close the app
2. Run it again
3. Load the file you just saved (config_test_new.json)
4. Verify:
   - autoRules.rdf shows the array you modified
   - parallelismByMode.rdf shows the number value
   - All fields load with correct widget types

## Expected Results (AFTER FIX)

| Field | Widget Type | Value | Expected Behavior |
|-------|-------------|-------|-------------------|
| autoRules.rdf | Array/Dropdown | ["RdfIntegrityRule", "...6 more..."] | ✅ Shows 7 items |
| autoRules.shapefiles | Array/Dropdown | [2+ items] | ✅ Shows array items |
| autoRules.3d-landmarks | Array/Dropdown | [1+ items] | ✅ Shows array items |
| parallelismByMode.rdf | Number Input | 3 | ✅ Shows numeric value |
| parallelismByMode.shapefiles | Number Input | 2 | ✅ Shows numeric value |
| parallelismByMode.3d-landmarks | Number Input | 2 | ✅ Shows numeric value |

## Previous Broken Behavior (BEFORE FIX)
| Field | Problem |
|-------|---------|
| autoRules.rdf | Saved as 3 instead of array ❌ |
| autoRules.shapefiles | Saved as 2 instead of array ❌ |
| autoRules.3d-landmarks | Saved as 2 instead of array ❌ |
| Form would show wrong widget types | Arrays appeared as number inputs ❌ |

## If You See Issues

### Issue 1: autoRules arrays still save as numbers
- **Cause**: Field name collision fix may not have compiled correctly
- **Check**: Verify field_widgets_ keys in debug output contain full paths like "autoRules.rdf"

### Issue 2: parallelismByMode.rdf shows as array instead of number
- **Cause**: Loading logic not using qualified names correctly  
- **Check**: Make sure parent_path parameter is being passed through updateFieldValue calls

### Issue 3: Save order is wrong
- **Note**: This is a SEPARATE ISSUE - user mentioned it as secondary
- **Status**: Will be fixed in next phase after collision fix is verified

## Debug Output Locations
Look in the app console/terminal for debug messages like:
```
[FormGenerator::addFieldToFormWithMetadata] Registering field with key: "autoRules.rdf"
[FormGenerator::collectDataRecursive] Processing property: "rdf", qualified_key: "autoRules.rdf"
[FormGenerator::updateFieldValue] Attempting to update field: "rdf", qualified_key: "autoRules.rdf"
```

These show that qualified names are being used throughout the data pipeline.

## Success Criteria
✅ Test passes if:
1. Load config.json shows autoRules arrays with correct items
2. parallelismByMode shows numeric values  
3. Save produces correctly formatted JSON with arrays in autoRules and numbers in parallelismByMode
4. Reload correctly restores all values in their proper widget types
5. No data corruption (arrays appearing as numbers, etc.)
