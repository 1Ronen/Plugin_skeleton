---
name: plugin-improve
description: >
  Add features, fix bugs, or refactor an existing plugin. Use when the user says
  "add a parameter", "fix this bug", "the knob doesn't respond", "add a second band",
  or any modification to an already-instantiated plugin.
allowed-tools:
  - Read
  - Write
  - Edit
  - Bash
---

# plugin-improve

Structured improvement workflow for existing plugins.

## Workflow

### Phase 1 — Classify the request

| Request type | Action |
|-------------|--------|
| Bug fix | Identify root cause, fix, rebuild |
| New parameter | Update parameter-spec, implement APVTS + UI |
| New DSP stage | Update architecture, implement component, wire in processBlock |
| UI change | Edit PluginEditor only, no DSP change |
| Refactor | Isolate scope, ensure no behaviour change |

### Phase 2 — Locate active plugin

```powershell
Select-String -Path CMakeLists.txt -Pattern "add_subdirectory\(plugins/(\w+)\)"
```

Confirm with user if ambiguous.

### Phase 3 — Read current state

Read in order:
1. `plugins/[Name]/.ideas/parameter-spec.md` (if exists)
2. `plugins/[Name]/.ideas/architecture.md` (if exists)
3. `plugins/[Name]/Source/PluginProcessor.h`
4. `plugins/[Name]/Source/PluginProcessor.cpp`
5. `plugins/[Name]/Source/PluginEditor.h` (if UI change)

### Phase 4 — Implement

**New parameter:**
1. Add to `createParameterLayout()` in PluginProcessor.cpp
2. Add read in `processBlock()` (atomic load)
3. Add `juce::Slider` / `juce::ToggleButton` member to PluginEditor.h
4. Add APVTS attachment member to PluginEditor.h
5. Initialize in PluginEditor constructor
6. Wire attachment to APVTS
7. Position in `resized()`
8. Update parameter-spec.md

**Bug fix:**
1. Reproduce the bug in code (identify the bad line/condition)
2. Fix minimally — do not refactor surrounding code
3. Add comment only if the fix is non-obvious

**New DSP stage:**
1. Create `Source/DSP/[Component].h` and `.cpp`
2. Add `target_sources` entry in plugin CMakeLists.txt
3. Declare member in PluginProcessor.h (before `apvts`)
4. Call `prepare()` in `prepareToPlay()`
5. Call `reset()` in `releaseResources()`
6. Call `process()` in `processBlock()` at correct chain position

### Phase 5 — Build and verify

Run `/build-compile` after changes. Fix any errors before reporting success.

### Phase 6 — Update contracts (if applicable)

If parameters or architecture changed, update:
- `plugins/[Name]/.ideas/parameter-spec.md`
- `plugins/[Name]/.ideas/architecture.md`

## Rules

- One concern per change. Don't fix bugs and add features in the same pass.
- Never change parameter IDs of existing parameters (breaks presets).
- Never remove parameters without a deprecation strategy.
- Real-time safety: `processBlock` reads only atomics, never allocates, never locks.
- The APVTS `apvts` member must stay `public` (PluginEditor accesses it directly).
