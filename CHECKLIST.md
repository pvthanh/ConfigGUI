# Checklist: AutoRules Save Issue - Investigation Complete

## Investigation Phase ✅

### Code Analysis
- [x] Identified exact issue: rdf/shapefiles/3d-landmarks saving as numbers
- [x] Traced data flow for save operation (getFormData → collectDataRecursive)
- [x] Traced data flow for load operation (setFormData → applyDataRecursive)
- [x] Examined widget registration and creation
- [x] Reviewed ArrayWidget implementation
- [x] Checked schema structure
- [x] Identified field registration patterns
- [x] Found that count values match array sizes

### Debug Instrumentation
- [x] Added logging to collectDataRecursive()
- [x] Added logging to getFormData()
- [x] Added logging to addFieldToFormWithMetadata()
- [x] Added logging to field_widgets_ access
- [x] Build successful with all logging
- [x] No compiler errors

### Documentation
- [x] DEBUGGING_AUTORUN_SAVE.md - Testing guide with expected output
- [x] AUTORUN_ANALYSIS.md - 5 hypotheses with evidence patterns
- [x] INVESTIGATION_SUMMARY.md - High-level overview
- [x] QUICK_DEBUG.md - Quick reference one-pager
- [x] SESSION_SUMMARY.md - This session's work

### Hypotheses
- [x] Hypothesis 1: Fields not registered in field_widgets_
- [x] Hypothesis 2: Wrong widget type stored
- [x] Hypothesis 3: ArrayWidget.getValues() returns number
- [x] Hypothesis 4: field_widgets_ map cleared
- [x] Hypothesis 5: JSON serialization bug
- [x] Each with evidence patterns and root causes

## Testing Phase ⏳

### Prerequisites
- [ ] User has access to built binary at `build/ConfigGUI`
- [ ] User can run GUI application
- [ ] User can capture stderr/stdout to log file

### Test Execution
- [ ] Run: `./build/ConfigGUI 2>&1 | tee debug.log &`
- [ ] File → Open Schema (select config.schema.json)
- [ ] File → Open Configuration (select config.json)
- [ ] File → Save Configuration (save as config_debug.json)
- [ ] Exit application
- [ ] Capture is complete

### Log Analysis
- [ ] Extract relevant lines using grep
- [ ] Identify which hypothesis matches output
- [ ] Note exact field names showing in errors
- [ ] Copy relevant sections of debug output

## Fix Implementation Phase ⏳

### Based on Test Results
- [ ] Identify which hypothesis was correct
- [ ] Locate exact code causing issue
- [ ] Implement targeted fix
- [ ] Rebuild application
- [ ] Verify fix works
- [ ] Document fix in code

### Expected Fixes by Hypothesis
1. **Fields Not Registered** → Fix field registration in addFieldToFormWithMetadata()
2. **Wrong Widget Type** → Fix widget factory selection logic
3. **ArrayWidget.getValues() Bug** → Fix getValues() to return array not count
4. **Map Cleared** → Ensure field_widgets_ not cleared prematurely
5. **JSON Serialization** → Check data after collection

## Verification Phase ⏳

### After Fix
- [ ] Rebuild with fix applied
- [ ] Run same test procedure
- [ ] Verify no more "NOT FOUND" messages for rdf/shapefiles/3d-landmarks
- [ ] Verify ArrayWidget appears as correct widget type
- [ ] Verify getValues() returns array not number
- [ ] Check saved config has correct arrays in autoRules
- [ ] Load saved config and verify values appear correctly in GUI

### Regression Testing
- [ ] Load original config.json (still works?)
- [ ] Load default values (still works?)
- [ ] Save new config (correctly?)
- [ ] Verify other sections (configureRules, etc.) still work

## Documentation Update ⏳

### After Fix
- [ ] Document root cause in code comments
- [ ] Update README if needed
- [ ] Create bug fix summary document
- [ ] Update any affected documentation

## Commit History So Far

```
99b0620 Add comprehensive session summary
c7b4190 Add quick debug reference guide
850eee3 Add investigation summary
490035c Add comprehensive analysis of autoRules save issue
e62a381 Add comprehensive debug logging for autoRules save issue
```

## Files to Monitor During Testing
- Debug output: `debug.log` (generated during test)
- Saved config: `config_debug.json` (check its contents)
- Source files: 
  - `src/ui/form_generator.cpp` (main collection logic)
  - `src/ui/array_widget.cpp` (widget values)
  - `src/ui/widget_factory.cpp` (widget creation)

## Success Criteria
✅ Investigation: All code analysis and instrumentation complete
⏳ Testing: Debug output shows which hypothesis is correct
⏳ Fix: Arrays save and load correctly as strings not numbers
⏳ Verify: config_debug.json has proper array format
⏳ Complete: All tests pass, no regressions

## Notes for User

The investigation phase is **100% complete**. All code has been analyzed, instrumented with comprehensive debug logging, and documented. The application builds successfully.

Next step: **Run the debug test** following instructions in QUICK_DEBUG.md or DEBUGGING_AUTORUN_SAVE.md. The debug output will pinpoint exactly which hypothesis is correct and where the bug is located.

Once you provide the debug log output, the fix can be implemented quickly (15-30 minutes).

---

**Status**: Investigation Complete, Ready for Testing
**Time Estimate**: ~50 minutes remaining (test + fix + verify)
**Blockers**: None - ready to proceed
