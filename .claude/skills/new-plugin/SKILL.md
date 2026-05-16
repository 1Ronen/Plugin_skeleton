---
name: new-plugin
description: >
  Entry point for every new JUCE VST3 plugin. Copies the skeleton to
  D:\Dev\Plugins\[PluginName]\, instantiates the template, creates .ideas/
  contracts, and updates CLAUDE.md. Invoke with:
  "Create new plugin called [Name], type [synth/fx]"
allowed-tools:
  - Read
  - Write
  - Edit
  - Bash
---

# new-plugin

Entry point for every new JUCE VST3 plugin project.

## Inputs Required

Collected from the invocation phrase: `Create new plugin called [Name], type [synth/fx]`

| Field | Source | Example |
|-------|--------|---------|
| `PluginName` | Name from invocation | `SimpleDelay` |
| `PluginCode` | First 4 chars of PluginName, uppercase | `SIMD` |
| `IsSynth` | `synth` -> `TRUE`, `fx` -> `FALSE` | `FALSE` |

## Workflow

### 1. Derive and validate values

From the invocation extract:
- `PluginName` -- strip spaces, ensure CamelCase, valid C++ identifier
- `PluginCode` -- first 4 chars of PluginName, uppercase (pad with `X` if name shorter than 4)
- `IsSynth` -- `TRUE` if type is `synth`, `FALSE` if type is `fx`
- `NeedsMidi` -- same as `IsSynth`
- `Category` -- `"Instrument|Synth"` if synth, `"Fx"` if fx

Validation:
- `PluginName` must match `^[A-Z][A-Za-z0-9]+$` -- no spaces, no underscores, starts uppercase
- `D:\Dev\Plugins\[PluginName]\` must not already exist -- stop and report if it does

### 2. Copy skeleton

Copy `D:\Dev\PluginSkeleton\` to `D:\Dev\Plugins\[PluginName]\`, excluding `.git\`, `build\`, and `plugins\`.

```powershell
$src = "D:\Dev\PluginSkeleton"
$dst = "D:\Dev\Plugins\$PluginName"

New-Item -ItemType Directory -Path "D:\Dev\Plugins" -Force | Out-Null
robocopy $src $dst /E /XD ".git" "build" "plugins" /NFL /NDL /NJH /NJS | Out-Null
```

After copy, `$dst` contains: `CMakeLists.txt`, `template\`, `.claude\`, `scripts\`,
`juce8-critical-patterns.md`, `troubleshooting\`, `CLAUDE.md`, `.gitignore`

### 3. Instantiate plugin source from template

Create the plugin source directory and copy all five template files with placeholders replaced.

```powershell
$pluginDir = "$dst\plugins\$PluginName"
New-Item -ItemType Directory -Path "$pluginDir\Source" -Force | Out-Null
```

Copy each file and apply replacements:

| Source | Destination |
|--------|-------------|
| `D:\Dev\PluginSkeleton\template\CMakeLists.txt` | `$pluginDir\CMakeLists.txt` |
| `D:\Dev\PluginSkeleton\template\Source\PluginProcessor.h` | `$pluginDir\Source\PluginProcessor.h` |
| `D:\Dev\PluginSkeleton\template\Source\PluginProcessor.cpp` | `$pluginDir\Source\PluginProcessor.cpp` |
| `D:\Dev\PluginSkeleton\template\Source\PluginEditor.h` | `$pluginDir\Source\PluginEditor.h` |
| `D:\Dev\PluginSkeleton\template\Source\PluginEditor.cpp` | `$pluginDir\Source\PluginEditor.cpp` |

Apply these replacements in every copied file (case-sensitive):

| Placeholder | Replacement |
|-------------|-------------|
| `[PluginName]` | actual PluginName (e.g. `SimpleDelay`) |
| `[PluginCode]` | 4-char code (e.g. `SIMD`) |
| `[ProductName]` | same as PluginName |
| `[Category]` | `"Instrument|Synth"` or `"Fx"` |
| `IS_SYNTH                 FALSE` | `IS_SYNTH                 TRUE` or `FALSE` |
| `NEEDS_MIDI_INPUT         FALSE` | `NEEDS_MIDI_INPUT         TRUE` or `FALSE` |

### 4. Update root CMakeLists.txt

In `$dst\CMakeLists.txt`, replace the active plugin subdirectory line:

```cmake
# Before:
add_subdirectory(plugins/SimpleGain)

# After:
add_subdirectory(plugins/[PluginName])
```

Use Edit to make this change precisely -- do not rewrite the whole file.

### 5. Create .ideas/ contracts

Create `$dst\.ideas\` and write four empty contract files:

**creative-brief.md**
```
# Creative Brief

## Plugin Name

## Type
<!-- Effect | Instrument -->

## One-Line Description

## Target User

## Core Problem Solved

## Inspiration / Reference Plugins

## Must-Have Features

## Out of Scope
```

**parameter-spec.md**
```
# Parameter Spec

| ID | Type | Min | Max | Default | Unit | DSP Role |
|----|------|-----|-----|---------|------|----------|

## Parameter Details

<!--
## [param_id]
- Type: Float | Bool | Choice
- Range: min - max (step)
- Default: value
- Taper: linear | logarithmic | exponential
- DSP role:
- UI: Knob | Toggle | Dropdown
-->
```

**plan.md**
```
# Plan

## Complexity Score
<!-- 1-5 -->

## DSP Stages

## Parameter Count
<!-- Total: N -->

## Risk Register

| Risk | Likelihood | Mitigation |
|------|-----------|------------|

## Open Questions
```

**architecture.md**
```
# Architecture

## Signal Chain

[Input] -> [Stage 1] -> [Output]

## DSP Components

<!--
### ComponentName
- Responsibility:
- JUCE class (if applicable):
- Parameters consumed:
- Latency introduced:
-->

## Parameter -> DSP Mapping

| Parameter ID | Component | Setter / Effect |
|-------------|-----------|-----------------|

## Member Declaration Order in PluginProcessor.h
<!-- List DSP members in init order -- all must appear BEFORE apvts -->
```

### 6. Update .claude/CLAUDE.md Active Plugin section

In `$dst\.claude\CLAUDE.md`, replace the Active Plugin table values:

```markdown
## Active Plugin

| Field | Value |
|---|---|
| Name | [PluginName] |
| Path | `plugins/[PluginName]/` |
| Current stage | planning |
| Last completed | none |
```

### 7. Print directory tree

```powershell
Get-ChildItem -Path $dst -Recurse |
    Where-Object { -not $_.PSIsContainer } |
    Select-Object -ExpandProperty FullName |
    ForEach-Object { $_.Replace($dst, "").TrimStart("\") } |
    Sort-Object
```

### 8. Confirm

```
new-plugin complete
Plugin:  [PluginName]
Type:    [Synth | FX]
Code:    [PluginCode]
Path:    D:\Dev\Plugins\[PluginName]\

Files created:
  plugins/[PluginName]/CMakeLists.txt
  plugins/[PluginName]/Source/PluginProcessor.h
  plugins/[PluginName]/Source/PluginProcessor.cpp
  plugins/[PluginName]/Source/PluginEditor.h
  plugins/[PluginName]/Source/PluginEditor.cpp
  .ideas/creative-brief.md
  .ideas/parameter-spec.md
  .ideas/plan.md
  .ideas/architecture.md

Root CMakeLists.txt -> add_subdirectory(plugins/[PluginName])
.claude/CLAUDE.md   -> Active Plugin: [PluginName], stage: planning

Next steps:
  1. Fill in .ideas/creative-brief.md and .ideas/parameter-spec.md
  2. Run /plugin-ideation to refine contracts
  3. Run foundation-agent to implement APVTS from parameter-spec
  4. Run /build-compile to verify
```

## Constraints

- Never modify files under `D:\Dev\PluginSkeleton\` -- all writes go to `D:\Dev\Plugins\[PluginName]\`
- Never add parameters to PluginProcessor -- leave processBlock as pass-through until dsp-engineer runs
- Never overwrite an existing plugin directory -- stop and report if destination already exists
- PluginCode must be exactly 4 uppercase characters -- pad with `X` if PluginName is shorter than 4 chars
