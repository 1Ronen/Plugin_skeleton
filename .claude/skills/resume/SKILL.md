---
name: resume
description: >
  Reads the Last Completed Gate from CLAUDE.md and automatically activates the
  correct next agent in the pipeline. Invoke with "Resume" or "/resume".
allowed-tools:
  - Read
  - Bash
---

# resume

Resumes the plugin build pipeline at the correct next agent, determined by
the last completed gate recorded in CLAUDE.md.

## Reads From

- `.claude/CLAUDE.md` - Active Plugin name, Path, Last completed gate

## Gate-to-Agent Map

| Last Completed Gate | Next Agent to Activate |
|---------------------|------------------------|
| none | plugin-planner |
| Init | plugin-planner |
| Stage1 | dsp-engineer |
| Stage2 | ui-engineer |
| Stage3 | qa-tester |
| QA | refactor-agent |
| Refactor | installer-agent |
| Installer | [complete] - report and run backup |

## Workflow

### 1. Read CLAUDE.md

Extract:
- `Name` - active plugin identifier
- `Path` - plugin root path
- `LastGate` - last completed gate value

If no active plugin is set, stop:
```
No active plugin found in CLAUDE.md. Run new-plugin first.
```

### 2. Resolve next agent

Look up `LastGate` in the Gate-to-Agent Map above.

### 3. Pre-flight check

Before activating the next agent, verify its required input files exist:

| Next Agent | Required inputs |
|------------|----------------|
| plugin-planner | Plugin directory exists at Path |
| dsp-engineer | `Source/PluginProcessor.cpp`, `.ideas/architecture.md`, VST3 binary in `build/` |
| ui-engineer | `Source/PluginProcessor.h`, `.ideas/parameter-spec.md`, VST3 binary in `build/` |
| qa-tester | VST3 binary in `build/` |
| refactor-agent | `qa-report.md` with Status: PASS |
| installer-agent | VST3 binary confirmed, `qa-report.md` Status: PASS |

If any required input is missing, report exactly what is missing and stop:
```
resume: cannot activate [agent] — missing inputs:
  - [file path]
Pre-flight failed. Resolve missing files before resuming.
```

### 4. Activate next agent

Print the handoff line, then activate the agent:

```
resume: Last gate = [LastGate]
resume: Activating [NextAgent] for plugin [Name]
---
```

Then invoke the agent as described in its agent.md.

### 5. Special case: Installer complete

If `LastGate = Installer`:

```
Plugin [Name] is SHIPPED.

Installer: plugins/[Name]/installer/[Name]_Setup.exe

Post-ship checklist:
  1. Test installer on a clean machine
  2. Add lessons learned to juce8-critical-patterns.md
  3. Run /backup to push to git
  4. Review skeleton for improvements (propose, do not apply without confirmation)
```

Do not activate any agent. Prompt user to run `/backup`.

## Constraints

- Never skips a stage — if pre-flight check fails, stops and reports
- Never modifies CLAUDE.md — only reads it
- If the pipeline is already at `complete`, reports shipped status and stops
