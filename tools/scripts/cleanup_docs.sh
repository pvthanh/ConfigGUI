#!/bin/bash

###############################################################################
# ConfigGUI Documentation Cleanup Script
# Purpose: Archive old documents and organize root folder
# Date: October 28, 2025
###############################################################################

set -e

CONFIGGUI_ROOT=$(pwd)
DOCS_DIR="docs"
ARCHIVE_DIR="docs/archive"

echo "=========================================="
echo "ConfigGUI Documentation Cleanup"
echo "=========================================="
echo ""
echo "Root directory: $CONFIGGUI_ROOT"
echo ""

# Create archive directory if it doesn't exist
if [ ! -d "$ARCHIVE_DIR" ]; then
    mkdir -p "$ARCHIVE_DIR"
    echo "✓ Created archive directory: $ARCHIVE_DIR"
fi

# List of documents to archive (old phase reports, duplicates, obsolete)
ARCHIVE_FILES=(
    "ALL_TASKS_COMPLETED.md"
    "CLEANUP_SUMMARY.md"
    "DOCUMENTATION_COMPLETION_INDEX.md"
    "DOCUMENTATION_INDEX.md"
    "FINAL_COMPLETION_REPORT.md"
    "FINAL_VERIFICATION.txt"
    "INI_BUG_FIXED_SUMMARY.md"
    "INI_CODE_CHANGE_DETAILS.md"
    "INI_DEBUG_GUIDE.md"
    "INI_ISSUE_RESOLVED.md"
    "INI_LOADING_COMPLETION.md"
    "INI_LOADING_FIXED.md"
    "INI_LOADING_PROGRESS.md"
    "INI_PARSING_FIX.md"
    "INI_QUICK_FIX.md"
    "INI_QUICK_REF.md"
    "INI_QUICK_REF_FIX.md"
    "INI_SECTION_PARSING_FIX.md"
    "INTEGRATION_TEST_SUMMARY.md"
    "PHASE_1_2_DELIVERABLES.md"
    "PHASE_2D_COMPLETION_REPORT.md"
    "PHASE_3_4_5_6_IMPLEMENTATION.md"
    "PHASE_3_4_READY_TO_START.md"
    "PHASE_3_6_DELIVERY_SUMMARY.md"
    "PHASE_3_T024_COMPLETION.md"
    "PHASE_3_THROUGH_6_COMPLETION.md"
    "PHASE_5_FINAL_COMPLETE.md"
    "PHASE_5_INI_FORMAT_FIX_COMPLETE.md"
    "PHASE_5_SERVER_BACKUP_RESTORE.md"
    "PROJECT_STATUS_DASHBOARD.md"
    "QT_INI_FORMAT_BEFORE_AFTER.md"
    "QT_INI_FORMAT_FIX_REPORT.md"
    "QT_INI_FORMAT_ISSUE_RESOLVED.md"
    "QT_INI_LOADING_PLAN.md"
    "QUICK_REFERENCE_PHASE_2D.md"
    "SESSION_ACCOMPLISHMENTS_SUMMARY.md"
    "SESSION_COMPLETE.md"
    "SESSION_COMPLETION_SUMMARY.md"
    "SESSION_PHASE_1_2_COMPLETE.md"
    "SESSION_SUMMARY_PHASE_1_2.md"
    "START_HERE.md"
    "T024_FINAL_VERIFICATION.md"
)

# Documents to keep in root
KEEP_FILES=(
    "README.md"
    "CMakeLists.txt"
)

echo "Archiving old documentation files..."
echo ""

ARCHIVED_COUNT=0
KEPT_COUNT=0

for file in "${ARCHIVE_FILES[@]}"; do
    if [ -f "$CONFIGGUI_ROOT/$file" ]; then
        mv "$CONFIGGUI_ROOT/$file" "$ARCHIVE_DIR/$file"
        echo "→ Archived: $file"
        ((ARCHIVED_COUNT++))
    fi
done

echo ""
echo "=========================================="
echo "Files kept in root:"
for file in "${KEEP_FILES[@]}"; do
    if [ -f "$CONFIGGUI_ROOT/$file" ]; then
        echo "✓ $file"
        ((KEPT_COUNT++))
    fi
done

echo ""
echo "=========================================="
echo "NEW DOCUMENTATION in docs/ folder:"
echo ""
echo "Core Documentation Files:"
echo "  ✓ INDEX.md"
echo "  ✓ DESIGN.md"
echo "  ✓ GETTING_STARTED.md"
echo "  ✓ USER_GUIDE.md"
echo "  ✓ API_REFERENCE.md"
echo "  ✓ DEPLOYMENT.md"
echo "  ✓ SOFTWARE_ARCHITECTURE.md"
echo "  ✓ BUG_FIXES_AND_SOLUTIONS.md"
echo "  ✓ INSTALLATION_AND_USAGE_GUIDE.md"
echo ""
echo "Archive:"
echo "  ✓ docs/archive/ ($ARCHIVED_COUNT old files)"
echo ""
echo "=========================================="
echo "Summary:"
echo "=========================================="
echo "✓ Archived: $ARCHIVED_COUNT old files"
echo "✓ Kept in root: README.md, CMakeLists.txt"
echo "✓ Total docs in docs/: 9 files"
echo "✓ Archive directory: docs/archive/"
echo ""
echo "CLEANUP COMPLETE!"
echo ""
echo "Next steps:"
echo "1. Review docs/INDEX.md for navigation guide"
echo "2. Update your bookmarks to docs/GETTING_STARTED.md"
echo "3. All old phase reports are in docs/archive/"
echo ""
