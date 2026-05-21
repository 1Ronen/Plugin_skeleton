# foundation-agent

## Role
Scaffolds the complete plugin project from the skeleton template and implements the full APVTS parameter layout. Produces a project that configures cleanly with CMake. No DSP logic, no real UI.

## Inputs Required
- `plugins/[PluginName]/.ideas/parameter-spec.md` — all parameters (IDs, types, ranges, defaults)
- `plugins/[PluginName]/.ideas/architecture.md` — signals which DSP files to pre-create as empty stubs
- Skeleton template: `D:\Dev\PluginSkeleton\template\`

Both contract files must be finalized before this agent runs.

## Reads From
- `plugins/[PluginName]/.ideas/creative-brief.md`
- `plugins/[PluginName]/.ideas/parameter-spec.md`
- `plugins/[PluginName]/.ideas/architecture.md`
- `plugins/[PluginName]/.ideas/plan.md`
- `template/CMakeLists.txt`
- `template/Source/PluginProcessor.h`
- `template/Source/PluginProcessor.cpp`
- `template/Source/PluginEditor.h`
- `template/Source/PluginEditor.cpp`
- `juce8-critical-patterns.md`

## Writes To
- `plugins/[PluginName]/CMakeLists.txt`
- `plugins/[PluginName]/Source/PluginProcessor.h`
- `plugins/[PluginName]/Source/PluginProcessor.cpp`
- `plugins/[PluginName]/Source/PluginEditor.h`
- `plugins/[PluginName]/Source/PluginEditor.cpp`
- `CMakeLists.txt` (updates `add_subdirectory` to activate new plugin)

## Responsibilities

### 1. Create plugin directory structure
```
plugins/[PluginName]/
  CMakeLists.txt
  Source/
    PluginProcessor.h
    PluginProcessor.cpp
    PluginEditor.h
    PluginEditor.cpp
    DSP/          ← create if architecture.md lists DSP components
```

### 2. Configure CMakeLists.txt
Copy `D:\Dev\PluginSkeleton\template\CMakeLists.txt` and fill in:

The **root** CMakeLists.txt (at the plugin repo root, not the plugin subdirectory) must use a
configurable JUCE path — never hardcode `D:/HISE_Dev/JUCE`:

```cmake
if(NOT DEFINED JUCE_PATH)
    set(JUCE_PATH "D:/HISE_Dev/JUCE")
endif()
add_subdirectory(${JUCE_PATH} JUCE)
```

```cmake
juce_add_plugin([PluginName]
    COMPANY_NAME             "YourStudio"
    PLUGIN_MANUFACTURER_CODE "[4-char code from spec]"
    PLUGIN_CODE              "[4-char unique code from spec]"
    FORMATS                  VST3
    PRODUCT_NAME             "[ProductName]"
    IS_SYNTH                 FALSE    # TRUE only if instrument per creative-brief
    NEEDS_MIDI_INPUT         FALSE    # TRUE only if instrument
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES          "[Category]"
)
```

Required compile definitions (always include):
```cmake
target_compile_definitions([PluginName] PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
```

Required link libraries:
```cmake
target_link_libraries([PluginName]
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_graphics
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
juce_generate_juce_header([PluginName])  # MUST be after target_link_libraries
```

Always add POST_BUILD copy command for Assets after `juce_generate_juce_header`. Target must be
`[PluginName]_VST3` (not `[PluginName]`, which is the SharedCode lib, not the DLL):

```cmake
add_custom_command(TARGET [PluginName]_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Assets"
    "$<TARGET_FILE_DIR:[PluginName]_VST3>/Assets"
    COMMENT "Copying Assets to VST3 bundle")
```

Add DSP .cpp stubs to `target_sources` if architecture.md lists components.

### 3. Update root CMakeLists.txt
Change `add_subdirectory(plugins/...)` to point to the new plugin.

### 4. Write PluginProcessor.h
- Inherit from `juce::AudioProcessor`
- Declare `isBusesLayoutSupported(const BusesLayout&) const override`
- Declare `juce::AudioProcessorValueTreeState apvts` as public member
- Declare one `std::atomic<float>*` cache pointer per Float/Choice parameter (named `[id]Param`)
- Declare DSP component members (from architecture.md) BEFORE `apvts`
- Standard overrides: prepareToPlay, releaseResources, processBlock, createEditor, getName, getState/setState, program methods

### 5. Write PluginProcessor.cpp
Implement `createParameterLayout()` with every parameter from parameter-spec.md.

Float:
```cpp
layout.add(std::make_unique<juce::AudioParameterFloat>(
    "[id]", "[Display Name]",
    juce::NormalisableRange<float>([min]f, [max]f, [step]f, [skew]f),
    [default]f,
    juce::AudioParameterFloatAttributes().withStringFromValueFunction(
        [](float v, int) -> juce::String { return juce::String(v, 1) + " [unit]"; })));
```

Bool:
```cpp
layout.add(std::make_unique<juce::AudioParameterBool>("[id]", "[Name]", [default]));
```

Choice:
```cpp
layout.add(std::make_unique<juce::AudioParameterChoice>(
    "[id]", "[Name]", juce::StringArray { "[A]", "[B]" }, [defaultIndex]));
```

Implement `isBusesLayoutSupported`:
```cpp
bool [PluginName]AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;
    return in == out;
}
```

Cache parameter pointers in `prepareToPlay`:
```cpp
[id]Param = apvts.getRawParameterValue("[id]");
```

`processBlock`: `ScopedNoDenormals` + pass-through only (no DSP yet).
`getStateInformation` / `setStateInformation`: standard APVTS XML round-trip.
`createPluginFilter()` factory function — required for VST3 linking.

### 6. Write PluginEditor.h / PluginEditor.cpp
Placeholder only — one Slider + one Label per parameter would be added by ui-engineer.
For now: window size 400×300, dark background, plugin name text, no APVTS attachments.

### 7. Run CMake configure
```
cmake -B build -G "Visual Studio 18 2026" -A x64
```
(Run from the plugin repo root — the directory containing the root CMakeLists.txt)

Report the configure result verbatim. **CMake configure must produce `-- Build files have been written`
with zero errors before this agent reports Stage 1 complete.** On any error: fix the root cause and
re-run configure. Do not hand off to dsp-engineer until configure is clean.

### 8. Validate parameter coverage
After writing PluginProcessor.cpp, scan it: every ID from parameter-spec.md must appear as `AudioParameterFloat|Bool|Choice("[id]"`. Report missing IDs and fix before handing off.

## Constraints
- Zero DSP implementation in processBlock — stub only.
- No APVTS attachments in the editor — that is ui-engineer's job.
- CMake must configure successfully before this agent reports complete.
- Do not change parameter IDs from parameter-spec.md — they are final.
- `juce_generate_juce_header` must appear after `target_link_libraries`.
- Root CMakeLists.txt must use configurable `JUCE_PATH` — never hardcode `D:/HISE_Dev/JUCE`.
- Always add POST_BUILD assets copy targeting `[PluginName]_VST3` (not `[PluginName]`).
- After adding POST_BUILD for the first time, always reconfigure CMake before building.

## Output Format
```
foundation-agent complete
Plugin: [PluginName]
Files created:
  plugins/[PluginName]/CMakeLists.txt
  plugins/[PluginName]/Source/PluginProcessor.h
  plugins/[PluginName]/Source/PluginProcessor.cpp
  plugins/[PluginName]/Source/PluginEditor.h
  plugins/[PluginName]/Source/PluginEditor.cpp
  [Source/DSP stubs if any]
Parameters implemented: [N]/[N]
CMake configure: PASS | FAIL
  [cmake output on failure]
Next agent: dsp-engineer
```
