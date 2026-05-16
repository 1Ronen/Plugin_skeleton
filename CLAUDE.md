# PluginSkeleton

JUCE VST3 plugin skeleton for Windows (MSVC/CMake). Reusable framework for audio effects.

## Environment

- JUCE: `D:\HISE_Dev\JUCE`
- Compiler: MSVC (Visual Studio 2022, x64)
- Format: VST3 only
- GUI: JUCE native (no WebView, no Standalone)

## Directory Layout

```
CMakeLists.txt              — root build: adds JUCE, includes active plugin
template/                   — template files with [PluginName] placeholders
plugins/[Name]/             — instantiated plugin
  CMakeLists.txt
  Source/
    PluginProcessor.h/cpp
    PluginEditor.h/cpp
.ideas/                     — contract templates
  creative-brief.md
  parameter-spec.md
  architecture.md
  plan.md
.claude/skills/             — workflow skills
.claude/agents/             — subagent definitions
juce8-critical-patterns.md  — MSVC/JUCE 8 gotchas — read before implementing
scripts/build.ps1           — one-command build script
```

## Build

```powershell
.\scripts\build.ps1
```

Or manually:
```powershell
cmake -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
```

VST3 output: `build\plugins\[Name]\[Name]_artefacts\Release\VST3\`

## New Plugin Workflow

1. Fill in `.ideas/` contract templates for the new plugin
2. Use `/new-plugin` skill — it instantiates the template and updates `CMakeLists.txt`
3. Use `/plugin-ideation` to refine contracts if needed
4. Use `/build-compile` to build and verify

## Active Plugin

Currently: `plugins/SimpleGain`  
Switch: edit root `CMakeLists.txt` → update `add_subdirectory(plugins/YourPlugin)`

## Critical Reference

Read `juce8-critical-patterns.md` before implementing any DSP or UI code.
