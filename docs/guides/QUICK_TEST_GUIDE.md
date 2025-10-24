# Quick Testing Guide - ConfigGUI

## Current Status: ✅ ALL MAIN ISSUES FIXED

The application now has complete functionality:
- ✅ Schema Loading & Form Generation  
- ✅ Configuration Loading & Form Population
- ✅ Save Functionality

---

## How to Test

### Setup
```bash
cd /home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui/build
./ConfigGUI
```

### Test Case 1: Load Schema
**Steps:**
1. Launch ConfigGUI
2. Click "Load Schema" button
3. Select `resources/schemas/application-schema.json`

**Expected Result:**
- Form appears with sections:
  - **application:** (bold header)
    - enabled: [checkbox]
    - name: [text field with "Enter text..." placeholder]
    - port: [number spinner showing "1"]
    - version: [text field with "Enter text..." placeholder]
  - **database:** (bold header)
    - connection_string: [text field]
    - max_connections: [number spinner]
    - type: [text field]
  - **server:** (bold header)
    - environment: [text field]
    - host: [text field]
    - timeout: [number spinner]
- Status bar shows: "Schema loaded: application-schema.json"
- Success message displays

**Pass Criteria:** ✓ All input fields appear and are ready for user input

---

### Test Case 2: Load Configuration
**Prerequisites:** Must complete Test Case 1 first

**Steps:**
1. After schema is loaded, click "Load Configuration" button
2. Select `resources/configs/sample-config.json`

**Expected Result:**
- Form fields populate with values:
  - application.name: "My Test Application"
  - application.version: "1.0.0"
  - application.enabled: ✓ (checked)
  - application.port: 8080
  - database.type: "PostgreSQL"
  - database.connection_string: "postgresql://user@localhost/appdb"
  - database.max_connections: 100
  - server.host: "api.example.com"
  - server.timeout: 300
- Status bar shows: "Loaded config: sample-config.json"
- Success message displays

**Pass Criteria:** ✓ All fields show loaded values

---

### Test Case 3: Modify & Save
**Prerequisites:** Must complete Test Case 2 first

**Steps:**
1. Modify a field value, e.g., change application.port from 8080 to 9999
2. Modify application.name to "Updated App Name"
3. Click "Save" button

**Expected Result:**
- Success message: "Configuration saved successfully!"
- Status bar shows: "Saved config: sample-config.json"
- File `resources/configs/sample-config.json` is updated on disk

**Pass Criteria:** ✓ Modifications are saved to file

---

### Test Case 4: Verify Save Persistence
**Prerequisites:** Must complete Test Case 3 first

**Steps:**
1. Click "Load Configuration" again
2. Select the same `resources/configs/sample-config.json`

**Expected Result:**
- Form repopulates with previously modified values:
  - application.port: 9999 (changed value)
  - application.name: "Updated App Name" (changed value)
  - Other fields maintain their values

**Pass Criteria:** ✓ Modified values persist after reload

---

## Troubleshooting

### Issue: "Cannot open file" error
- Check that file paths are correct
- Ensure schema and config files exist in `resources/` directory
- Verify file permissions

### Issue: Form not appearing after loading schema
- Ensure schema JSON is valid
- Check that schema has "properties" field
- Check console for JSON parsing errors

### Issue: Form fields empty after loading config
- Verify configuration JSON structure matches schema
- Check that field names in config match those in schema
- Ensure data types are compatible

### Issue: Save fails
- Ensure a configuration file was loaded first
- Check file write permissions
- Verify disk space available

---

## File Locations

**Test Resources:**
- Schema: `/home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui/resources/schemas/application-schema.json`
- Config: `/home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui/resources/configs/sample-config.json`

**Executable:**
- `/home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui/build/ConfigGUI`

**Source Code:**
- Main Window: `src/ui/main_window.h/cpp`
- Form Generator: `src/ui/form_generator.h/cpp`
- Form State: `src/core/form_state.h/cpp`

---

## Configuration File Format

**Before Save:**
```json
{
  "application": {
    "name": "My Test Application",
    "version": "1.0.0",
    "enabled": true,
    "port": 8080
  },
  "database": {
    "type": "PostgreSQL",
    "connection_string": "postgresql://user@localhost/appdb",
    "max_connections": 100
  },
  "server": {
    "host": "api.example.com",
    "timeout": 300
  }
}
```

**After Modification & Save:**
```json
{
  "application": {
    "name": "Updated App Name",
    "version": "1.0.0",
    "enabled": true,
    "port": 9999
  },
  "database": {
    "type": "PostgreSQL",
    "connection_string": "postgresql://user@localhost/appdb",
    "max_connections": 100
  },
  "server": {
    "host": "api.example.com",
    "timeout": 300
  }
}
```

---

## Known Limitations

1. **No validation error display yet** - Form accepts any input
2. **No undo/redo** - Changes cannot be undone
3. **Limited error feedback** - Some errors show generic messages
4. **No YAML support yet** - Only JSON files supported
5. **No multi-file management** - Can only work with one config at a time

---

## Next Features (Priority Order)

1. **Real-time validation** - Show validation errors in form
2. **Save As...** - Allow saving to different file
3. **YAML support** - Load/save YAML formats
4. **Change tracking** - Highlight modified fields
5. **Configuration comparison** - Compare two configs side-by-side

---

## Quick Command Reference

```bash
# Navigate to project
cd /home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui

# Build
cd build && make ConfigGUI

# Run GUI
./build/ConfigGUI

# View test results
cat resources/configs/sample-config.json
```

---

## Issues Found & Fixed

| # | Issue | Status | Fix |
|---|-------|--------|-----|
| 1 | Form not generating from schema | ✅ FIXED | Added nested object detection |
| 2 | Config not populating form | ✅ FIXED | Implemented setFormData() |
| 3 | Save button not working | ✅ FIXED | Implemented onFileSave() |

---

**Ready for testing!** 🚀
