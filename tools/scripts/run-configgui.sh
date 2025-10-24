#!/bin/bash
# ConfigGUI Quick Run Script

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║       ConfigGUI - Schema-Driven Configuration GUI         ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$SCRIPT_DIR/build"
EXECUTABLE="$BUILD_DIR/ConfigGUI"

# Check if executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: ConfigGUI executable not found at $EXECUTABLE"
    echo "Please build the project first using:"
    echo "  mkdir -p build && cd build && cmake .. && cmake --build ."
    exit 1
fi

echo -e "${GREEN}✓ ConfigGUI found: $EXECUTABLE${NC}"
echo -e "${GREEN}✓ Size: $(du -h "$EXECUTABLE" | cut -f1)${NC}"
echo ""

# Display test resources
echo -e "${BLUE}Available Test Resources:${NC}"
if [ -f "$SCRIPT_DIR/resources/schemas/application-schema.json" ]; then
    echo -e "${GREEN}✓ Schema: resources/schemas/application-schema.json${NC}"
fi
if [ -f "$SCRIPT_DIR/resources/configs/sample-config.json" ]; then
    echo -e "${GREEN}✓ Config: resources/configs/sample-config.json${NC}"
fi

echo ""
echo -e "${BLUE}Starting ConfigGUI...${NC}"
echo ""

# Run the executable
cd "$BUILD_DIR" || exit 1
exec "$EXECUTABLE"
