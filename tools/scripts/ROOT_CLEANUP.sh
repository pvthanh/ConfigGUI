#!/bin/bash

# ============================================================================
# ROOT FOLDER CLEANUP SCRIPT
# Moves all documentation from root to docs/archive/
# Keeps only: README.md, CMakeLists.txt, Dockerfile*, .* config files
# ============================================================================

set -e

ROOT_DIR="/home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui"
ARCHIVE_DIR="$ROOT_DIR/docs/archive"

echo "=========================================="
echo "  CONFIGGUI ROOT FOLDER CLEANUP"
echo "=========================================="
echo ""
echo "Root folder: $ROOT_DIR"
echo "Archive destination: $ARCHIVE_DIR"
echo ""

# Create archive directory if it doesn't exist
if [ ! -d "$ARCHIVE_DIR" ]; then
    echo "[INFO] Creating archive directory..."
    mkdir -p "$ARCHIVE_DIR"
fi

# Files to KEEP in root (everything else gets moved)
KEEP_FILES=(
    "README.md"
    "CMakeLists.txt"
    ".clang-format"
    ".clang-tidy"
    ".dockerignore"
    ".git"
    ".github"
    ".gitignore"
    ".specify"
    ".vscode"
    "Dockerfile.html"
    "cleanup_docs.sh"
    "ROOT_CLEANUP.sh"
    "build"
    "docs"
    "resources"
    "specs"
    "src"
    "tests"
    "tools"
    "app_output.log"
    "configgui.log"
    "file_utils.o"
    "logger.o"
    "string_utils.o"
    "test_ini_loading.sh"
    "test_qt_compatible"
    "test_qt_compatible.cpp"
    "test_schema_load"
    "test_schema_load.cpp"
)

# Convert to associative array for faster lookup
declare -A keep_map
for item in "${KEEP_FILES[@]}"; do
    keep_map["$item"]=1
done

# Count statistics
moved_count=0
total_size=0

echo "[SCANNING] Scanning root folder for files to archive..."
echo ""

# Find all files in root directory (not subdirectories)
while IFS= read -r file; do
    basename=$(basename "$file")
    
    # Check if file should be kept
    if [ "${keep_map[$basename]}" = "1" ]; then
        continue
    fi
    
    # Skip if it's a directory (we only care about files)
    if [ -d "$file" ]; then
        continue
    fi
    
    # Get file size
    file_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null || echo "0")
    total_size=$((total_size + file_size))
    
    # Move file to archive
    echo "[MOVING] $basename"
    mv "$file" "$ARCHIVE_DIR/"
    moved_count=$((moved_count + 1))
    
done < <(find "$ROOT_DIR" -maxdepth 1 -type f)

echo ""
echo "=========================================="
echo "  CLEANUP RESULTS"
echo "=========================================="
echo ""
echo "✅ Files moved to archive: $moved_count"
echo "📦 Total size archived: $(numfmt --to=iec-i --suffix=B $total_size 2>/dev/null || echo "${total_size} bytes")"
echo ""
echo "Archive location: $ARCHIVE_DIR"
echo ""

# List what's left in root
echo "=========================================="
echo "  REMAINING FILES IN ROOT"
echo "=========================================="
echo ""
ls -la "$ROOT_DIR" | grep -E "^-" | awk '{print $9, "(" $5 " bytes)"}' | column -t
echo ""

# Show archive contents summary
echo "=========================================="
echo "  ARCHIVE CONTENTS"
echo "=========================================="
echo ""
archive_count=$(find "$ARCHIVE_DIR" -maxdepth 1 -type f | wc -l)
echo "📁 Files in archive: $archive_count"
echo ""
echo "Recent files (by date):"
ls -lt "$ARCHIVE_DIR" | head -10 | awk '{print $6, $7, $8, $9}'
echo ""

echo "=========================================="
echo "  ✅ CLEANUP COMPLETE"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Verify root folder is clean: ls -la"
echo "2. Check archive: ls -la docs/archive/"
echo "3. Commit changes: git add -A && git commit -m 'Clean up root folder'"
echo ""
