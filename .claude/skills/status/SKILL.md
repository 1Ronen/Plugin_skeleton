---
name: status
description: >
  Reports the active plugin name, current stage, last completed gate, and next
  required agent in a single line. Also lists any prerequisite files missing for
  the current stage. Invoke with "Run status" or "/status".
allowed-tools:
  - Read
  - Bash
---

# status

Single-line snapshot of where a plugin build currently stands.

## Reads From

- `.claude/CLAUDE.md` - Active Plugin section (Name, Path, Current stage, Last completed)
- `plugins/[ActivePlugin]/qa-report.md` - if it exists, shows pass/fail summary

## Workflow

### 1. Read CLAUDE.md

Extract from the Active Plugin table:
- `Name` - plugin identifier
- `Path` - relative path to plugin root (e.g. `plugins/SimpleDelay/`)
- `CurrentStage` - planning | scaffold | dsp | gui | qa | refactor | installer | complete
- `LastGate` - none | Init | Stage1 | Stage2 | Stage3 | QA | Refactor | Installer

### 2. Determine next agent

| Last Completed Gate | Next Agent |
|---------------------|-----------|
| none | plugin-planner |
| Init | plugin-planner |
| Stage1 | dsp-engineer |
| Stage2 | ui-engineer |
| Stage3 | qa-tester |
| QA | refactor-agent |
| Refactor | installer-agent |
| Installer | [shipped] |

### 3. Check gate files for current stage

For the current stage, check whether all prerequisite files exist.

| Stage | Required files |
|-------|---------------|
| planning | `.ideas/creative-brief.md`, `.ideas/parameter-spec.md`, `.ideas/plan.md` |
| architecture | `.ideas/architecture.md` |
| scaffold | `Source/PluginProcessor.h`, `Source/PluginProcessor.cpp`, `Source/PluginEditor.h`, `Source/PluginEditor.cpp`, `CMakeLists.txt` |
| dsp | VST3 binary in `build/` |
| gui | VST3 binary in `build/` |
| qa | `qa-report.md` |
| refactor | `qa-report.md` (Status: PASS) |
| installer | `installer/[Name]_Setup.exe`, `installer/install-log.md` |

Paths are relative to `Path` extracted from CLAUDE.md.

```powershell
$pluginPath = # value from CLAUDE.md Path field
# check each file for current stage
$missing = @()
foreach ($f in $requiredFiles) {
    if (-not (Test-Path "$pluginPath\$f")) { $missing += $f }
}
```

### 4. Read qa-report.md (if exists and stage is qa or later)

If `qa-report.md` exists, extract the overall status line (PASS / FAIL) and append to output.

### 5. Output

**Primary line** (always printed):
```
Plugin: [Name] | Stage: [CurrentStage] | Last Gate: [LastGate] | Next Agent: [NextAgent]
```

**Missing files** (printed only if any missing):
```
Missing for current stage:
  - [relative path]
  - [relative path]
```

**QA summary** (printed only if qa-report.md exists):
```
QA: PASS | FAIL
```

**Example output (clean build):**
```
Plugin: SimpleDelay | Stage: dsp | Last Gate: Stage1 | Next Agent: dsp-engineer
```

**Example output (missing files):**
```
Plugin: SimpleDelay | Stage: planning | Last Gate: none | Next Agent: plugin-planner
Missing for current stage:
  - .ideas/parameter-spec.md
  - .ideas/plan.md
```

## Constraints

- Read-only: never modifies any file
- If CLAUDE.md has no active plugin set, output: `No active plugin. Run new-plugin first.`
- Paths are resolved relative to the current working directory
