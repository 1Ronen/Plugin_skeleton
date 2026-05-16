# Plugin Skeleton — Developer Manual
**Version 1.0 | Stack: JUCE 8 / MSVC / CMake / VST3 / Windows**

---

## 1. System Overview

The Plugin Skeleton is a reusable, agent-driven development framework for building professional JUCE VST3 plugins on Windows. Every plugin produced by this system follows an identical pipeline — only the DSP and UI contents change. The skeleton lives at `D:\Dev\PluginSkeleton\` and is never modified during a plugin build. Each new plugin is instantiated from it.

### Core Principle
**Files are the handshake between agents.** No agent communicates with another directly. All coordination happens through the filesystem — specifically through `.ideas/` contract files and compiled build artifacts. Every agent has a defined input set it reads and an output set it writes. Nothing else.

---

## 2. Directory Structure

```
D:\Dev\PluginSkeleton\
├── .claude\
│   ├── CLAUDE.md                    ← Master context — loaded every session
│   ├── agents\
│   │   ├── plugin-planner\agent.md
│   │   ├── dsp-architect\agent.md
│   │   ├── foundation-agent\agent.md
│   │   ├── dsp-engineer\agent.md
│   │   ├── ui-engineer\agent.md
│   │   ├── qa-tester\agent.md
│   │   ├── debug-agent\agent.md
│   │   ├── refactor-agent\agent.md
│   │   └── knowledge-agent\agent.md
│   ├── skills\
│   │   └── plugin-workflow\         ← Master orchestrator skill
│   └── juce8-critical-patterns.md  ← Injected into all implementation agents
├── plugins\
│   └── [PluginName]\                ← Created per plugin, never in skeleton root
│       ├── .ideas\
│       │   ├── creative-brief.md
│       │   ├── parameter-spec.md
│       │   ├── architecture.md
│       │   └── plan.md
│       ├── Source\
│       │   ├── PluginProcessor.h
│       │   ├── PluginProcessor.cpp
│       │   ├── PluginEditor.h
│       │   ├── PluginEditor.cpp
│       │   ├── DSP\
│       │   └── UI\
│       ├── build\
│       ├── CMakeLists.txt
│       └── qa-report.md
└── troubleshooting\
    ├── index.md                     ← KB index, updated after every resolved bug
    └── [YYYY-MM-DD]-[slug].md       ← One file per resolved incident
```

---

## 3. Agent Roster

### 3.1 Pipeline Agents (Sequential)

#### `plugin-planner`
**Role:** Translates a plugin concept into complete `.ideas/` contract files. No code.
**Reads:** User input (concept, name, sonic goal)
**Writes:** `.ideas/creative-brief.md`, `.ideas/parameter-spec.md`, `.ideas/plan.md`
**Gateway IN:** User must provide plugin name, type (synth/FX), and sonic goal
**Gateway OUT:** All three `.ideas/` files exist and are complete — no empty fields

#### `dsp-architect`
**Role:** Designs signal flow and selects algorithms. No implementation.
**Reads:** `.ideas/creative-brief.md`, `.ideas/parameter-spec.md`
**Writes:** `.ideas/architecture.md`
**Gateway IN:** `plugin-planner` gateway OUT passed
**Gateway OUT:** `architecture.md` contains signal chain (numbered), algorithm-per-stage table, risk register

#### `foundation-agent`
**Role:** Scaffolds CMakeLists.txt, project structure, and parameter skeleton.
**Reads:** All `.ideas/` files, `D:\Dev\PluginSkeleton\` template, `juce8-critical-patterns.md`
**Writes:** `CMakeLists.txt`, `PluginProcessor.h/.cpp`, `PluginEditor.h/.cpp` (empty stubs)
**Gateway IN:** `dsp-architect` gateway OUT passed
**Gateway OUT:** `cmake -B build -G "Visual Studio 17 2022" -A x64` exits with code 0

#### `dsp-engineer`
**Role:** Implements all audio processing logic.
**Reads:** `.ideas/architecture.md`, `.ideas/parameter-spec.md`, `PluginProcessor.h`, `juce8-critical-patterns.md`, `troubleshooting\index.md`
**Writes:** `Source\DSP\*.h/.cpp`, updates `PluginProcessor.cpp`
**Gateway IN:** `foundation-agent` gateway OUT passed (CMake configure green)
**Gateway OUT:** `cmake --build build --config Release` exits with code 0, VST3 binary exists

#### `ui-engineer`
**Role:** Implements JUCE native GUI.
**Reads:** `.ideas/parameter-spec.md`, `.ideas/creative-brief.md` (UX section), `PluginProcessor.h`, `juce8-critical-patterns.md`
**Writes:** `Source\UI\*.h/.cpp`, updates `PluginEditor.cpp`
**Gateway IN:** `dsp-engineer` gateway OUT passed (VST3 binary exists)
**Gateway OUT:** Plugin compiles with GUI, all parameters have SliderAttachment, editor renders at correct size

#### `qa-tester`
**Role:** Validates compiled plugin against technical and functional requirements.
**Reads:** Compiled VST3 binary, `.ideas/parameter-spec.md`
**Writes:** `qa-report.md`
**Gateway IN:** `ui-engineer` gateway OUT passed
**Gateway OUT:** pluginval exits with 0 errors, all parameters verified, qa-report.md complete

### 3.2 Reactive Agents (Non-Sequential)

#### `debug-agent`
**Role:** Diagnoses and resolves build failures, runtime crashes, DSP artifacts.
**Fires:** Automatically when any pipeline agent reports an error
**Reads:** Error output, `troubleshooting\index.md` (KB check first)
**Writes:** Fixed source files, re-runs failed step
**Constraint:** Fixes only the reported error. If fix requires modifying `.ideas/` contracts, stops and flags to user — does not modify contracts unilaterally.
**Hands back to:** Originating agent after fix confirmed

#### `knowledge-agent`
**Role:** Captures every resolved error into the persistent troubleshooting KB.
**Fires:** After every successful `debug-agent` resolution
**Reads:** debug-agent session output (error + root cause + fix)
**Writes:** `troubleshooting\[YYYY-MM-DD]-[slug].md`, updates `troubleshooting\index.md`
**Entry format:** Error (exact message) / Context (stage + agent) / Root Cause / Fix Applied / Prevention / Tag

### 3.3 Post-QA Agent

#### `refactor-agent`
**Role:** Cleans and optimizes code after QA passes. Zero functional changes.
**Fires:** Manually, after qa-tester gateway OUT confirmed
**Reads:** Full `Source\` tree, `qa-report.md`
**Writes:** Cleaned source files in place
**Constraint:** Never runs before QA passes. Recompile + pluginval re-run required after refactor.
**Gateway OUT:** pluginval still passes after refactor

---

## 4. Execution Workflow

### 4.1 Standard Plugin Build — Full Sequence

```
START
  │
  ▼
[1] Update CLAUDE.md → set Active Plugin name, path, stage = "planning"
  │
  ▼
[2] plugin-planner
    Input:  User concept
    Output: .ideas/ (brief + params + plan)
    Check:  All files exist, no empty fields
  │
  ▼
[3] dsp-architect
    Input:  .ideas/creative-brief + parameter-spec
    Output: .ideas/architecture.md
    Check:  Signal chain numbered, algorithm table complete, risk register present
  │
  ▼
[4] foundation-agent
    Input:  All .ideas/ + skeleton template
    Output: CMakeLists.txt + Source scaffold
    Check:  cmake configure exits 0
  │         ← STAGE 1 GATE
  ▼
[5] dsp-engineer
    Input:  architecture.md + parameter-spec + PluginProcessor.h
    Output: DSP/ classes + updated PluginProcessor.cpp
    Check:  cmake --build exits 0, VST3 binary exists
  │         ← STAGE 2 GATE
  ▼
[6] ui-engineer
    Input:  parameter-spec + creative-brief (UX) + PluginProcessor.h
    Output: UI/ classes + updated PluginEditor.cpp
    Check:  Compiles, all params have attachments, editor renders
  │         ← STAGE 3 GATE
  ▼
[7] qa-tester
    Input:  VST3 binary + parameter-spec
    Output: qa-report.md
    Check:  pluginval 0 errors, all params verified
  │         ← QA GATE
  ▼
[8] refactor-agent
    Input:  Full Source/ + qa-report
    Output: Cleaned source
    Check:  pluginval still passes
  │
  ▼
SHIPPED ✓
  │
  ▼
[9] Update skeleton if lessons learned
    → Add to juce8-critical-patterns.md
    → Update agent.md files if gaps found
    → Propose changes only — do not apply without review
```

### 4.2 Error Path (Any Stage)

```
Agent reports error
  │
  ▼
debug-agent activates
  ├── Check troubleshooting/index.md first
  ├── Identify root cause
  ├── Apply fix
  └── Re-run failed step
        │
        ├── PASS → knowledge-agent logs incident → return to pipeline
        └── FAIL → debug-agent iterates (max 3 attempts)
                    │
                    └── After 3 failures → stop, report to user with full context
```

---

## 5. Checkpoints Reference Table

| Stage | Agent | Command / Check | Pass Condition |
|---|---|---|---|
| Planning | plugin-planner | File existence check | 3 `.ideas/` files complete |
| Architecture | dsp-architect | File existence check | `architecture.md` has chain + table + risks |
| Scaffold | foundation-agent | `cmake -B build -G "Visual Studio 17 2022" -A x64` | Exit code 0 |
| DSP | dsp-engineer | `cmake --build build --config Release` | Exit code 0 + VST3 binary |
| GUI | ui-engineer | Compile + visual check | All params wired, editor renders |
| QA | qa-tester | `pluginval.exe --validate-in-process [path]` | 0 errors |
| Refactor | refactor-agent | `pluginval.exe --validate-in-process [path]` | 0 errors (same as pre-refactor) |

---

## 6. File Dependency Map

```
User Input
    │
    ▼
creative-brief.md ──────────────────────────────► ui-engineer
parameter-spec.md ──────────► foundation-agent ──► dsp-engineer
    │                               │               ui-engineer
    ▼                               ▼               qa-tester
architecture.md ────────────► dsp-engineer
    │
    ▼
juce8-critical-patterns.md ──► foundation-agent
                                dsp-engineer
                                ui-engineer

troubleshooting/index.md ────► debug-agent
                                dsp-engineer (pre-read)
```

---

## 7. CLAUDE.md — Per-Session Update Protocol

At the start of every Claude Code session, update the Active Plugin section in `CLAUDE.md`:

```markdown
## Active Plugin
- Name: [PluginName]
- Path: D:\Dev\PluginSkeleton\plugins\[PluginName]\
- Current Stage: [planning | scaffold | dsp | gui | qa | refactor | complete]
- Last Completed Gate: [none | Stage1 | Stage2 | Stage3 | QA]
```

This ensures every agent in the session has correct context without re-stating it per invocation.

---

## 8. Invoking Agents in Claude Code

### Full Automated Run (Recommended)
```
Run /plugin-workflow for a new plugin called "[Name]".
Concept: [one sentence description].
Type: [synth / fx].
Sonic goal: [what it should sound like / do].
Execute all stages autonomously. Report gate results only.
```

### Single Agent Invocation
```
Activate [agent-name].
Active plugin: [Name] at D:\Dev\PluginSkeleton\plugins\[Name]\
[Any specific instruction if needed]
```

### Error Recovery
```
Activate debug-agent.
Error from [stage name]:
[paste exact error output]
```

### Post-QA Skeleton Update
```
QA passed on [PluginName].
Run refactor-agent, then review this build for skeleton improvements.
Propose changes to juce8-critical-patterns.md and agent files — do not apply without my confirmation.
```

---

## 9. Rules

1. Never modify `.ideas/` contracts after implementation starts. Changes require a new version via a fresh planning cycle.
2. Never skip a gate. Each gate exists because the next stage will produce incorrect output without it.
3. Never run refactor-agent before QA passes.
4. debug-agent checks `troubleshooting/index.md` before attempting any fix. Knowledge reuse is mandatory.
5. knowledge-agent runs after every debug resolution without exception. The KB is only valuable if it's complete.
6. Skeleton files are never edited during an active plugin build. Improvements are proposed post-ship, reviewed, then applied.
7. All agents operate on Windows/MSVC/VST3 only. No Mac, no AU, no WebView.

---

## 10. Starting a New Plugin — Quickstart

```
1. Open terminal: cd D:\Dev\PluginSkeleton
2. Open Claude Code: claude
3. Update CLAUDE.md Active Plugin section
4. Run: "Start a new plugin called [Name]. [One sentence concept]. Run plugin-workflow."
5. Monitor gate results. Intervene only on 3-failure debug loops.
6. Load VST3 in Ableton after QA passes.
7. Run post-ship skeleton review.
```

---

*This manual is a living document. Update it after every skeleton improvement cycle.*
