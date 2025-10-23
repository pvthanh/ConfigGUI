# AutoRules Save Issue - Investigation Summary

## Investigation Completed
Comprehensive debug logging has been added to trace the exact point where autoRules array fields (rdf, shapefiles, 3d-landmarks) are being saved as numbers instead of string arrays.

## Build Status ✓
- Successfully built with debug logging enabled
- No compilation errors
- All debug statements added and functional

## Debugging Documentation Created

### 1. DEBUGGING_AUTORUN_SAVE.md
- How to run the app and capture debug output
- Step-by-step testing procedure
- Expected vs. actual debug output
- Specific grep commands to analyze output

### 2. AUTORUN_ANALYSIS.md  
- Five detailed hypotheses for the bug
- Evidence patterns for each hypothesis
- Root cause possibilities
- Expected debug flow diagram
- Test execution plan

## Debug Logging Added

### In `collectDataRecursive()` (save operation)
- Logs all properties being processed
- Lists all entries in field_widgets_ map
- For each property:
  - Whether it's a nested object
  - Whether it's found in field_widgets_
  - Widget type when found
  - Actual values being collected
  - Final result stored in JSON

### In `getFormData()` (save trigger)
- Logs when collection starts
- Logs complete result dump at end

### In `addFieldToFormWithMetadata()` (form generation)
- Logs each field registered
- Shows widget type for each field

### In `applyDataRecursive()` (load operation)
- Already had logging for data loading

## Key Observations from Code Analysis

### Data Flow
1. **Loading** (works correctly):
   - Config loaded → `applyDataRecursive()` → `updateFieldValue()` → Widget.setValue()
   - ArrayWidget correctly updated with loaded values

2. **Saving** (has bug):
   - `getFormData()` → `collectDataRecursive()` → Looks for fields in field_widgets_
   - If found: ArrayWidget.getValues() called
   - If not found: Data for that field not collected (left blank/default)
   - Somehow ends up as numbers (2, 3) instead of arrays

### Suspicious Observations
- "rdf" saving as 3 (looks like partial count of defaults?)
- "shapefiles" saving as 2 (exact count of its defaults!)
- "3d-landmarks" saving as 2 (exact count of its defaults!)
- "default" field saves correctly as array
- "rootRules" saves correctly as array

This pattern strongly suggests the values are getting set to the COUNT of array elements somewhere!

### Schema Structure
- autoRules is type: "object" with properties
- Each property (rdf, shapefiles, etc.) is type: "array"
- Each array property has default values from schema
- Each property should create an ArrayWidget

## Next Steps

### For User
1. Build the application
2. Run with debug output capture
3. Load schema and configuration
4. Save configuration
5. Share the debug log output

### Expected Debug Output Will Show
- Whether fields are being registered in field_widgets_
- Whether fields are being found during collection
- What widget types are stored
- Whether ArrayWidget.getValues() is called
- What values are actually collected

### Bug Fix Will Be One Of
1. Fix field registration during form generation
2. Fix widget factory to create correct widget type
3. Fix ArrayWidget.getValues() to not return count
4. Fix field_widgets_ map lifecycle
5. Fix JSON serialization after collection

## Testing
No easy way to test without GUI interaction, but debug logging will make the issue obvious once run.

Current binary: `/home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui/build/ConfigGUI`

Test config files:
- Schema: `/home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui/resources/schemas/config.schema.json`
- Config: `/home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui/resources/configs/config.json`

## Files Modified This Session
1. `src/ui/form_generator.cpp` - Added comprehensive debug logging
2. `DEBUGGING_AUTORUN_SAVE.md` - Testing guide
3. `AUTORUN_ANALYSIS.md` - Technical analysis

## Git Commits
1. "Add comprehensive debug logging for autoRules save issue"
2. "Add comprehensive analysis of autoRules save issue"

## Ready for Testing
All investigation and instrumentation complete. Ready for user to run tests and provide debug output.
