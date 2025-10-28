# INI File Loading - Debug Guide

## Issue Reported
User reports that the application does not allow loading `.ini` files through the browser form.

## Enhanced Debugging

Enhanced logging has been added to identify the exact validation issue:

### Debug Logs Added
```javascript
const mimeTypeValid = validTypes.includes(file.type);
const extensionValid = validExtensions.test(file.name);

console.log('[UPLOAD] MIME type check:', file.type, 'Valid:', mimeTypeValid);
console.log('[UPLOAD] Extension check:', file.name, 'Matches regex:', extensionValid);
console.log('[UPLOAD] Validation result:', mimeTypeValid || extensionValid);
```

### What to Look For in Browser Console

When you try to upload an INI file, you should see logs like:
```
[UPLOAD] File selected: myconfig.ini Type: text/plain
[UPLOAD] MIME type check: text/plain Valid: true
[UPLOAD] Extension check: myconfig.ini Matches regex: true
[UPLOAD] Validation result: true
[UPLOAD] File type valid, reading...
[UPLOAD] File content length: 256
[UPLOAD] Detected INI format, parsing...
```

## Validation Logic

### Current Validation
```javascript
const validTypes = ['application/json', 'text/yaml', 'text/plain', 'text/x-ini'];
const validExtensions = /\.(json|yaml|yml|ini)$/i;

// File is accepted if EITHER condition is true:
// 1. MIME type is in the valid list, OR
// 2. File extension matches the regex pattern

if (!mimeTypeValid && !extensionValid) {
    // Reject only if BOTH fail
    showToast('error', 'Invalid File', 'Please upload a JSON, YAML, or INI file');
    return;
}
```

### Accepted MIME Types
- `application/json` - JSON files
- `text/yaml` - YAML files
- `text/plain` - Text files (includes INI)
- `text/x-ini` - INI files

### Accepted Extensions
- `.json`
- `.yaml`
- `.yml`
- `.ini` (case-insensitive)

## Test Files

### Simple Test INI File
Location: `/tmp/test_config_simple.ini`

```ini
[appSettings]
appName=ConfigApp
appVersion=1.0
debugMode=true
maxRetries=5

[serverConfig]
hostname=localhost
portNumber=8080
enableSSL=false

[databaseConfig]
dbHost=db.local
dbPort=5432
dbName=configdb
maxConnections=10
```

### To Test Manually:

1. **Start Server**
   ```bash
   cd /home/thanhpv/Documents/ProjectDocument/SourceCode/testSpecKit/configGui
   ./build/src/html/ConfigGUIServer &
   ```

2. **Open Browser**
   - Go to: http://localhost:8080
   - Open Developer Console (F12 → Console tab)

3. **Load Schema**
   - Click "Load Schema"
   - Upload any JSON schema file (e.g., Parcels.schema.json)
   - Form should display with fields

4. **Try Loading INI Config**
   - Click "Load Configuration" button
   - **WATCH CONSOLE** - Look for validation logs
   - Select the INI file: `/tmp/test_config_simple.ini`
   - Check console for logs starting with `[UPLOAD]`

5. **Interpret Results**

   **If you see:**
   ```
   [UPLOAD] Validation result: true
   [UPLOAD] File type valid, reading...
   ```
   ✅ **File was accepted** - Continue to next step (INI parsing)

   **If you see:**
   ```
   [UPLOAD] Validation result: false
   ```
   ❌ **File was rejected** - Check MIME type and extension logs above

## Possible Issues and Solutions

### Issue 1: Browser Sending Wrong MIME Type
**Symptom**: `[UPLOAD] MIME type check: application/octet-stream Valid: false`

**Solution**: Extension should still work: `validExtensions.test(file.name)` should be `true` for `.ini` files

### Issue 2: Extension Regex Not Matching
**Symptom**: `[UPLOAD] Extension check: test.ini Matches regex: false`

**Problem**: Regex `/\.(json|yaml|yml|ini)$/i` should match `.ini` files

**Debug**: Check if filename has the exact format expected

### Issue 3: Both Checks Failing
**Symptom**:
```
[UPLOAD] MIME type check: application/octet-stream Valid: false
[UPLOAD] Extension check: test.ini Matches regex: false
```

**Possible Causes**:
- File doesn't have `.ini` extension (e.g., named `test.txt` instead of `test.ini`)
- MIME type not in list (but should still pass extension check)

## Console Inspection Steps

1. **Open Browser DevTools**: Press F12
2. **Go to Console Tab**
3. **Try uploading INI file**
4. **Look for logs**:
   - `[UPLOAD] File selected:` - Shows filename and detected MIME type
   - `[UPLOAD] MIME type check:` - Shows if MIME is recognized
   - `[UPLOAD] Extension check:` - Shows if extension pattern matches
   - `[UPLOAD] Validation result:` - Final decision (true = accept, false = reject)

## If File is Accepted But Still Has Issues

If validation passes but file doesn't populate form:

### Check for INI Parsing Errors
Look for logs:
```
[UPLOAD] Detected INI format, parsing...
[INI-PARSE] Starting INI to JSON conversion...
[INI-PARSE] Conversion complete, result: {...}
```

If you see errors about INI parsing, the issue is in `parseIniToJson()` function.

### Check for Form Population Issues
Look for logs:
```
[UPLOAD] File type detection: isSchema= false isConfig= true
[UPLOAD] Detected as CONFIG file, routing to config handler
```

## Server Status Check

```bash
# Check if server is running
ps aux | grep ConfigGUIServer

# Check server logs (if available)
tail -f /tmp/server.log

# Verify server responds
curl -s http://localhost:8080/api/config/list | jq .
```

## Code Locations

### File Upload Handler
- File: `src/html/assets/js/main.js`
- Lines: 3044-3130
- Function: `handleFileUpload()`

### Validation Logic
- Lines: 3054-3070 (Validation checks)
- Lines: 3073-3080 (Extended debugging)

### INI Parser
- Lines: 1030-1135 (parseIniToJson and setNestedValue)

## Next Steps

1. **Reproduce the issue** and collect console logs
2. **Check validation logs** to see which check is failing
3. **Share the exact error message** from console
4. **Provide filename** of the INI file being tested
5. **Check MIME type** being reported by browser

## Browser Compatibility Note

Different browsers may report different MIME types for INI files:
- Chrome: `text/plain` or `application/octet-stream`
- Firefox: `text/plain`
- Safari: `text/plain`
- Edge: `text/plain`

The regex extension check should catch all these cases for files ending in `.ini`.

---

**Server Running**: Yes (port 8080)  
**Test File Available**: `/tmp/test_config_simple.ini`  
**Debug Mode**: ENABLED (Enhanced logging active)

