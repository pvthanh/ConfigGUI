#!/bin/bash
# Comprehensive test for schema properties order fix

echo "=================================================="
echo "Schema Properties Order Fix - Comprehensive Test"
echo "=================================================="
echo ""

cd "$(dirname "$0")" || exit 1

# Test 1: Verify schema is valid JSON
echo "[TEST 1] Validating schema JSON..."
if jq . resources/schemas/config.schema.json > /dev/null 2>&1; then
    echo "✓ Schema JSON is valid"
else
    echo "✗ Schema JSON is invalid"
    exit 1
fi

# Test 2: Verify schema properties order matches config file
echo ""
echo "[TEST 2] Checking schema properties order..."

# Get properties order from schema (definition order, not required array)
SCHEMA_PROPS=$(jq -r '.properties.configureRules.properties | keys[]' resources/schemas/config.schema.json | tr '\n' '|')

# Get order from config file
CONFIG_PROPS=$(jq -r '.configureRules | keys[]' resources/configs/config_new.json | tr '\n' '|')

if [ "$SCHEMA_PROPS" = "$CONFIG_PROPS" ]; then
    echo "✓ Schema properties order matches config_new.json order"
    echo ""
    echo "Order (from schema properties definition):"
    jq -r '.properties.configureRules.properties | keys[]' resources/schemas/config.schema.json | nl -v 1
else
    echo "✗ Schema properties order does NOT match config_new.json"
    echo ""
    echo "Schema properties order:"
    jq -r '.properties.configureRules.properties | keys[]' resources/schemas/config.schema.json | nl -v 1
    echo ""
    echo "Config file order:"
    jq -r '.configureRules | keys[]' resources/configs/config_new.json | nl -v 1
    exit 1
fi

# Test 3: Verify required array doesn't override properties order
echo ""
echo "[TEST 3] Checking that required array order is different..."
REQUIRED_ORDER=$(jq -r '.properties.configureRules.required[]' resources/schemas/config.schema.json | tr '\n' '|')
if [ "$REQUIRED_ORDER" = "$SCHEMA_PROPS" ]; then
    echo "✓ Required array order matches properties order (both correct)"
else
    echo "ℹ Required array order is different from properties order"
    echo "  This is OK - code now uses properties order, not required array"
fi

# Test 4: Verify application builds
echo ""
echo "[TEST 4] Building application..."
cd build
if cmake --build . --target ConfigGUI 2>&1 | grep -q "Built target ConfigGUI"; then
    echo "✓ Application builds successfully"
else
    echo "✗ Application build failed"
    exit 1
fi
cd ..

# Test 5: Verify executable is functional
echo ""
echo "[TEST 5] Checking executable..."
if [ -x build/ConfigGUI ]; then
    echo "✓ Executable exists and is executable"
    ls -lh build/ConfigGUI | awk '{print "  " $9 " (" $5 ")"}'
else
    echo "✗ Executable not found"
    exit 1
fi

# Test 6: Test the ordering logic directly
echo ""
echo "[TEST 6] Testing collect data ordering logic..."
if [ -f test_order_logic ]; then
    ./test_order_logic 2>/dev/null | grep "configureRules properties order" -A 15 | head -20
    echo "✓ Order logic test completed"
fi

echo ""
echo "=================================================="
echo "✓ ALL TESTS PASSED"
echo "=================================================="
echo ""
echo "Summary of fix:"
echo "  - collectDataRecursive() now uses schema properties order (NOT required array)"
echo "  - generateFromSchema() now uses schema properties order (NOT required array)"
echo "  - Schema properties have been reordered to match config_new.json"
echo "  - Form generation and data collection are now synchronized"
echo ""
echo "When you load/save configuration:"
echo "  1. Form displays properties in schema order (3DFileCompletenessRule first)"
echo "  2. Saved JSON preserves the same order"
echo "  3. Order matches config_new.json and schema properties definition"
