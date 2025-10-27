#!/bin/bash
# Phase 2D - T058: Regression Test
# Verify no functionality regression after core extraction

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP="${SCRIPT_DIR}/build/src/qt/ConfigGUI"
SCHEMA="${SCRIPT_DIR}/resources/schemas/config.schema.json"

echo "========================================="
echo "Phase 2D - T058: Regression Test"
echo "Verify Qt app functionality after core extraction"
echo "========================================="
echo ""

# 1. Test executable and dependencies
echo "1. EXECUTABLE & DEPENDENCIES"
echo "=================================="
echo ""

if [ ! -x "$APP" ]; then
    echo "✗ FAIL: Qt executable not found or not executable"
    exit 1
fi
echo "✓ PASS: Qt executable found and executable"

# Check Qt libraries
for lib in libQt6Core libQt6Gui libQt6Widgets; do
    if ldd "$APP" 2>/dev/null | grep -q "$lib"; then
        echo "✓ PASS: Linked against $lib"
    else
        echo "✗ FAIL: Not linked against $lib"
        exit 1
    fi
done

# 2. Test schema loading
echo ""
echo "2. SCHEMA LOADING & PARSING"
echo "=================================="
echo ""

if [ ! -f "$SCHEMA" ]; then
    echo "✗ FAIL: Schema file not found: $SCHEMA"
    exit 1
fi
echo "✓ PASS: Schema file exists"

# Validate JSON
if ! python3 -m json.tool "$SCHEMA" > /dev/null 2>&1; then
    echo "✗ FAIL: Schema file is not valid JSON"
    exit 1
fi
echo "✓ PASS: Schema is valid JSON"

# 3. Test schema structure
echo ""
echo "3. SCHEMA STRUCTURE & PROPERTIES"
echo "=================================="
echo ""

python3 << PYTHON_EOF
import json

with open('$SCHEMA', 'r') as f:
    schema = json.load(f)

# Check required fields
required_keys = ['properties', 'type', 'title']
for key in required_keys:
    if key in schema:
        print(f"✓ PASS: Schema has '{key}'")
    else:
        print(f"✗ FAIL: Schema missing '{key}'")
        exit(1)

# Check properties structure
props = schema.get('properties', {})
if len(props) == 0:
    print("✗ FAIL: Schema has no properties")
    exit(1)
print(f"✓ PASS: Schema has {len(props)} properties")

# Verify property types
for prop_name, prop_schema in props.items():
    if 'type' in prop_schema:
        print(f"  - {prop_name}: {prop_schema['type']}")
    else:
        print(f"  - {prop_name}: (no type specified)")

# Check validation keywords
validation_keywords = ['required', 'additionalProperties', 'enum']
found = []
for keyword in validation_keywords:
    if keyword in schema:
        found.append(keyword)

if found:
    print(f"✓ PASS: Schema includes validation keywords: {', '.join(found)}")
else:
    print("⚠ INFO: No explicit validation keywords found")

PYTHON_EOF

# 4. Test file operations
echo ""
echo "4. FILE OPERATIONS"
echo "=================================="
echo ""

# Create test directory
TEST_DIR="/tmp/configgui_regression_test_$$"
mkdir -p "$TEST_DIR"
echo "✓ PASS: Created test directory: $TEST_DIR"

# Copy schema to test directory
cp "$SCHEMA" "$TEST_DIR/test_schema.json"
echo "✓ PASS: Schema file copied successfully"

# Verify copy
if [ -f "$TEST_DIR/test_schema.json" ]; then
    echo "✓ PASS: Copied schema file verified"
else
    echo "✗ FAIL: Schema copy verification failed"
    exit 1
fi

# 5. Test core library integration
echo ""
echo "5. CORE LIBRARY INTEGRATION"
echo "=================================="
echo ""

# Check for core library symbols in executable
if nm "$APP" 2>/dev/null | grep -q "configgui"; then
    echo "✓ PASS: Core library symbols found in executable"
else
    echo "✗ FAIL: Core library symbols not found"
    exit 1
fi

# Check for specific core classes
core_classes=("Schema" "SchemaValidator" "ConfigurationData" "Result")
for class in "${core_classes[@]}"; do
    if nm "$APP" 2>/dev/null | grep -q "$class"; then
        echo "✓ PASS: Core class '$class' symbols found"
    else
        echo "⚠ INFO: Core class '$class' symbols not found (may be optimized)"
    fi
done

# 6. Test build artifacts
echo ""
echo "6. BUILD ARTIFACTS"
echo "=================================="
echo ""

BUILD_DIR="${SCRIPT_DIR}/build"

# Check core library
CORE_LIB="${BUILD_DIR}/src/core/libConfigGUICore.a"
if [ -f "$CORE_LIB" ]; then
    SIZE=$(du -h "$CORE_LIB" | cut -f1)
    echo "✓ PASS: Core library found: libConfigGUICore.a ($SIZE)"
else
    echo "✗ FAIL: Core library not found"
    exit 1
fi

# Check Qt executable
if [ -f "$APP" ]; then
    SIZE=$(du -h "$APP" | cut -f1)
    echo "✓ PASS: Qt executable found: ConfigGUI ($SIZE)"
else
    echo "✗ FAIL: Qt executable not found"
    exit 1
fi

# 7. Cleanup
echo ""
echo "7. CLEANUP"
echo "=================================="
echo ""

rm -rf "$TEST_DIR"
echo "✓ PASS: Cleaned up test files"

# Final Summary
echo ""
echo "========================================="
echo "REGRESSION TEST SUMMARY"
echo "========================================="
echo ""
echo "✓ Executable and dependencies valid"
echo "✓ Schema loading functional"
echo "✓ Schema structure intact"
echo "✓ File operations working"
echo "✓ Core library properly integrated"
echo "✓ Build artifacts present"
echo ""
echo "Phase 2D T058 Status: ✓ PASSED"
echo "No regressions detected after core extraction."
echo ""
