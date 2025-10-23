# Save Workflow Enhancement

## Overview
Improved the configuration save functionality to provide an intuitive workflow for both creating new configurations and updating existing ones.

## Implementation Details

### Workflow Scenarios

#### Scenario 1: Creating a New Configuration (No Config Loaded)
**User Action**: Click "Save Configuration" → File Chooser Opens
- App shows a file dialog asking user where to save
- User selects location and filename (e.g., `my-config.json`)
- App saves to selected location
- `current_config_file_` is updated to the new path

**Flow**:
```
Save clicked → No current_config_file_ → File chooser dialog
→ User selects path → Save to path → Update current_config_file_
```

#### Scenario 2: Updating Existing Configuration (Config Already Loaded)
**User Action**: Edit form → Click "Save Configuration" → Overwrite Confirmation
- App shows confirmation dialog: "Do you want to overwrite the current configuration file?"
- **If Yes**: Overwrites the currently loaded file
- **If No**: Opens file chooser to save to a different location

**Flow**:
```
Save clicked → current_config_file_ exists → Confirmation dialog
→ Yes: Save to current_config_file_
→ No: File chooser dialog → User selects path → Save to new path
   → Update current_config_file_ to new path
```

### Code Changes
File: `src/ui/main_window.cpp`

Key additions:
1. **Check if configuration loaded**:
   - `if (current_config_file_.isEmpty())` → New configuration workflow
   - `else` → Loaded configuration workflow

2. **File Chooser for New Configs**:
   ```cpp
   save_path = QFileDialog::getSaveFileName(this,
       tr("Save New Configuration"), "",
       tr("JSON Configuration Files (*.json);;All Files (*)"));
   ```

3. **Confirmation Dialog for Loaded Configs**:
   ```cpp
   QMessageBox::StandardButton reply = QMessageBox::question(this,
       tr("Overwrite Configuration?"),
       tr("Do you want to overwrite the current configuration file?\n\nFile: %1")
           .arg(QFileInfo(current_config_file_).fileName()),
       QMessageBox::Yes | QMessageBox::No);
   ```

4. **Dynamic Path Update**:
   ```cpp
   if (save_path != current_config_file_)
   {
       current_config_file_ = save_path;  // Update for future saves
   }
   ```

### User Experience Benefits

| Scenario | Before | After |
|----------|--------|-------|
| **New Config** | ❌ Error message | ✅ File chooser to save anywhere |
| **Loaded Config** | ⚠️ Always overwrites | ✅ Confirmation + choice of location |
| **Save As** | ❌ Not possible | ✅ Click "No" in confirmation dialog |
| **Next Save** | N/A | ✅ Remembers new location as default |

### Edge Cases Handled

1. **User cancels file dialog**: Save operation cancelled, form state unchanged
2. **Save to read-only location**: Error message shown, user can retry
3. **Save with schema only (no config)**: Error message guides user to load schema first
4. **Multiple saves to different locations**: Current file path updates each time

### Testing Checklist

- [ ] Load schema only, click Save → File chooser appears
- [ ] Load config, click Save → Overwrite confirmation appears
- [ ] In confirmation, click Yes → Saves to original file
- [ ] In confirmation, click No → File chooser appears, can save to new location
- [ ] After saving to new location, click Save again → Confirms overwriting new location
- [ ] Cancel file dialog → Returns to app without saving
- [ ] Save to read-only location → Error message shown

## Commit
**10c5282**: feat: Improve Save workflow - file chooser for new configs, overwrite prompt for loaded configs
