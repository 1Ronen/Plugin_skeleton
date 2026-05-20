# JUCE 8 Critical Patterns — Windows / MSVC

Non-negotiable patterns. Deviating causes silent failures, build errors, or DAW crashes.

---

## CMake

### `juce_generate_juce_header` placement

Call it **after** `target_link_libraries`, not before.

```cmake
# CORRECT
target_link_libraries(MyPlugin PRIVATE juce::juce_audio_utils ...)
juce_generate_juce_header(MyPlugin)

# WRONG — JuceHeader.h may not resolve module paths
juce_generate_juce_header(MyPlugin)
target_link_libraries(MyPlugin ...)
```

### VST3 only on Windows

```cmake
FORMATS VST3   # no Standalone (entry point conflict), no AU (macOS only)
```

### Required compile definitions

```cmake
target_compile_definitions(MyPlugin PUBLIC
    JUCE_WEB_BROWSER=0        # native GUI, no browser component
    JUCE_USE_CURL=0           # no libcurl on Windows
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
```

### MSVC runtime library

Set in root CMakeLists.txt **before** `add_subdirectory(JUCE)`:

```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
```

This prevents CRT mismatch crashes when loading the VST3 in a DAW.

### LTO on MSVC

`juce_recommended_lto_flags` enables `/GL` + `/LTCG`. Link time increases significantly for large projects. Safe to include; remove from Release config if link time is unacceptable.

### Path separators

Use forward slashes in CMake paths, even on Windows:

```cmake
add_subdirectory("D:/HISE_Dev/JUCE" JUCE)   # correct
add_subdirectory("D:\\HISE_Dev\\JUCE" JUCE)  # also works but avoid mixing
```

---

## C++ / JUCE API

### Include style

Use the generated header (requires `juce_generate_juce_header` in CMake):

```cpp
#include <JuceHeader.h>
```

Or individual module headers (no CMake requirement):

```cpp
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
```

Do not `#include` JuceHeader.h from the JUCE source tree directly.

### APVTS member declaration order

`apvts` must be declared **after** all DSP members it initializes in the member init list.
Init order in the constructor follows declaration order in the header.

```cpp
// Header — declare apvts last
class MyProcessor : public juce::AudioProcessor {
    MyDspComponent dsp;           // declared first
    juce::AudioProcessorValueTreeState apvts;  // declared after
};

// .cpp — init apvts after AudioProcessor base
MyProcessor::MyProcessor()
    : AudioProcessor(BusesProperties()...),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{}
```

### Parameter ID format

Both formats compile in JUCE 8. Use `juce::ParameterID` for preset-migration support:

```cpp
// Preferred (JUCE 8, supports versioned migration)
layout.add(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID { "gain", 1 }, "Gain", ...));

// Also valid (simpler, matches JUCE 7 style)
layout.add(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", ...));
```

### Real-time safe parameter reading

```cpp
// CORRECT — atomic read, lock-free
float gain = apvts.getRawParameterValue("gain")->load();

// WRONG — acquires a lock internally, not real-time safe
float gain = apvts.getParameter("gain")->getValue();
```

### `ScopedNoDenormals`

Always the first line of `processBlock`:

```cpp
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    // ...
}
```

### State management (preset save/load)

```cpp
void getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
```

### `isBusesLayoutSupported` — always override for audio effects

Without this override, JUCE's default only accepts layouts that exactly match the
configured buses. Ableton's VST3 host negotiates multiple layout candidates at load time;
if none match, the plugin silently enters bypass mode — audio passes through, knobs do nothing.

```cpp
// PluginProcessor.h
bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

// PluginProcessor.cpp
bool MyProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;

    return in == out;   // require matching channel count
}
```

This accepts mono-in/mono-out and stereo-in/stereo-out — the two standard Ableton configurations.

### Cache `getRawParameterValue` pointer in `prepareToPlay`

`getRawParameterValue` does a hash-map lookup. Cache the `std::atomic<float>*` once
in `prepareToPlay` and read it in `processBlock`:

```cpp
// Header
std::atomic<float>* gainParam { nullptr };

// prepareToPlay
gainParam = apvts.getRawParameterValue("gain");

// processBlock
const float gainDb = gainParam->load();
```

### `juce::Font` — use `FontOptions` in JUCE 8

The two-argument `Font(size, style)` constructor is deprecated in JUCE 8:

```cpp
// DEPRECATED — C4996 warning
juce::Font(18.0f, juce::Font::bold)

// CORRECT — JUCE 8
juce::Font(juce::FontOptions().withHeight(18.0f).withStyle("Bold"))
juce::Font(juce::FontOptions().withHeight(11.0f))   // regular weight
```

Applies everywhere: `g.setFont(...)`, `label.setFont(...)`, `LookAndFeel` overrides.

### Editor destructor

Always call `setLookAndFeel(nullptr)` and `stopTimer()` before destruction:

```cpp
MyEditor::~MyEditor()
{
    stopTimer();           // if using Timer
    setLookAndFeel(nullptr);
}
```

---

## Build & Output

### Build configuration

Always validate VST3 loading with a **Release** build.  
Debug VST3s link the debug CRT — most DAWs ship with Release CRT and will refuse to load.

### VST3 output location

After `cmake --build build --config Release`:

```
build\plugins\[PluginName]\[PluginName]_artefacts\Release\VST3\[ProductName].vst3
```

The `.vst3` on Windows is a **directory** (same as macOS bundle), not a single file.

### System-wide VST3 install (for DAW scanning)

Copy the `.vst3` directory to:

```
C:\Program Files\Common Files\VST3\
```

Or use xcopy/robocopy in the build script.

### PLUGIN_CODE uniqueness

The 4-character `PLUGIN_CODE` must be unique across all plugins. Collisions cause DAWs to identify two plugins as the same and may load the wrong one.

---

## Common MSVC Errors

| Error | Cause | Fix |
|---|---|---|
| `LNK2019: unresolved external symbol createPluginFilter` | Missing `juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()` in PluginProcessor.cpp | Add the factory function |
| `error C2039: 'ParameterID': not a member of 'juce'` | Old JUCE version (<8) | Update JUCE or use bare string ID |
| CRT mismatch crash on DAW load | Runtime library mismatch | Set `CMAKE_MSVC_RUNTIME_LIBRARY` in root CMakeLists.txt |
| `JuceHeader.h: No such file` | `juce_generate_juce_header` not called or called before `target_link_libraries` | Fix CMakeLists.txt order |
| Splash screen at startup | `JUCE_DISPLAY_SPLASH_SCREEN` not set | Add to `target_compile_definitions` |

---

## Studio Branding Patterns

### LookAndFeel setup
- Always create a custom class inheriting from `LookAndFeel_V4`
- Override in PluginEditor constructor: `setLookAndFeel(&customLookAndFeel)`
- Destructor: `setLookAndFeel(nullptr)` — never skip this, causes crash on close
- All ColourId overrides go in the custom LookAndFeel constructor
- Never call `setColour()` directly on components — use LookAndFeel only

### Logo rendering
- Load once in PluginEditor constructor:
  `logoImage = ImageCache::getFromFile(File(logoPath))`
- Render in `paint()` after all base painting:
  ```cpp
  g.drawImageWithin(logoImage, x, y, w, h,
      RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize);
  ```
- Always check `logoImage.isValid()` before drawing — missing file = crash
- Logo path must be absolute or resolved relative to plugin binary

### Knob double-ring pattern
- Outer ring: `drawEllipse` with Background secondary fill, Border stroke
- Inner ring: `drawEllipse` with Background primary fill, Accent stroke
- Pointer line: `drawLine` from center outward, Accent color, 2px stroke,
  rounded cap, rotated by normalised value mapped to -135° to +135°
- Value label: drawn below knob rect, centered, Text primary, 10px bold

### Common branding mistakes
- Forgetting `setLookAndFeel(nullptr)` in destructor → crash on plugin close
- Using `ImageCache::getFromFile` with relative path → not found in DAW context
- Drawing logo before background is painted → logo gets overdrawn
- Hardcoding colors instead of reading from LookAndFeel → breaks identity system
