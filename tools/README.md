# 🛠️ Tools Folder

This folder contains utility scripts and test programs used during development and testing.

## Folder Structure

```
tools/
├── scripts/           - Project utility scripts
└── test-utilities/    - Test programs and utilities
```

## 📂 Contents

### `tools/scripts/`
Project utility scripts for building, testing, and maintenance.

**Files:**
- `run-configgui.sh` - Script to build and run the ConfigGUI application
- `verify_configrules_fix.sh` - Verification script for configureRules save order fix
- `verify_properties_order_fix.sh` - Verification script for schema properties order fix
- `reorder_schema.py` - Python utility to reorder schema properties

**Usage:**
```bash
cd tools/scripts/
./run-configgui.sh                      # Build and run GUI
./verify_configrules_fix.sh             # Verify configRules fix
./verify_properties_order_fix.sh        # Verify properties order fix
python3 reorder_schema.py               # Reorder schema properties
```

### `tools/test-utilities/`
Test programs and verification utilities.

**Files:**
- `test_order_logic.cpp` / `test_order_logic` - Test for order preservation logic
- `test_save_order.cpp` / `test_save_order` - Test for save/load order
- `test_schema_order.cpp` / `test_schema_order` - Test for schema property order

**Usage:**
```bash
cd tools/test-utilities/
./test_order_logic          # Run order logic test
./test_save_order           # Run save order test
./test_schema_order         # Run schema order test
```

## 🔧 Common Tasks

### Build and Run GUI
```bash
cd tools/scripts/
./run-configgui.sh
```

### Verify Recent Fixes
```bash
cd tools/scripts/
./verify_configrules_fix.sh
./verify_properties_order_fix.sh
```

### Run Test Utilities
```bash
cd tools/test-utilities/
for test in test_*; do ./"$test"; done
```

## 📝 Notes

- All test utilities are compiled from C++ source files
- Scripts require bash shell and appropriate permissions
- Python scripts require Python 3.x
- Test utilities verify specific fixes made during development

---

**Last Updated**: October 24, 2025
**Organization Status**: ✅ Clean and organized
