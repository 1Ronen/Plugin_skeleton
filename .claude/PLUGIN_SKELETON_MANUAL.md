# Plugin Skeleton — Developer Manual
**Version 2.0 | Stack: JUCE 8 / MSVC / CMake / VST3 / Windows**

---

## 1. System Overview

The Plugin Skeleton is a reusable, agent-driven development framework for building professional JUCE VST3 plugins on Windows. Every plugin produced by this system follows an identical pipeline — only the DSP and UI contents change. The skeleton lives at `D:\Dev\PluginSkeleton\` and is never modified during a plugin build. Each new plugin is instantiated into its own directory via the `new-plugin` skill before any agent fires.

### Core Principle
**Files are the handshake between agents.** No agent communicates with another directly. All coordination happens through the filesystem — specifically through `.ideas/` contract files and compiled build artifacts. Every agent has a defined input set it reads and an output set it writes. Nothing else.

---

## 2. Directory Structure

```
D:\Dev\PluginSkeleton\                   <- Skeleton — never edited during a build
├── .claude\
│   ├── CLAUDE.md                        <- Master context — loaded every session
│   ├── PLUGIN_SKELETON_MANUAL.md        <- This file (AI-readable reference)
│   ├── agents\
│   │   ├── plugin-planner\agent.md
│   │   ├── dsp-architect\agent.md
│   │   ├── foundation-agent\agent.md
│   │   ├── dsp-engineer\agent.md
│   │   ├── ui-engineer\agent.md
│   │   ├── qa-tester\agent.md
│   │   ├── debug-agent\agent.md
│   │   ├── refactor-agent\agent.md
│   │   ├── knowledge-agent\agent.md
│   │   └── installer-agent\agent.md
│   ├── skills\
│   │   ├── plugin-workflow\             <- Master orchestrator
│   │   ├── new-plugin\                  <- Plugin instantiation from skeleton
│   │   └── backup\                      <- Git push skill
│   └── juce8-critical-patterns.md      <- Injected into all implementation agents
├── installer\
│   └── eula-template.md                 <- Base EULA, customized per plugin
├── docs\
│   └── PLUGIN_SKELETON_MANUAL.docx      <- Human-readable version of this file
└── troubleshooting\
    ├── index.md                         <- KB index, updated after every resolved bug
    └── [YYYY-MM-DD]-[slug].md           <- One file per resolved incident

D:\Dev\Plugins\                          <- All instantiated plugins live here
└── [PluginName]\                        <- Created by new-plugin skill
    ├── .ideas\
    │   ├── creative-brief.md
    │   ├── parameter-spec.md
    │   ├── architecture.md
    │   └── plan.md
    ├── Source\
    │   ├── PluginProcessor.h/.cpp
    │   ├── PluginEditor.h/.cpp
    │   ├── DSP\
    │   └── UI\
    ├── installer\
    │   ├── [PluginName].nsi
    │   ├── [PluginName]_Setup.exe
    │   └── install-log.md
    ├── build\
    ├── CMakeLists.txt
    └── qa-report.md
```

---

## 3. Agent Roster

### 3.1 Entry Skill — `new-plugin`

Not an agent — a skill. Mandatory first step for every plugin. Invoke before any agent.

**What it does:**
- Copies `D:\Dev\PluginSkeleton\` to `D:\Dev\Plugins\[PluginName]\`
- Excludes: `.git\`, `build\`, existing plugin subfolders
- Renames all CMakeLists.txt placeholders: PLUGIN_NAME, PLUGIN_CODE, PRODUCT_NAME, IS_SYNTH
- Creates empty `.ideas\` folder with blank template files
- Updates `CLAUDE.md` Active Plugin section: name, path, stage = planning, gate = none
- Prints full directory tree of new plugin folder on completion

**Invocation:** `"Create new plugin called [Name], type [synth/fx]"`

---

### 3.2 Pipeline Agents (Sequential)

#### `plugin-planner`
**Role:** Translates a plugin concept into complete `.ideas/` contract files. No code.
**Reads:** User input (concept, name, sonic goal)
**Writes:** `.ideas/creative-brief.md`, `.ideas/parameter-spec.md`, `.ideas/plan.md`
**Gateway IN:** `new-plugin` skill completed. Plugin directory exists at `D:\Dev\Plugins\[Name]\`
**Gateway OUT:** All three `.ideas/` files exist and are complete — no empty fields

#### `dsp-architect`
**Role:** Designs signal flow and selects algorithms. No implementation — specification only.
**Reads:** `.ideas/creative-brief.md`, `.ideas/parameter-spec.md`
**Writes:** `.ideas/architecture.md`
**Gateway IN:** `plugin-planner` Gateway OUT passed
**Gateway OUT:** `architecture.md` contains numbered signal chain, algorithm-per-stage table, risk register

#### `foundation-agent`
**Role:** Scaffolds CMakeLists.txt, project structure, and parameter skeleton from template.
**Reads:** All `.ideas/` files, `D:\Dev\PluginSkeleton\` template, `juce8-critical-patterns.md`
**Writes:** `CMakeLists.txt`, `PluginProcessor.h/.cpp`, `PluginEditor.h/.cpp` (empty stubs)
**Gateway IN:** `dsp-architect` Gateway OUT passed
**Gateway OUT:** `cmake -B build -G "Visual Studio 17 2022" -A x64` exits code 0

#### `dsp-engineer`
**Role:** Implements all audio processing logic in processBlock and DSP classes.
**Reads:** `.ideas/architecture.md`, `.ideas/parameter-spec.md`, `PluginProcessor.h`, `juce8-critical-patterns.md`, `troubleshooting\index.md`
**Writes:** `Source\DSP\*.h/.cpp`, updates `PluginProcessor.cpp`
**Gateway IN:** `foundation-agent` Gateway OUT passed
**Gateway OUT:** `cmake --build build --config Release` exits 0. VST3 binary exists in `build\`
**Constraint:** If a DSP decision contradicts `architecture.md`, stop and flag — do not improvise

#### `ui-engineer`
**Role:** Implements JUCE native GUI — layout, knobs, labels, LookAndFeel.
**Reads:** `.ideas/parameter-spec.md`, `.ideas/creative-brief.md` (UX section), `PluginProcessor.h`, `juce8-critical-patterns.md`
**Writes:** `Source\UI\*.h/.cpp`, updates `PluginEditor.cpp`
**Gateway IN:** `dsp-engineer` Gateway OUT passed
**Gateway OUT:** Compiles clean. All parameters have SliderAttachment. Editor renders at correct size.
**Constraint:** JUCE native components only — no WebView, no third-party UI libraries

#### `qa-tester`
**Role:** Validates compiled plugin against technical and functional requirements.
**Reads:** Compiled VST3 binary, `.ideas/parameter-spec.md`
**Writes:** `qa-report.md`
**Gateway IN:** `ui-engineer` Gateway OUT passed
**Gateway OUT:** pluginval exits 0 errors. All parameters verified. `qa-report.md` written.
**Constraint:** On failure: log to `troubleshooting/` and invoke `debug-agent`. Never modifies source directly.

---

### 3.3 Reactive Agents (Fire on Error)

#### `debug-agent`
**Role:** Diagnoses and resolves build failures, runtime crashes, DSP artifacts.
**Fires:** Automatically when any pipeline agent reports an error
**Reads:** Error output, `troubleshooting\index.md` (KB check first)
**Writes:** Fixed source files, re-runs failed step
**Constraint:** Fix only the reported error. Max 3 attempts before escalating. If fix requires modifying `.ideas/` contracts, stop and flag — never modify contracts unilaterally.
**Hands back to:** Originating agent after fix confirmed

#### `knowledge-agent`
**Role:** Captures every resolved error into the persistent troubleshooting KB.
**Fires:** After every successful `debug-agent` resolution
**Reads:** debug-agent session output (error + root cause + fix)
**Writes:** `troubleshooting\[YYYY-MM-DD]-[slug].md`, updates `troubleshooting\index.md`
**Entry format:** Error / Context (stage + agent) / Root Cause / Fix Applied / Prevention / Tag

---

### 3.4 Post-QA Agents (Sequential)

#### `refactor-agent`
**Role:** Cleans and optimizes code after QA passes. Zero functional changes.
**Fires:** Manually, after `qa-tester` Gateway OUT confirmed
**Reads:** Full `Source\` tree, `qa-report.md`
**Writes:** Cleaned source files in place
**Gateway OUT:** pluginval still passes. Change list documented.
**Constraint:** Never runs before QA passes. Recompile + pluginval re-run mandatory after refactor.

#### `installer-agent`
**Role:** Generates a Windows installer wizard (.exe) for the compiled VST3 using NSIS.
**Fires:** After `refactor-agent` Gateway OUT confirmed
**Reads:** `.ideas/creative-brief.md` (name, vendor, version), compiled VST3 binary, `installer\eula-template.md`
**Writes:** `installer\[PluginName].nsi`, `installer\[PluginName]_Setup.exe`, `installer\install-log.md`
**Gateway IN:** `refactor-agent` complete. pluginval passing. VST3 binary confirmed.
**Gateway OUT:** `[PluginName]_Setup.exe` exists. `install-log.md` written. File size > 0.
**Constraint:** NSIS must be in PATH. If missing, output download instructions and stop. Never overwrites existing installer without warning. Version from `creative-brief.md` only — never hardcoded.

**Installer wizard includes:**
- Welcome screen (plugin name + vendor)
- EULA page (from `eula-template.md`, customized per plugin)
- Install destination: `C:\Program Files\Common Files\VST3\[Vendor]\`
- VST3 bundle copy with correct folder structure
- Presets folder creation at `Documents\[Vendor]\[PluginName]\Presets\`
- Uninstaller registration in Windows Add/Remove Programs
- Finish page with DAW rescan instructions

---

## 4. Execution Workflow

### 4.1 Standard Build — Full Sequence

```
START
  |
  v
[0] new-plugin skill
    "Create new plugin called [Name], type [synth/fx]"
    Creates D:\Dev\Plugins\[Name]\ from skeleton
    <- INIT GATE: directory exists, CLAUDE.md updated
  |
  v
[1] plugin-planner
    Output: .ideas/ (brief + params + plan)
    Gate:   All files complete, no empty fields
  |
  v
[2] dsp-architect
    Output: .ideas/architecture.md
    Gate:   Chain + algorithm table + risk register present
  |
  v
[3] foundation-agent
    Output: CMakeLists.txt + Source scaffold
    Gate:   cmake configure exits 0
  |         <- STAGE 1 GATE
  v
[4] dsp-engineer
    Output: DSP/ classes + updated PluginProcessor.cpp
    Gate:   cmake --build exits 0, VST3 binary exists
  |         <- STAGE 2 GATE
  v
[5] ui-engineer
    Output: UI/ classes + updated PluginEditor.cpp
    Gate:   All params wired, editor renders
  |         <- STAGE 3 GATE
  v
[6] qa-tester
    Output: qa-report.md
    Gate:   pluginval 0 errors
  |         <- QA GATE
  v
[7] refactor-agent
    Output: Cleaned source
    Gate:   pluginval still passes
  |
  v
[8] installer-agent
    Output: [PluginName]_Setup.exe + install-log.md
    Gate:   .exe exists, size > 0
  |         <- INSTALLER GATE
  v
SHIPPED
  |
  v
[9] Post-ship
    -> Add lessons to juce8-critical-patterns.md
    -> Update agent.md files if gaps found
    -> Run backup skill
    -> Propose skeleton changes — apply after your review
```

### 4.2 Error Path (Any Stage)

```
Agent reports error
  |
  v
debug-agent activates
  |- Check troubleshooting/index.md first (KB lookup)
  |- Identify root cause
  |- Apply fix
  +- Re-run failed step
        |
        |- PASS -> knowledge-agent logs incident -> return to pipeline
        +- FAIL -> iterate (max 3 attempts)
                    |
                    +- Escalate to user with full context
```

---

## 5. Checkpoint Reference

| Stage | Agent / Skill | Command / Check | Pass Condition |
|---|---|---|---|
| Init | new-plugin | Directory tree check | Plugin folder + `.ideas/` exist |
| Planning | plugin-planner | File existence check | 3 `.ideas/` files complete |
| Architecture | dsp-architect | File existence check | Chain + table + risks present |
| Scaffold | foundation-agent | `cmake -B build -G "Visual Studio 17 2022" -A x64` | Exit code 0 |
| DSP | dsp-engineer | `cmake --build build --config Release` | Exit 0 + VST3 binary |
| GUI | ui-engineer | Compile + visual check | All params wired, renders |
| QA | qa-tester | `pluginval.exe --validate-in-process [path]` | 0 errors |
| Refactor | refactor-agent | `pluginval.exe --validate-in-process [path]` | 0 errors post-refactor |
| Installer | installer-agent | File existence + size check | `.exe` exists, size > 0 |

---

## 6. File Dependency Map

```
new-plugin skill
  | creates plugin directory
  v
User Input ─────────────────────────────────────────── installer-agent
  |                                                           ^
  v                                                           |
creative-brief.md ──────────────────── ui-engineer    eula-template.md
parameter-spec.md ── foundation-agent ─ dsp-engineer
  |                        |            ui-engineer
  v                        v            qa-tester
architecture.md ──── dsp-engineer

juce8-critical-patterns.md ── foundation-agent
                               dsp-engineer
                               ui-engineer

troubleshooting/index.md ──── debug-agent
                               dsp-engineer (pre-read)
```

---

## 7. CLAUDE.md — Per-Session Update Protocol

Update at the start of every Claude Code session:

```
## Active Plugin
- Name: [PluginName]
- Path: D:\Dev\Plugins\[PluginName]\
- Current Stage: [planning | scaffold | dsp | gui | qa | refactor | installer | complete]
- Last Completed Gate: [none | Init | Stage1 | Stage2 | Stage3 | QA | Installer]
```

---

## 8. Agent Invocation Reference

### Start a New Plugin
```
Create new plugin called [Name], type [synth/fx].
```

### Full Automated Build (after new-plugin)
```
Run /plugin-workflow for [Name].
Concept: [one sentence].
Sonic goal: [what it sounds like / does].
Execute all stages autonomously. Report gate results only.
```

### Single Agent
```
Activate [agent-name].
Active plugin: [Name] at D:\Dev\Plugins\[Name]\
[Specific instruction if needed]
```

### Error Recovery
```
Activate debug-agent.
Error from [stage]:
[paste exact error output]
```

### Build Installer Only
```
Activate installer-agent for [PluginName].
VST3 path: D:\Dev\Plugins\[PluginName]\build\[PluginName]_artefacts\Release\VST3\
```

### Git Backup
```
Run backup.
```

### Post-QA Skeleton Update
```
QA passed on [PluginName].
Run refactor-agent, then review this build for skeleton improvements.
Propose changes to juce8-critical-patterns.md and agent files.
Do not apply without my confirmation.
```

---

## 9. Remote Initiation from iPhone

You can start a full plugin build from your iPhone without touching your PC.

### Option A — SSH via Termius (Recommended)

**One-time PC setup (20 minutes):**
```
1. Enable OpenSSH Server:
   Settings > System > Optional Features > OpenSSH Server > Install > Start

2. Find your PC local IP:
   Run in terminal: ipconfig
   Look for IPv4 Address (e.g. 192.168.1.x)

3. Confirm SSH works from PC first:
   ssh [your-windows-username]@localhost
```

**One-time iPhone setup:**
```
1. Install Termius (free — App Store)
2. New Host: [your PC local IP], port 22
3. Username: your Windows username
4. Password: your Windows password
5. Connect — you should land in your Windows terminal
```

**Starting a build from iPhone:**
```
In Termius:
  cd D:\Dev\PluginSkeleton
  claude
  "Create new plugin called [Name], type [synth/fx]. Run plugin-workflow."

Claude Code runs on your PC. You watch and intervene from iPhone.
Build output streams to your phone screen in real time.
```

**Keep session alive if phone locks:**
```
Before connecting from iPhone, run on PC:
  wsl -- screen -S plugin-build

Then connect via SSH and attach:
  wsl -- screen -r plugin-build

Session survives phone lock and reconnects cleanly.
```

### Option B — Claude.ai Mobile (Concept Only)
Use Claude.ai on iPhone to draft and refine the plugin concept and parameter spec. Copy the finalized prompt. Paste into Termius SSH session to kick off the build. Good for planning during commute — execution still goes through SSH.

### Option C — GitHub Trigger (Future / Advanced)
Push a `plugin-request.md` from iPhone → GitHub repo via Shortcuts app → GitHub Action triggers Claude Code via self-hosted runner on your PC. Fully automated. Revisit after the skeleton is stable and you have 5+ plugins shipped.

### Recommended Now
**Option A only.** SSH + Termius. Works today, zero additional cost, 20 minutes setup.

---

## 10. Prerequisite Checklist

| Tool | Location / Command | Required For |
|---|---|---|
| JUCE 8 | `D:\HISE_Dev\JUCE` | All builds |
| CMake >= 3.22 | `cmake --version` | All builds |
| Visual Studio 2022 | MSVC + C++ workload | All builds |
| Git | `git --version` | Backup skill |
| pluginval | `pluginval.exe --version` | qa-tester |
| NSIS | `makensis /VERSION` | installer-agent |
| GitHub CLI (optional) | `gh --version` | Repo creation |
| OpenSSH Server (PC) | Windows Optional Features | iPhone remote |
| Termius (iPhone) | App Store — free | iPhone remote |

---

## 11. Rules

1. Always run `new-plugin` skill first — never build directly in `D:\Dev\PluginSkeleton\`
2. Never modify `.ideas/` contracts after implementation starts — changes require a new version cycle
3. Never skip a gate — each gate exists because the next stage will fail without it
4. Never run `refactor-agent` before QA passes
5. Never run `installer-agent` before `refactor-agent` completes
6. `debug-agent` checks `troubleshooting/index.md` before attempting any fix — KB reuse mandatory
7. `knowledge-agent` runs after every debug resolution — KB is only valuable if complete
8. Skeleton files are never edited during an active plugin build — improvements proposed post-ship only
9. All agents: Windows / MSVC / VST3 only — no Mac, no AU, no WebView
10. Run `backup` skill after every completed gate on a new plugin

---

## 12. Skills Reference

Skills are lightweight orchestrators invoked directly in the Claude Code prompt. They are not agents — they do not build or analyse code on their own. They read files, delegate to agents, and report results.

| Skill | Invocation | What it does |
|---|---|---|
| `new-plugin` | `Create new plugin called [Name], type [synth/fx]` | Copies skeleton to `D:\Dev\Plugins\[Name]\`, instantiates template, creates empty `.ideas/` contracts, updates CLAUDE.md |
| `build-compile` | `Build` or `/build-compile` | CMake configure + Release build; reports VST3 output path; surfaces compiler errors |
| `plugin-ideation` | `/plugin-ideation` | Interactive contract refinement — fills in `.ideas/` files through structured Q&A before implementation |
| `plugin-improve` | `/plugin-improve` | Applies targeted improvements to a shipped plugin (DSP quality, UI polish) without re-running the full pipeline |
| `status` | `Run status` or `/status` | One-line snapshot: `Plugin: [Name] \| Stage: [stage] \| Last Gate: [gate] \| Next Agent: [agent]` — also lists missing gate files |
| `resume` | `Resume` or `/resume` | Reads last completed gate from CLAUDE.md and activates the correct next agent automatically |
| `version-bump` | `Bump version to [x.x.x]` | Updates version string in `creative-brief.md` and `CMakeLists.txt`; prints old -> new for both |
| `new-version` | `Start new version [x.x.x] of [PluginName]` | Archives `.ideas/` to `versions/v[N]/`, resets to planning stage with archived brief as baseline, runs version-bump |

### Typical skill sequences

**Starting a new plugin:**
```
Create new plugin called [Name], type [synth/fx].
/plugin-ideation
Resume
```

**Checking where you left off:**
```
/status
/resume
```

**Shipping a new version:**
```
Start new version 1.1.0 of [PluginName].
[Fill in updated .ideas/ contracts]
/resume
```

**After any gate passes:**
```
/status         <- confirm gate recorded
/resume         <- fire next agent
```

---

## 13. New Plugin Quickstart

```
From iPhone (SSH) or PC terminal:

1. cd D:\Dev\PluginSkeleton
2. claude
3. "Create new plugin called [Name], type [synth/fx]."
4. "Run plugin-workflow. Concept: [one sentence]. Sonic goal: [description]."
5. Monitor gate results — intervene only on 3-failure debug loops
6. Load VST3 in Ableton after QA gate passes
7. Run installer-agent — test [PluginName]_Setup.exe
8. Run backup — push to git
9. Run post-ship skeleton review
```

---

*This manual is a living document. Update after every skeleton improvement cycle.*
*Version the DOCX manually (v2.0, v2.1 etc.) after each update.*
