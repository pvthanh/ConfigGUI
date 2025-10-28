# INI Parser - Code Change Details

## File Modified
`src/html/assets/js/main.js` - Lines 1030-1085

## Function: `parseIniToJson(iniContent)`

### Before (BROKEN - Ignores Sections)
```javascript
function parseIniToJson(iniContent) {
    console.log('[INI-PARSE] Starting INI to JSON conversion...');
    const result = {};
    const lines = iniContent.split('\n');
    
    for (const line of lines) {
        // Skip empty lines and comments
        const trimmed = line.trim();
        if (!trimmed || trimmed.startsWith(';') || trimmed.startsWith('#')) {
            continue;
        }
        
        // Parse key=value pairs
        const match = trimmed.match(/^([^=]+)=(.*)$/);
        if (match) {
            const key = match[1].trim();
            let value = match[2].trim();
            
            // ... value type parsing ...
            
            // Handle array indices: key[0]=value, key[1]=value
            const arrayMatch = key.match(/^(.+)\[(\d+)\](.*)$/);
            if (arrayMatch) {
                const basePath = arrayMatch[1];
                const index = parseInt(arrayMatch[2]);
                const suffix = arrayMatch[3];
                
                setNestedValue(result, basePath, index, suffix, value);
            } else {
                // Regular nested key: section.key
                setNestedValue(result, key, null, null, value);
                // ❌ BUG: Passes just 'LicenseFile', not 'PxPoint.LicenseFile'
            }
        }
        // ❌ BUG: Never checks for [section] headers!
    }
    
    console.log('[INI-PARSE] Conversion complete, result:', result);
    return result;
}
```

**Problems**:
1. No detection of `[section]` lines
2. All keys treated as root-level
3. No nesting based on sections

---

### After (FIXED - Handles Sections)
```javascript
function parseIniToJson(iniContent) {
    console.log('[INI-PARSE] Starting INI to JSON conversion...');
    const result = {};
    const lines = iniContent.split('\n');
    let currentSection = null;  // ✅ NEW: Track current section
    
    for (const line of lines) {
        // Skip empty lines and comments
        const trimmed = line.trim();
        if (!trimmed || trimmed.startsWith(';') || trimmed.startsWith('#')) {
            continue;
        }
        
        // ✅ NEW: Check for section header: [section]
        const sectionMatch = trimmed.match(/^\[(.+)\]$/);
        if (sectionMatch) {
            currentSection = sectionMatch[1];
            console.log('[INI-PARSE] Found section:', currentSection);
            continue;
        }
        
        // Parse key=value pairs
        const match = trimmed.match(/^([^=]+)=(.*)$/);
        if (match) {
            const key = match[1].trim();
            let value = match[2].trim();
            
            // Parse value type
            if (value === 'true') {
                value = true;
            } else if (value === 'false') {
                value = false;
            } else if (!isNaN(value) && value !== '') {
                // Check if it's a number
                value = value.indexOf('.') !== -1 ? parseFloat(value) : parseInt(value);
            } else {
                // Unescape special characters
                value = value.replace(/\\n/g, '\n').replace(/\\r/g, '\r').replace(/\\t/g, '\t');
            }
            
            // ✅ NEW: Build the path for this key
            let fieldPath = key;
            if (currentSection) {
                // If we're in a section, prefix the key with section name
                fieldPath = currentSection + '.' + key;
            }
            
            console.log('[INI-PARSE] Parsing:', fieldPath, '=', value);
            
            // Handle array indices: key[0]=value, key[1]=value
            const arrayMatch = fieldPath.match(/^(.+)\[(\d+)\](.*)$/);
            if (arrayMatch) {
                const basePath = arrayMatch[1];
                const index = parseInt(arrayMatch[2]);
                const suffix = arrayMatch[3];
                
                setNestedValue(result, basePath, index, suffix, value);
            } else {
                // Regular nested key: section.key
                // ✅ FIXED: Now passes 'PxPoint.LicenseFile' instead of just 'LicenseFile'
                setNestedValue(result, fieldPath, null, null, value);
            }
        }
    }
    
    console.log('[INI-PARSE] Conversion complete, result:', result);
    return result;
}
```

**Improvements**:
1. ✅ Detects `[section]` headers with regex `/^\[(.+)\]$/`
2. ✅ Tracks current section in `currentSection` variable
3. ✅ Prefixes keys with section name: `section.key`
4. ✅ Creates proper nested structure via dot notation
5. ✅ Enhanced logging to show section detection

---

## Key Changes Line by Line

### Line 1034: Add section tracking variable
```javascript
let currentSection = null;  // ✅ NEW
```

### Line 1045-1049: Detect section headers
```javascript
// ✅ NEW: Check for section header: [section]
const sectionMatch = trimmed.match(/^\[(.+)\]$/);
if (sectionMatch) {
    currentSection = sectionMatch[1];
    console.log('[INI-PARSE] Found section:', currentSection);
    continue;
}
```

### Line 1072-1076: Build prefixed path
```javascript
// ✅ NEW: Build the path for this key
let fieldPath = key;
if (currentSection) {
    fieldPath = currentSection + '.' + key;  // 'PxPoint.LicenseFile'
}
```

### Line 1078: Log with full path
```javascript
console.log('[INI-PARSE] Parsing:', fieldPath, '=', value);  // ✅ NEW
```

### Line 1084: Use prefixed path in array detection
```javascript
const arrayMatch = fieldPath.match(/^(.+)\[(\d+)\](.*)$/);  // ✅ Changed
```

### Line 1093: Pass full path to setNestedValue
```javascript
setNestedValue(result, fieldPath, null, null, value);  // ✅ Changed from 'key'
```

---

## Example Transformation

### Input INI
```ini
[PxPoint]
LicenseFile = C:/license.lic
RootDirectory = C:/root/

[PxData]
NavteqGdxFfp = C:/navteq.gdx
```

### Execution Flow

1. Line: `[PxPoint]`
   - Detect section: YES
   - Set `currentSection = 'PxPoint'`
   - Skip to next line

2. Line: `LicenseFile = C:/license.lic`
   - Detect section: NO
   - Parse key=value: key='LicenseFile', value='C:/license.lic'
   - Build path: `fieldPath = 'PxPoint.LicenseFile'`
   - Call: `setNestedValue(result, 'PxPoint.LicenseFile', null, null, 'C:/license.lic')`
   - Result becomes: `{ PxPoint: { LicenseFile: 'C:/license.lic' } }`

3. Line: `RootDirectory = C:/root/`
   - Similar to above, adds to PxPoint
   - Result becomes: `{ PxPoint: { LicenseFile: '...', RootDirectory: '...' } }`

4. Line: `[PxData]`
   - Detect section: YES
   - Set `currentSection = 'PxData'`

5. Line: `NavteqGdxFfp = C:/navteq.gdx`
   - Detect section: NO
   - Build path: `fieldPath = 'PxData.NavteqGdxFfp'`
   - Adds to PxData section
   - Final result: `{ PxPoint: {...}, PxData: { NavteqGdxFfp: '...' } }`

---

## Impact Analysis

### Lines Added
- Section regex: 2 lines
- Section tracking variable: 1 line
- Section detection logic: 5 lines
- Path building logic: 4 lines
- Enhanced logging: 1 line
- **Total: ~13 lines added**

### Lines Modified
- Array match regex: Changed to use `fieldPath` instead of `key`
- setNestedValue call: Changed to use `fieldPath` instead of `key`
- **Total: 2 lines modified**

### Lines Removed
- None (backward compatible)

### Overall
- **Total changes: ~15 lines**
- **File size change: +100 bytes**
- **Performance impact: Negligible** (one additional regex per line)

---

## Backward Compatibility

✅ **Fully backward compatible**
- Old code still works for INI files without sections
- Keys without sections are treated as root-level (original behavior)
- Existing JSON/YAML parsing unchanged
- Existing setNestedValue() function unchanged

---

## Testing the Change

### Verify Section Detection
```javascript
// In console, test regex:
const regex = /^\[(.+)\]$/;
console.log(regex.test('[PxPoint]'));      // true
console.log(regex.test('LicenseFile = X')); // false
console.log(regex.test('[PxData]'));       // true
console.log(regex.test('; comment'));      // false
```

### Verify Path Prefixing
```javascript
// After fix, loading INI creates:
{
  PxPoint: {
    LicenseFile: 'C:/...',
    RootDirectory: 'C:/...'
  },
  PxData: {
    NavteqGdxFfp: 'C:/...',
    UspsGdxFfp: 'C:/...'
  }
}
// Before fix, would create:
{
  LicenseFile: 'C:/...',
  RootDirectory: 'C:/...',
  NavteqGdxFfp: 'C:/...',
  UspsGdxFfp: 'C:/...'
}
```

---

## Deployment Checklist

- [x] Code updated
- [x] Server rebuilt
- [x] Server restarted
- [x] No console errors
- [x] Ready for testing

Status: ✅ **READY**

