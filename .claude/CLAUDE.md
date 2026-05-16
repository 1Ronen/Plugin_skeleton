# PluginSkeleton — Agent System

## Stack

| Item | Value |
|---|---|
| JUCE | `D:\HISE_Dev\JUCE` (version 8) |
| Compiler | MSVC (Visual Studio 2026, v18) |
| CMake generator | `Visual Studio 18 2026` |
| Format | VST3 only |
| GUI | JUCE native — no WebView |
| Platform | Windows x64 only |

Build root: `D:\Dev\PluginSkeleton\`
Plugin root: `D:\Dev\PluginSkeleton\plugins\`
Skeleton template: `D:\Dev\PluginSkeleton\template\`

## Agent Roster

### Primary Pipeline Agents

| Agent | Location | Role |
|---|---|---|
| `plugin-planner` | `.claude/agents/plugin-planner/agent.md` | Produces all `.ideas/` contract files from user brief |
| `dsp-architect` | `.claude/agents/dsp-architect/agent.md` | Designs signal chain and produces `architecture.md` |
| `foundation-agent` | `.claude/agents/foundation-agent/agent.md` | Instantiates template, implements CMakeLists + APVTS parameters |
| `dsp-engineer` | `.claude/agents/dsp-engineer/agent.md` | Implements `processBlock`, `prepareToPlay`, DSP component classes |
| `ui-engineer` | `.claude/agents/ui-engineer/agent.md` | Implements `PluginEditor` with all APVTS control attachments |
| `qa-tester` | `.claude/agents/qa-tester/agent.md` | Runs pluginval at strictness 10, writes `qa-report.md` |

### Reactive Agents

| Agent | Location | Role |
|---|---|---|
| `debug-agent` | `.claude/agents/debug-agent/agent.md` | Diagnoses build/runtime errors, applies fix, retries failing stage |
| `knowledge-agent` | `.claude/agents/knowledge-agent/agent.md` | Writes resolved error to `troubleshooting/` and updates the index |

### Post-QA Agent

| Agent | Location | Role |
|---|---|---|
| `refactor-agent` | `.claude/agents/refactor-agent/agent.md` | Cleans source after all-pass QA — zero functional changes |

### Legacy Flat Agents (used directly by skills)

| Agent | Location | Role |
|---|---|---|
| `foundation-agent` (flat) | `.claude/agents/foundation-agent.md` | Original foundation agent — generates CMakeLists + PluginProcessor from contracts |
| `dsp-agent` (flat) | `.claude/agents/dsp-agent.md` | Original DSP agent — implements processBlock and DSP component files |
| `validation-agent` (flat) | `.claude/agents/validation-agent.md` | Original validation agent — verifies VST3 artifact, parameter coverage, real-time safety |

## Agent Execution Order

### Primary pipeline (always run in order)
```
plugin-planner → dsp-architect → foundation-agent → dsp-engineer → ui-engineer → qa-tester
```

No stage may begin until the previous stage's completion criteria are met:
- plugin-planner: all .ideas/ contracts written and confirmed by user
- dsp-architect: architecture.md complete, all parameters mapped
- foundation-agent: CMake configure passes, parameter coverage 100%
- dsp-engineer: Release build passes, processBlock implemented
- ui-engineer: Release build passes, all controls attached to APVTS
- qa-tester: pluginval passes strictness 10, all validation checks green

### Reactive agents (fire on error from any stage)
```
[failing stage] → debug-agent → knowledge-agent → [failing stage retried]
```

### Post-QA (optional, run after qa-tester all-pass)
```
refactor-agent
```

## File Handshake Chain

Each agent's **Writes To** is the next agent's **Reads From**. Verify files exist before starting each stage.

```
plugin-planner
  Writes: plugins/[Name]/.ideas/creative-brief.md
          plugins/[Name]/.ideas/parameter-spec.md
          plugins/[Name]/.ideas/plan.md
          ↓
dsp-architect
  Reads:  .ideas/creative-brief.md, .ideas/parameter-spec.md, .ideas/plan.md
  Writes: plugins/[Name]/.ideas/architecture.md
          ↓
foundation-agent
  Reads:  .ideas/creative-brief.md, .ideas/parameter-spec.md,
          .ideas/architecture.md, .ideas/plan.md,
          juce8-critical-patterns.md, template/
  Writes: plugins/[Name]/CMakeLists.txt
          plugins/[Name]/Source/PluginProcessor.h
          plugins/[Name]/Source/PluginProcessor.cpp
          plugins/[Name]/Source/PluginEditor.h
          plugins/[Name]/Source/PluginEditor.cpp
          ↓
dsp-engineer
  Reads:  .ideas/architecture.md, .ideas/parameter-spec.md,
          Source/PluginProcessor.h, Source/PluginProcessor.cpp,
          juce8-critical-patterns.md
  Writes: plugins/[Name]/Source/PluginProcessor.cpp (processBlock, prepareToPlay)
          plugins/[Name]/Source/DSP/[Component].h/.cpp
          plugins/[Name]/CMakeLists.txt (adds DSP sources)
          ↓
ui-engineer
  Reads:  .ideas/parameter-spec.md, .ideas/creative-brief.md,
          Source/PluginProcessor.h, Source/PluginEditor.h/.cpp,
          juce8-critical-patterns.md
  Writes: plugins/[Name]/Source/PluginEditor.h
          plugins/[Name]/Source/PluginEditor.cpp
          ↓
qa-tester
  Reads:  build/.../[ProductName].vst3, Source/PluginProcessor.cpp
  Writes: plugins/[Name]/qa-report.md
          ↓
[on error at any stage]
debug-agent
  Reads:  build logs, Source files, troubleshooting/index.md
  Writes: Source files (fix applied)
          ↓
knowledge-agent
  Reads:  debug-agent output (error, root cause, fix)
  Writes: troubleshooting/[YYYY-MM-DD]-[category]-[slug].md
          troubleshooting/index.md (one row appended)
```

## Active Plugin

| Field | Value |
|---|---|
| Name | *(set per plugin)* |
| Path | `plugins/[PluginName]/` |
| Current stage | *(set per plugin)* |
| Last completed | *(set per plugin)* |

Update this section when a new plugin is started or a stage completes.

## Global Rules

1. **Never skip stages.** foundation-agent does not run before dsp-architect. dsp-engineer does not run before foundation-agent passes CMake configure.

2. **Always read .ideas/ contracts before implementing.** Do not rely on memory of a prior conversation. Read the files.

3. **Terse output only.** Report results and errors. No narration, no explanation of what you are about to do.

4. **Windows + MSVC only.** Zero Mac, AU, Linux, or Standalone references anywhere in any output.

5. **VST3 only.** Never add AU, VST2, or Standalone to FORMATS.

6. **JUCE native GUI only.** No WebView, no third-party UI libraries.

7. **Contracts are final once confirmed.** Parameter IDs in parameter-spec.md do not change after foundation-agent runs. Architecture decisions in architecture.md do not change after dsp-architect completes. Any required change goes back to the user for approval.

8. **debug-agent is the only agent that touches broken code.** Other agents do not improvise fixes — they stop, flag, and invoke debug-agent.

9. **knowledge-agent fires after every debug-agent success.** Every resolved error becomes a KB entry.

## Critical JUCE 8 Patterns

Full reference: `juce8-critical-patterns.md`

Read that file before implementing any agent step. Key rules:

- `juce_generate_juce_header` after `target_link_libraries` in CMakeLists.txt
- `isBusesLayoutSupported` must be overridden — accept mono and stereo, in == out
- `juce::Font(size, style)` deprecated — use `juce::Font(juce::FontOptions().withHeight(N).withStyle("Bold"))`
- `getRawParameterValue("id")->load()` in processBlock — cache pointer in prepareToPlay
- `ScopedNoDenormals` as first line of processBlock
- `FORMATS VST3` only — no Standalone
- `JUCE_WEB_BROWSER=0` and `JUCE_USE_CURL=0` always in compile definitions
- `CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"` in root CMakeLists.txt

## Skills

| Skill | Location | Invocation | Role |
|---|---|---|---|
| `new-plugin` | `.claude/skills/new-plugin/SKILL.md` | `"Create new plugin called [Name], type [synth/fx]"` | Copies skeleton to `D:\Dev\Plugins\[Name]\`, instantiates template, scaffolds `.ideas/`, updates CLAUDE.md |
| `build-compile` | `.claude/skills/build-compile/SKILL.md` | `"Build"` or `"/build-compile"` | Runs CMake configure + Release build, reports VST3 path and any errors |
| `plugin-ideation` | `.claude/skills/plugin-ideation/SKILL.md` | `"/plugin-ideation"` | Refines `.ideas/` contracts through structured Q&A before implementation starts |
| `plugin-improve` | `.claude/skills/plugin-improve/SKILL.md` | `"/plugin-improve"` | Applies targeted improvements to an existing plugin without full pipeline re-run |
| `status` | `.claude/skills/status/SKILL.md` | `"Run status"` or `"/status"` | Single-line snapshot: plugin name, stage, last gate, next agent, missing gate files |
| `resume` | `.claude/skills/resume/SKILL.md` | `"Resume"` or `"/resume"` | Reads last completed gate and activates the correct next agent automatically |
| `version-bump` | `.claude/skills/version-bump/SKILL.md` | `"Bump version to [x.x.x]"` | Updates version string in `creative-brief.md` and `CMakeLists.txt` |
| `new-version` | `.claude/skills/new-version/SKILL.md` | `"Start new version [x.x.x] of [PluginName]"` | Archives current `.ideas/` to `versions/v[N]/`, resets pipeline to planning with v[N] as baseline |

## Troubleshooting Knowledge Base

Index: `troubleshooting/index.md`

When debug-agent resolves an error, knowledge-agent writes a dated entry to `troubleshooting/` and appends one row to the index. Before debugging any error, check `troubleshooting/index.md` for a prior solution.

## VST3 Output Path

```
build\plugins\[PluginName]\[PluginName]_artefacts\Release\VST3\[ProductName].vst3
```

Install to DAW: copy `.vst3` directory to `C:\Program Files\Common Files\VST3\`
