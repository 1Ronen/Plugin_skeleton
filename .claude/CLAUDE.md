# PluginSkeleton — Agent System

Build root: `D:\Dev\PluginSkeleton\`
Plugin root: `D:\Dev\PluginSkeleton\plugins\`
Template: `D:\Dev\PluginSkeleton\template\`

## Stack

| Item | Value |
|---|---|
| JUCE | `D:\HISE_Dev\JUCE` (v8) |
| Compiler | MSVC (Visual Studio 2026, v18) |
| CMake generator | `Visual Studio 18 2026` |
| Format | VST3 only |
| GUI | JUCE native — no WebView |
| Platform | Windows x64 only |

## Agents

| Agent | Role |
|---|---|
| `plugin-planner` | Produces all `.ideas/` contract files |
| `dsp-architect` | Designs signal chain, produces `architecture.md` |
| `foundation-agent` | Instantiates template, CMakeLists + APVTS parameters |
| `dsp-engineer` | Implements `processBlock`, `prepareToPlay`, DSP components |
| `ui-engineer` | Implements `PluginEditor` with all APVTS attachments |
| `qa-tester` | Runs pluginval strictness 10, writes `qa-report.md` |
| `debug-agent` | Diagnoses build/runtime errors, applies fix, retries stage |
| `knowledge-agent` | Writes resolved errors to `troubleshooting/` |
| `refactor-agent` | Cleans source after all-pass QA — zero functional changes |
| `foundation-agent` (flat) | Legacy: generates CMakeLists + PluginProcessor from contracts |
| `dsp-agent` (flat) | Legacy: implements processBlock and DSP component files |
| `validation-agent` (flat) | Legacy: verifies VST3 artifact, parameter coverage, safety |

## Skills

| Skill | Invocation | Role |
|---|---|---|
| `new-plugin` | `"Create new plugin called [Name], type [synth/fx]"` | Copies skeleton, instantiates template, scaffolds `.ideas/` |
| `build-compile` | `"Build"` / `"/build-compile"` | CMake configure + Release build, reports VST3 path |
| `plugin-ideation` | `"/plugin-ideation"` | Refines `.ideas/` contracts through Q&A |
| `plugin-improve` | `"/plugin-improve"` | Targeted improvements to existing plugin |
| `status` | `"Run status"` / `"/status"` | Snapshot: plugin name, stage, gate, next agent |
| `resume` | `"Resume"` / `"/resume"` | Activates correct next agent from last completed gate |
| `version-bump` | `"Bump version to [x.x.x]"` | Updates version in `creative-brief.md` + `CMakeLists.txt` |
| `new-version` | `"Start new version [x.x.x] of [PluginName]"` | Archives `.ideas/`, resets pipeline to planning |
| `backup` | `"backup"` / `"/backup"` | QA-gated push — both repos, versioned commit |

## Git

| Repo | Path | Remote |
|---|---|---|
| Plugins | `D:\Dev\Plugins\` | `github.com/1Ronen/my-plugins` |
| Skeleton | `D:\Dev\PluginSkeleton\` | `github.com/1Ronen/plugin-skeleton` |

Push policy: backup skill only. `qa-report.md` Status: PASS required. Both repos always pushed together.

## Active Plugin

| Field | Value |
|---|---|
| Name | *(set per plugin)* |
| Path | `plugins/[PluginName]/` |
| Current stage | *(set per plugin)* |
| Last completed | *(set per plugin)* |

## Rules

1. Pipeline order is enforced — no stage runs before the previous stage's gate passes.
2. Always read `.ideas/` contracts before implementing — never rely on conversation memory.
3. VST3 only — no AU, no Standalone, no WebView, no third-party UI libraries.
4. `debug-agent` is the only agent that touches broken code — others stop and flag.
5. `knowledge-agent` fires after every `debug-agent` success — every fix becomes a KB entry.

## References

Studio identity: `studio-identity.md`
JUCE 8 patterns: `juce8-critical-patterns.md`
Troubleshooting KB: `troubleshooting/index.md`
VST3 output: `build\plugins\[PluginName]\[PluginName]_artefacts\Release\VST3\`
