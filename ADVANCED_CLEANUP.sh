#!/bin/bash

# ============================================================================
# ADVANCED ROOT FOLDER CLEANUP AND FILE ORGANIZATION
# Moves shell scripts, C++ files, object files, and less important markdown
# ============================================================================

set -e

ROOT_DIR="/home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui"
TOOLS_SCRIPTS_DIR="$ROOT_DIR/tools/scripts"
TESTS_DIR="$ROOT_DIR/tests"
DOCS_ARCHIVE_DIR="$ROOT_DIR/docs/archive"

echo "=========================================="
echo "  ADVANCED ROOT CLEANUP & ORGANIZATION"
echo "=========================================="
echo ""

# Create directories if they don't exist
echo "[INFO] Creating target directories..."
mkdir -p "$TOOLS_SCRIPTS_DIR"
mkdir -p "$TESTS_DIR"
mkdir -p "$DOCS_ARCHIVE_DIR"
echo "[✓] Directories ready"
echo ""

# ============================================================================
# SECTION 1: MOVE SHELL SCRIPTS TO tools/scripts/
# ============================================================================

echo "=========================================="
echo "  SECTION 1: ORGANIZING SHELL SCRIPTS"
echo "=========================================="
echo ""

SHELL_SCRIPTS=(
    "cleanup_docs.sh"
    "ROOT_CLEANUP.sh"
    "test_ini_loading.sh"
)

echo "[MOVING] Shell scripts to tools/scripts/"
for script in "${SHELL_SCRIPTS[@]}"; do
    if [ -f "$ROOT_DIR/$script" ]; then
        echo "  → $script"
        mv "$ROOT_DIR/$script" "$TOOLS_SCRIPTS_DIR/"
        chmod +x "$TOOLS_SCRIPTS_DIR/$script"
    fi
done
echo "[✓] Shell scripts organized"
echo ""

# ============================================================================
# SECTION 2: MOVE C++ FILES TO tests/ (test sources)
# ============================================================================

echo "=========================================="
echo "  SECTION 2: ORGANIZING C++ SOURCE FILES"
echo "=========================================="
echo ""

CPP_FILES=(
    "test_qt_compatible.cpp"
    "test_schema_load.cpp"
)

echo "[MOVING] C++ test files to tests/"
for cpp_file in "${CPP_FILES[@]}"; do
    if [ -f "$ROOT_DIR/$cpp_file" ]; then
        echo "  → $cpp_file"
        mv "$ROOT_DIR/$cpp_file" "$TESTS_DIR/"
    fi
done
echo "[✓] C++ files organized"
echo ""

# ============================================================================
# SECTION 3: DELETE OBJECT FILES (.o) - Build artifacts
# ============================================================================

echo "=========================================="
echo "  SECTION 3: REMOVING BUILD ARTIFACTS"
echo "=========================================="
echo ""

OBJECT_FILES=(
    "file_utils.o"
    "logger.o"
    "string_utils.o"
)

echo "[DELETING] Object files (build artifacts)..."
for obj_file in "${OBJECT_FILES[@]}"; do
    if [ -f "$ROOT_DIR/$obj_file" ]; then
        echo "  🗑️  $obj_file ($(stat -f%z "$obj_file" 2>/dev/null || stat -c%s "$obj_file" 2>/dev/null) bytes)"
        rm "$ROOT_DIR/$obj_file"
    fi
done
echo "[✓] Build artifacts removed"
echo ""

# ============================================================================
# SECTION 4: MOVE LESS IMPORTANT MARKDOWN FILES TO archive/
# ============================================================================

echo "=========================================="
echo "  SECTION 4: ARCHIVING CLEANUP DOCS"
echo "=========================================="
echo ""

# Keep these markdown files in root (important documentation)
KEEP_MARKDOWN=(
    "README.md"
)

echo "[SCANNING] Markdown files in root..."
markdown_moved=0

for md_file in "$ROOT_DIR"/*.md; do
    if [ -f "$md_file" ]; then
        filename=$(basename "$md_file")
        
        # Check if this markdown should be kept
        should_keep=0
        for keep_file in "${KEEP_MARKDOWN[@]}"; do
            if [ "$filename" = "$keep_file" ]; then
                should_keep=1
                break
            fi
        done
        
        if [ $should_keep -eq 0 ]; then
            echo "  → Moving $filename to archive/"
            mv "$md_file" "$DOCS_ARCHIVE_DIR/"
            markdown_moved=$((markdown_moved + 1))
        else
            echo "  ✓ Keeping $filename (important)"
        fi
    fi
done
echo "[✓] Markdown files archived ($markdown_moved files)"
echo ""

# ============================================================================
# VERIFICATION & SUMMARY
# ============================================================================

echo "=========================================="
echo "  CLEANUP & ORGANIZATION COMPLETE"
echo "=========================================="
echo ""

# Count results
shell_scripts_count=$(find "$TOOLS_SCRIPTS_DIR" -maxdepth 1 -type f -name "*.sh" 2>/dev/null | wc -l)
cpp_files_count=$(find "$TESTS_DIR" -maxdepth 1 -type f -name "*.cpp" 2>/dev/null | wc -l)
object_files_removed=${#OBJECT_FILES[@]}
markdown_moved_count=$markdown_moved

echo "📊 SUMMARY"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Shell scripts moved:      $shell_scripts_count files → tools/scripts/"
echo "C++ test files moved:     $cpp_files_count files → tests/"
echo "Object files removed:     $object_files_removed files (build artifacts)"
echo "Markdown files archived:  $markdown_moved_count files → docs/archive/"
echo ""

echo "📁 ROOT FOLDER NOW CONTAINS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ls -1 "$ROOT_DIR" | grep -E "^[a-zA-Z0-9].*\.(md|txt|log)$" | sed 's/^/  ✓ /' | head -10
echo ""

echo "📍 SCRIPTS LOCATION"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ls -1 "$TOOLS_SCRIPTS_DIR" | sed 's/^/  📄 /'
echo ""

echo "🧪 TEST FILES LOCATION"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -f "$TESTS_DIR/test_qt_compatible.cpp" ] || [ -f "$TESTS_DIR/test_schema_load.cpp" ]; then
    ls -1 "$TESTS_DIR"/*.cpp 2>/dev/null | xargs -I {} basename {} | sed 's/^/  📝 /'
fi
echo ""

echo "📚 ARCHIVED MARKDOWN"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
archive_count=$(find "$DOCS_ARCHIVE_DIR" -maxdepth 1 -type f -name "*.md" 2>/dev/null | wc -l)
echo "Total markdown files in archive: $archive_count"
echo ""

echo "✅ ORGANIZATION COMPLETE"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Next steps:"
echo "1. View root folder:        ls -la"
echo "2. View tools/scripts:      ls -la tools/scripts/"
echo "3. View tests:              ls -la tests/"
echo "4. View archive:            ls -la docs/archive/"
echo "5. Commit changes:          git add -A && git commit -m 'organize: move build artifacts and test files'"
echo ""
