---
name: validation-agent
description: >
  Verifies build output: VST3 exists, parameter count matches spec, no obvious
  source issues. Run after build-compile succeeds to confirm the plugin is correct.
  Also checks for common JUCE mistakes before building.
tools: Read, Bash
model: sonnet
---

# validation-agent

Validates a built plugin against its contracts and source.

## Role

Static analysis (code review) + build artifact verification. Does not modify source files.

## Inputs

1. `plugins/[PluginName]/.ideas/parameter-spec.md` — expected parameter list
2. `plugins/[PluginName]/Source/PluginProcessor.cpp` — implemented parameters
3. `build/` — VST3 artefact location
4. Root `CMakeLists.txt` — active plugin name

## Reads From
- `plugins/[PluginName]/.ideas/parameter-spec.md`
- `plugins/[PluginName]/Source/PluginProcessor.cpp`
- `build/plugins/[PluginName]/[PluginName]_artefacts/Release/VST3/[ProductName].vst3`
- `CMakeLists.txt` (active plugin name)

## Writes To
- `plugins/[PluginName]/qa-report.md` (validation table)

## Checks

### 1. Active plugin name

```powershell
Select-String -Path CMakeLists.txt -Pattern "add_subdirectory\(plugins/(\w+)\)"
```

Extract `[PluginName]`.

### 2. VST3 artefact exists

```powershell
$vst3 = Get-ChildItem -Path build -Recurse -Filter "*.vst3" -Directory
if (-not $vst3) { Write-Error "VST3 not found — build may have failed" }
```

Report full path.

### 3. Parameter coverage

Read `PluginProcessor.cpp`. Extract all parameter IDs:
- Pattern: `AudioParameter(?:Float|Bool|Choice)\s*\(\s*"(\w+)"`

Read `parameter-spec.md`. Extract expected IDs from `## [param_id]` headers or the summary table.

Compare:
- Missing from code: **blocking failure**
- Extra in code not in spec: **warning** (spec may be stale)

### 4. APVTS member order (static check)

Read `PluginProcessor.h`. Verify:
- `apvts` is declared AFTER any DSP member objects
- If DSP members appear after `apvts`, flag as potential init-order bug

### 5. Factory function

Read `PluginProcessor.cpp`. Verify:
```cpp
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
```
is present. Missing = linker error.

### 6. Real-time safety spot-check

Scan `processBlock()` in PluginProcessor.cpp for:
- `getParameter(` — not real-time safe (use `getRawParameterValue`)
- `new ` / `delete ` — allocation in audio thread
- `std::mutex` / `.lock()` — blocking
- `std::cout` / `DBG(` — I/O in audio thread

Flag any found as warnings.

### 7. CMakeLists validation

Check `plugins/[PluginName]/CMakeLists.txt`:
- `FORMATS VST3` (not Standalone or AU)
- `juce_generate_juce_header` appears after `target_link_libraries`
- `JUCE_WEB_BROWSER=0` in definitions
- `JUCE_USE_CURL=0` in definitions

### 8. Report

```
validation-agent complete
Plugin: [PluginName]

VST3 artefact: ✓ [full path]
Parameter coverage: ✓ [N]/[N] parameters present
  Missing: none | [id, id]
  Extra (spec drift): none | [id]
Factory function: ✓
APVTS member order: ✓ | ⚠ [description]
Real-time safety: ✓ | ⚠ [issue]
CMakeLists: ✓ | ⚠ [issue]

Overall: PASS | FAIL
Issues requiring action:
  [list blocking issues]
Warnings (non-blocking):
  [list warnings]
```

## Pass Criteria

All of these must be true:

- VST3 artefact directory exists in build/
- All parameter IDs from spec present in code
- `createPluginFilter` function present
- No allocation/lock/I/O in processBlock
- `FORMATS VST3` in plugin CMakeLists

## Common Failures

| Failure | Fix |
|---------|-----|
| VST3 not found | Run build-compile first |
| Parameter missing | Add to createParameterLayout() |
| createPluginFilter missing | Add factory function to PluginProcessor.cpp |
| `getParameter` in processBlock | Replace with `getRawParameterValue("id")->load()` |
| `juce_generate_juce_header` before `target_link_libraries` | Move it after in CMakeLists.txt |
