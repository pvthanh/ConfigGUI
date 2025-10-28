#!/bin/bash

# Test script for INI configuration file loading

echo "========================================="
echo "INI Configuration Loading Test"
echo "========================================="
echo ""

# Test 1: List available configurations
echo "[TEST 1] Listing available configurations..."
curl -s http://localhost:8080/api/config/list | jq . 
echo ""

# Test 2: Create a test INI file and save it
echo "[TEST 2] Creating test schema..."
SCHEMA_JSON=$(cat <<'EOF'
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "title": "Test Configuration",
  "properties": {
    "server": {
      "type": "object",
      "title": "Server Settings",
      "properties": {
        "host": { "type": "string", "default": "localhost" },
        "port": { "type": "integer", "default": 8080 },
        "debug": { "type": "boolean", "default": false }
      }
    },
    "database": {
      "type": "object",
      "title": "Database Settings",
      "properties": {
        "name": { "type": "string", "default": "testdb" },
        "maxConnections": { "type": "integer", "default": 10 }
      }
    }
  }
}
EOF
)

# Save schema as JSON file (simulating schema load)
SCHEMA_FILE="/tmp/test_schema.json"
echo "$SCHEMA_JSON" > "$SCHEMA_FILE"
echo "Schema saved to: $SCHEMA_FILE"
echo ""

# Test 3: Create corresponding INI config file
echo "[TEST 3] Creating test INI configuration file..."
INI_FILE="/tmp/test_config.ini"
cat > "$INI_FILE" <<'EOF'
[server]
host=example.com
port=9090
debug=true

[database]
name=production_db
maxConnections=20
EOF

echo "INI config saved to: $INI_FILE"
echo "Content:"
cat "$INI_FILE"
echo ""

# Test 4: Save configuration via API (using INI format)
echo "[TEST 4] Saving configuration to server..."
CONFIG_JSON=$(cat <<'EOF'
{
  "server": {
    "host": "api.example.com",
    "port": 3000,
    "debug": false
  },
  "database": {
    "name": "api_db",
    "maxConnections": 50
  }
}
EOF
)

SAVE_RESPONSE=$(curl -s -X POST http://localhost:8080/api/config/save \
  -H "Content-Type: application/json" \
  -d "{\"filename\": \"test_config.ini\", \"format\": \"ini\", \"data\": $CONFIG_JSON}")

echo "Save response:"
echo "$SAVE_RESPONSE" | jq .
echo ""

# Test 5: List saved configurations
echo "[TEST 5] Listing configurations after save..."
curl -s http://localhost:8080/api/config/list | jq '.configs[]'
echo ""

# Test 6: Get the saved INI file directly
echo "[TEST 6] Retrieving saved INI file from server..."
INI_SAVED_FILE="resources/configs/test_config.ini"
if [ -f "$INI_SAVED_FILE" ]; then
  echo "Content of $INI_SAVED_FILE:"
  cat "$INI_SAVED_FILE"
  echo ""
else
  echo "File not found: $INI_SAVED_FILE"
  echo "Available files in resources/configs:"
  ls -la resources/configs/ 2>/dev/null || echo "Directory not found"
fi
echo ""

echo "========================================="
echo "INI Loading Feature Test Complete"
echo "========================================="
echo ""
echo "To test INI loading in the browser:"
echo "1. Open http://localhost:8080 in your browser"
echo "2. Load the test schema from resources/schemas"
echo "3. Try uploading the INI file: $INI_FILE"
echo "4. Verify form fields populate with INI values"
echo ""
echo "INI File for manual testing: $INI_FILE"
echo ""
