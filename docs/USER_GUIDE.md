# ConfigGUI User Guide

**Version**: 2.0  
**Date**: October 28, 2025  
**Audience**: End users, configuration administrators

---

## Table of Contents

1. [Overview](#overview)
2. [User Interface](#user-interface)
3. [Core Workflows](#core-workflows)
4. [Advanced Features](#advanced-features)
5. [File Format Details](#file-format-details)
6. [Best Practices](#best-practices)
7. [Tips and Tricks](#tips-and-tricks)

---

## Overview

ConfigGUI is a configuration management tool with two interfaces:
- **Qt GUI**: Desktop application with native OS look-and-feel
- **Web Interface**: Browser-based, accessible from any device

Both interfaces provide identical functionality for:
- Loading JSON schemas
- Generating dynamic forms from schemas
- Loading and editing configurations
- Saving configurations in multiple formats (JSON, YAML, INI)
- Backing up to server

---

## User Interface

### Qt Desktop Application

```
┌─ ConfigGUI ────────────────────────────────────────┐
│                                                    │
│  File  Edit  Help                                  │
│ ┌──────────────────────────────────────────────┐  │
│ │           📋 Load Schema                     │  │
│ │           ⚙️ Load Configuration              │  │
│ │                                              │  │
│ │  ┌────────────────────────────────────────┐  │  │
│ │  │ Database Section (collapsible)         │  │  │
│ │  │  ├─ Host:   ________________           │  │  │
│ │  │  ├─ Port:   [  5432  ]                 │  │  │
│ │  │  └─ Name:   ________________           │  │  │
│ │  │                                         │  │  │
│ │  │ Logging Section                        │  │  │
│ │  │  ├─ Level:  [- Select -]               │  │  │
│ │  │  │          [DEBUG]                    │  │  │
│ │  │  │          [INFO]                     │  │  │
│ │  │  └─ Rules:  [+ -]                      │  │  │
│ │  │     ├─ logger=info                     │  │  │
│ │  │                                         │  │  │
│ │  │                                         │  │  │
│ │  │              💾 Save  🔄 Reset         │  │  │
│ │  └────────────────────────────────────────┘  │  │
│ │                                              │  │
│ └──────────────────────────────────────────────┘  │
│                                                    │
│ Status: Ready   |  ✓ Valid form                   │
└────────────────────────────────────────────────────┘
```

### Web Interface

```
┌─ ConfigGUI Server (http://localhost:8080) ─────────┐
│                                                     │
│  ┌─ Main Menu ────────────────────────────────┐   │
│  │                                            │   │
│  │     📋 Load Schema                         │   │
│  │     Drop schema here or click to browse    │   │
│  │                                            │   │
│  │     ⚙️ Load Configuration                   │   │
│  │     Drop config here or click to browse    │   │
│  │                                            │   │
│  └────────────────────────────────────────────┘   │
│                                                     │
│  ┌─ Form View ────────────────────────────────┐   │
│  │ Configuration Form                         │   │
│  │                                            │   │
│  │ ▶ Server Settings                         │   │
│  │   ├─ Host: ______________                 │   │
│  │   └─ Port: [5432]                         │   │
│  │                                            │   │
│  │ ▼ Advanced                                 │   │
│  │   └─ Options: +  -                        │   │
│  │                                            │   │
│  │          💾 Save Configuration             │   │
│  │                                            │   │
│  └────────────────────────────────────────────┘   │
│                                                     │
│  Status: ✓ Connected to server                    │
└─────────────────────────────────────────────────────┘
```

### Navigation States

| Screen | Purpose | Actions |
|--------|---------|---------|
| **Main Menu** | Start point | Load Schema, Load Config |
| **Schema Selector** | Choose schema | Click schema card to load |
| **Config Selector** | Browse configs | Click config to load in form |
| **Form Generator** | Edit configuration | Modify fields, Save, Reset |

---

## Core Workflows

### Workflow 1: Loading and Editing Configuration

**Objective**: Load an existing configuration, edit it, and save with new values

**Steps**:

1. **Start Application**
   - Qt: Double-click ConfigGUI executable
   - Web: Open http://localhost:8080

2. **Load Schema**
   - Click "📋 Load Schema" button
   - Select schema file: `config.schema.json`
   - See form auto-generate with fields from schema

3. **Load Configuration**
   - In the generated form, click "⚙️ Load Configuration in Form"
   - Modal dialog appears showing saved configurations
   - Select existing config: e.g., `production.ini`
   - Form pre-fills with configuration values

4. **Edit Values**
   - Change any field values as needed
   - Validation occurs automatically
   - Fields turn red if invalid

5. **Save Configuration**
   - Click "💾 Save Configuration" button
   - Format dialog appears: choose JSON/YAML/INI
   - Filename dialog: enter new name like `production_v2.ini`
   - Configuration saves to:
     - Local PC (file download/file system)
     - Server backup (HTTP API)
   - Confirmation toast shows success

### Workflow 2: Creating Configuration from Scratch

**Objective**: Create new configuration from schema defaults

**Steps**:

1. **Load Schema**
   - Start application
   - Load schema that has default values defined

2. **Review Defaults**
   - Form shows all default values from schema
   - Some fields pre-populated

3. **Customize Values**
   - Override defaults with actual values
   - Add optional fields as needed
   - Use +/- buttons for arrays and dictionaries

4. **Validate**
   - Check all required fields marked with `*`
   - Ensure values meet constraints (min/max, patterns)
   - Form shows validation status

5. **Save**
   - Click "Save" when form is valid
   - Choose format and filename
   - Save locally and/or to server

### Workflow 3: Converting Between Formats

**Objective**: Load JSON configuration and save as INI

**Steps**:

1. **Load Original**
   - Load schema
   - Load JSON configuration file

2. **Form Displays**
   - Form shows all values from JSON
   - Structure preserved (nested objects, arrays)

3. **Change Format**
   - Click "Save Configuration"
   - Select different format: INI
   - Enter new filename with .ini extension

4. **Save**
   - System converts JSON to INI format
   - Files saves with structure preserved
   - Nested objects become `[section.subsection]`
   - Arrays become `items[0]=value`

5. **Verify**
   - Open saved file to verify format
   - Load file back to confirm round-trip works

---

## Advanced Features

### Nested Objects (Collapsible Sections)

**Definition**: Objects within objects, visualized as expandable sections

**How to Use**:
- Click on section header to expand/collapse
- Arrow indicates expand (▶) or collapse (▼) state
- Left border shows section nesting level
- Changes persist in nested fields

**Example**:
```json
{
  "server": {
    "http": {
      "host": "0.0.0.0",
      "port": 8080
    }
  }
}
```

Form shows:
```
▶ Server
  ▶ Http
    - Host: ________________
    - Port: [8080]
```

### Arrays (Expandable Lists)

**Definition**: Multiple items of same type, shown as list with +/- buttons

**How to Use**:
- **Add Item**: Click `+` button to add new item to array
- **Remove Item**: Click `-` button to remove last item
- **Edit Values**: Each item has editable fields
- **Reorder**: Drag items to reorder (in some implementations)

**Array Types**:
- **Simple Array**: `["item1", "item2"]` - text inputs
- **Object Array**: `[{name, value}, ...]` - expandable objects

**Example**:
```
▶ Rules (array of objects)
  + -
  ▼ Rule #1
    - Name: database
    - Level: info
  ▶ Rule #2
    - Name: cache
    - Level: debug
```

### Dictionary Fields (Key-Value Pairs)

**Definition**: Flexible key-value collection with user-defined keys

**How to Use**:
- **Add Pair**: Click `+` to add empty key-value row
- **Remove Pair**: Click `-` to remove last pair
- **Edit Keys**: First field is the key
- **Edit Values**: Second field is the value

**Example**:
```
▼ Environment Variables
  + -
  [DATABASE_URL    ] = [postgresql://localhost]
  [API_KEY         ] = [sk-xxxxxxxxxxxx]
  [DEBUG_MODE      ] = [false]
```

### Type-Specific Fields

| Type | Input Widget | Example |
|------|---------|---------|
| **String** | Text input | "localhost" |
| **Number** | Number input | 5432 |
| **Boolean** | Checkbox | ☑ Enabled |
| **Enum/Select** | Dropdown | [- Choose -] ▼ |
| **Textarea** | Multi-line | Long text... |
| **Array** | List +/- | [item1, item2] |
| **Object** | Collapsed section | ▶ ServerConfig |

### Form Validation

**Real-Time Validation**:
- As you type, fields validate against schema
- Invalid fields show with red border
- Help text explains requirement
- Save button disabled if form invalid

**Validation Rules** (from schema):
- **Type checking**: String, number, boolean, etc
- **Required fields**: Must have value (marked with `*`)
- **Min/Max constraints**: Numbers within range
- **Pattern matching**: Strings match regex
- **Enum validation**: Must be one of allowed values
- **Format validation**: Email, date, URI, etc

**Error Display**:
```
[ Host          ]  ← Valid field (green)
^
[ Port: 99999   ]  ← Invalid (red border)
  ⚠ Port must be between 1 and 65535
```

### Format Selection Dialog

**When Saving**, choose format:

```
┌─ Save Configuration Format ────┐
│                                │
│ Select format:                 │
│ ○ JSON (.json)                 │
│ ○ YAML (.yaml)                 │
│ ○ INI (.ini)                   │
│                                │
│       [Cancel]  [Continue]     │
└────────────────────────────────┘
```

**Format Comparison**:

| Aspect | JSON | YAML | INI |
|--------|------|------|-----|
| **Readability** | Good | Excellent | Good |
| **Nesting** | ✅ Deep nesting | ✅ Indentation | ⚠️ Dot notation |
| **Arrays** | ✅ Native | ✅ Native | ⚠️ Index notation |
| **Binary** | ❌ No | ❌ No | ❌ No |
| **Comments** | ❌ No | ✅ Yes | ✅ Yes |
| **Complexity** | Good | Best | Simple configs |

---

## File Format Details

### JSON Format

**Example**:
```json
{
  "database": {
    "host": "localhost",
    "port": 5432,
    "credentials": {
      "username": "admin",
      "password": "secret"
    }
  },
  "rules": [
    { "name": "rule1", "level": "info" },
    { "name": "rule2", "level": "debug" }
  ]
}
```

**Features**:
- ✅ Supports arbitrary nesting depth
- ✅ Native arrays and objects
- ✅ Type inference (numbers, booleans, strings)
- ❌ No comments in JSON
- ✅ Best for structured data

### YAML Format

**Example**:
```yaml
database:
  host: localhost
  port: 5432
  credentials:
    username: admin
    password: secret

rules:
  - name: rule1
    level: info
  - name: rule2
    level: debug
```

**Features**:
- ✅ Human-readable indentation
- ✅ Comments with `#`
- ✅ Supports all JSON features
- ✅ Type inference
- ✅ Best for configuration files

### INI Format (Sections)

**Example**:
```ini
[database]
host=localhost
port=5432
username=admin
password=secret

[database.credentials]
method=basic
timeout=30

[rules]
rules[0].name=rule1
rules[0].level=info
rules[1].name=rule2
rules[1].level=debug
```

**Features**:
- ✅ Simple key-value pairs
- ✅ Section organization with `[SectionName]`
- ✅ Nested sections: `[parent.child]`
- ✅ Comments with `;` or `#`
- ⚠️ Arrays use index notation: `key[0]`
- ✅ Best for simple, flat configurations

---

## Best Practices

### 1. Always Start with Schema

✅ **DO**: Load schema first, then load/create configurations
```
1. Load Schema → Defines structure
2. Generate Form → Based on schema
3. Load Config → Validates against schema
4. Save → Type-safe, validated
```

❌ **DON'T**: Load configuration without schema
```
- No validation occurs
- Form structure undefined
- Data integrity at risk
```

### 2. Use Appropriate Format

**For Development**: Use YAML or JSON
- Better readability
- Supports comments
- Easier to track in git

**For Deployment**: Use INI
- Simpler to manage
- Smaller file size
- Easier for shell scripts

**For APIs**: Use JSON
- Standard API format
- Browser-friendly
- Easy serialization

### 3. Validate Before Saving

✅ **DO**:
- Ensure all required fields (marked `*`) have values
- Check numerical ranges are within limits
- Verify email/URL formats
- Test round-trip: save and reload to verify

❌ **DON'T**:
- Save invalid configurations
- Ignore validation warnings
- Assume defaults without checking

### 4. Use Hybrid Storage

✅ **DO**: Save to both local PC and server
```
Click Save
→ Choose format
→ System saves both:
   ├─ Local: file download/filesystem
   └─ Server: HTTP backup
→ Both available for restore
```

❌ **DON'T**: Save only to server
- If server goes down, config inaccessible
- No client-side backup

### 5. Version Control Configurations

**For Important Configs**:
```bash
# Save to version control
git add production.yaml
git commit -m "Update production config: db timeout 30->60"
git push
```

**Use Meaningful Names**:
- ✅ `production_v2.yaml` - Clear version
- ✅ `app_config_2025-10-28.ini` - Date-stamped
- ❌ `config1.json` - Not descriptive
- ❌ `backup.yaml` - Which version?

---

## Tips and Tricks

### Tip 1: Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Tab` | Move to next field |
| `Shift+Tab` | Move to previous field |
| `Enter` | Submit form / Save config |
| `Escape` | Close dialogs |
| `Ctrl+S` | Save configuration |

### Tip 2: Quick Format Conversion

```
1. Load config in JSON format
2. Save as YAML (click Save → select YAML)
3. Same config now in YAML format
4. Repeat for other formats
```

### Tip 3: Bulk Updates with Arrays

```
Instead of clicking Add 10 times:
1. Edit raw JSON/YAML manually (10 items)
2. Save file locally
3. Load file back into form
4. Form shows all 10 items
5. Edit individual items as needed
```

### Tip 4: Field-Level Help

```
Hover over field description (if present):
- Shows constraint information
- Displays allowed range
- Explains field purpose
- Suggests valid formats
```

### Tip 5: Export/Backup

```
Before making changes:
1. Load existing config
2. Save as "backup_date.format"
3. Now make changes safely
4. If issues, reload from backup
```

### Tip 6: Nested Section Navigation

```
For deep nested objects:
1. Use arrow buttons to expand/collapse
2. Only expand sections you need to edit
3. Collapsed sections don't affect save
4. Keeps form compact and readable
```

### Tip 7: Dictionary (Key-Value) Fast Entry

```
To add many key-value pairs:
1. Click +  for each pair
2. Fill in first pair
3. Tab to move between rows
4. Continue entering data
5. Empty rows ignored on save
```

---

## Troubleshooting

### Issue: Form Won't Load

**Symptom**: "Cannot load schema" error

**Solution**:
1. Check schema file is valid JSON
2. Verify file permissions (readable)
3. Check file path is correct
4. Try re-uploading file

### Issue: Configuration Doesn't Load

**Symptom**: "Configuration doesn't match schema" error

**Solution**:
1. Ensure config is for correct schema
2. Check config file format is valid
3. Verify required fields are present
4. Check data types match (string vs number)

### Issue: Can't Save Configuration

**Symptom**: Save button disabled or "Cannot save" error

**Solution**:
1. Check if form is valid (no red fields)
2. Ensure all required fields have values
3. Check file permissions in save directory
4. Verify disk space is available

### Issue: Server Connection Lost

**Symptom**: Cannot reach server backup

**Solution**:
1. Verify server is running: `curl http://localhost:8080`
2. Check firewall isn't blocking port 8080
3. Try localhost vs IP address
4. Check network connectivity

### Issue: Wrong Format Exported

**Symptom**: Saved as JSON but expected INI

**Solution**:
1. Check file extension (.json vs .ini)
2. If needed, re-save with correct format
3. Check Save dialog format selection
4. Use browser download history to verify

---

## Examples

### Example 1: Database Configuration

**Schema**:
```json
{
  "properties": {
    "host": {"type": "string", "default": "localhost"},
    "port": {"type": "integer", "minimum": 1, "maximum": 65535},
    "database": {"type": "string"},
    "credentials": {
      "type": "object",
      "properties": {
        "username": {"type": "string"},
        "password": {"type": "string"}
      }
    }
  }
}
```

**Form Workflow**:
```
1. Load schema → Form generates
2. Fields shown: host, port, database, credentials section
3. Edit values
4. Save as YAML:
   database:
     host: myserver.com
     port: 5432
     database: myapp
     credentials:
       username: admin
       password: secret
```

### Example 2: Application Rules

**Schema with Arrays**:
```json
{
  "properties": {
    "rules": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "name": {"type": "string"},
          "level": {"enum": ["debug", "info", "warning", "error"]}
        }
      }
    }
  }
}
```

**Form Workflow**:
```
1. Load schema
2. Form shows: Rules [+ -]
3. Click + to add rule
4. Enter: name="request_log", level="info"
5. Click + again for second rule
6. Save as JSON with array
```

---

## Support

For issues and questions:
- 📖 Check [DESIGN.md](DESIGN.md) for architecture details
- 🔧 See [DEPLOYMENT.md](DEPLOYMENT.md) for setup issues
- 📚 Read [API_REFERENCE.md](API_REFERENCE.md) for technical details

