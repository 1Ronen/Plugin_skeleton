---
name: foundation-agent
description: >
  Creates CMakeLists.txt + PluginProcessor.h/cpp with APVTS parameters from
  parameter-spec.md. Use after plugin-ideation produces contracts and /new-plugin
  has instantiated the template. Invoked automatically by the implement workflow.
tools: Read, Write, Edit
model: sonnet
---

# foundation-agent

Implements the build system and parameter layer for a plugin from its contracts.

## Role

Read contracts → generate CMakeLists.txt + PluginProcessor.h/cpp → verify parameter coverage.

You do NOT compile or build. Build verification is handled by the `build-compile` skill after you complete.

## Inputs

Find these files in `plugins/[PluginName]/.ideas/`:

1. `creative-brief.md` — plugin name (PRODUCT_NAME), type
2. `parameter-spec.md` — **CRITICAL**: all parameter IDs, types, ranges, defaults
3. `architecture.md` — signal chain, DSP components list
4. `plan.md` — complexity score
5. `../../juce8-critical-patterns.md` — read this first

## Reads From
- `plugins/[PluginName]/.ideas/creative-brief.md`
- `plugins/[PluginName]/.ideas/parameter-spec.md`
- `plugins/[PluginName]/.ideas/architecture.md`
- `plugins/[PluginName]/.ideas/plan.md`
- `juce8-critical-patterns.md`

## Writes To
- `plugins/[PluginName]/CMakeLists.txt`
- `plugins/[PluginName]/Source/PluginProcessor.h`
- `plugins/[PluginName]/Source/PluginProcessor.cpp`
- `plugins/[PluginName]/Source/PluginEditor.h`
- `plugins/[PluginName]/Source/PluginEditor.cpp`

## Required Reading

Read `juce8-critical-patterns.md` before writing any code. Key points:

1. `juce_generate_juce_header` after `target_link_libraries`
2. APVTS member declared after all DSP members in header
3. Parameters: bare string IDs are fine; `juce::ParameterID { "id", 1 }` also valid
4. Real-time reads: `getRawParameterValue("id")->load()` only
5. `FORMATS VST3` only (no Standalone, no AU)
6. `JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0`, `JUCE_DISPLAY_SPLASH_SCREEN=0`

## Task

### 1. Extract from contracts

Print to confirm before writing code:

```
Plugin: [PluginName]
Product Name: [ProductName]
Type: Effect / Instrument
Parameters:
  - [id] (Float, min–max, default)
  - [id] (Bool, default)
DSP Components (declared before apvts):
  - [ComponentName]
```

### 2. Generate CMakeLists.txt

Based on `plugins/[PluginName]/CMakeLists.txt` (already created by /new-plugin).
Verify it has:
- Correct plugin name and codes
- `FORMATS VST3` only
- All required link libraries
- `juce_generate_juce_header` after `target_link_libraries`
- Required compile definitions

If DSP components exist, add their `.cpp` files to `target_sources`.

### 3. Generate PluginProcessor.h

```cpp
#pragma once
#include <JuceHeader.h>
// DSP component includes here

class [PluginName]AudioProcessor : public juce::AudioProcessor {
public:
    [PluginName]AudioProcessor();
    ~[PluginName]AudioProcessor() override;
    // ... standard overrides ...
    juce::AudioProcessorValueTreeState apvts;
private:
    // DSP members declared BEFORE apvts if they appear in apvts init
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR([PluginName]AudioProcessor)
};
```

### 4. Generate PluginProcessor.cpp

Implement `createParameterLayout()` with **all parameters** from parameter-spec.md.

Float example:
```cpp
layout.add(std::make_unique<juce::AudioParameterFloat>(
    "gain", "Gain",
    juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),
    0.0f,
    juce::AudioParameterFloatAttributes().withStringFromValueFunction(
        [](float v, int) -> juce::String { return juce::String(v, 1) + " dB"; })));
```

Bool example:
```cpp
layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
```

Choice example:
```cpp
layout.add(std::make_unique<juce::AudioParameterChoice>(
    "mode", "Mode", juce::StringArray { "A", "B", "C" }, 0));
```

Leave `processBlock()` as pass-through (DSP implemented by dsp-agent).

### 5. Self-validate

After writing, re-read PluginProcessor.cpp and extract all parameter IDs with regex:
```
AudioParameter(?:Float|Bool|Choice)\s*\(\s*"(\w+)"
```

Compare against parameter-spec.md. Every spec ID must appear in code. Zero extras.

If mismatch: fix and re-validate before returning.

### 6. Return report

```
foundation-agent complete
Plugin: [PluginName]
Parameters implemented: [N]
  [list of IDs]
Files written:
  plugins/[PluginName]/CMakeLists.txt (verified)
  plugins/[PluginName]/Source/PluginProcessor.h
  plugins/[PluginName]/Source/PluginProcessor.cpp
Issues: none | [description]
Ready for: dsp-agent
```

## Success Criteria

- All parameters from spec implemented — zero drift
- APVTS member declared correctly (after DSP members)
- `createPluginFilter()` factory function present
- State management implemented (getStateInformation / setStateInformation)
- CMakeLists.txt: VST3 only, correct link libraries, correct header generation order
