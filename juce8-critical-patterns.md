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

### Configurable JUCE path

Never hardcode the JUCE path. Use a cache variable with a local fallback so CI and
other machines can override without editing the file:

```cmake
if(NOT DEFINED JUCE_PATH)
    set(JUCE_PATH "D:/HISE_Dev/JUCE")
endif()
add_subdirectory(${JUCE_PATH} JUCE)
```

On CI, pass the override: `cmake -B build -DJUCE_PATH=/path/to/JUCE ...`

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

## NSIS Installer Patterns

### Installer directory selection (mandatory)

Always include `MUI_PAGE_DIRECTORY` — users must be able to change the install path.

```nsis
; Set directory page text immediately before the macro, undef after
!define MUI_PAGE_HEADER_TEXT    "Choose Install Location"
!define MUI_PAGE_HEADER_SUBTEXT "Choose where PRODUCT_NAME VST3 will be installed"
!define MUI_DIRECTORYPAGE_TEXT_TOP \
    "Install to your DAW's VST3 scan folder.$\nThe default location is recognized by most DAWs automatically."
!insertmacro MUI_PAGE_DIRECTORY
; Note: MUI2 consumes and cleans up MUI_PAGE_HEADER_* and MUI_DIRECTORYPAGE_* defines
; internally — do not !undef them, it causes "not defined" warnings
```

Page order must always be: `Welcome → License → Directory → Install → Finish`

### Default install path — use $COMMONFILES64

`$COMMONFILES` on 64-bit Windows resolves to `C:\Program Files (x86)\Common Files` (the WOW64 path).
VST3 plugins must go to `C:\Program Files\Common Files\VST3\` which requires `$COMMONFILES64`.

```nsis
; CORRECT
InstallDir "$COMMONFILES64\VST3\Orient Plugins"

; WRONG — installs to x86 path, DAWs won't scan it
InstallDir "$COMMONFILES\VST3\Orient Plugins"
```

### Uninstaller must use $INSTDIR

The user may change the install directory on the Directory page. Hardcoding the removal path
causes broken uninstalls when the user chose a non-default location.

```nsis
; CORRECT — follows user-selected path
Section "VST3 Plugin" SEC_VST3
    SetOutPath "$INSTDIR"
    File /r "path\to\Plugin.vst3"
    WriteUninstaller "$INSTDIR\Uninstall Plugin.exe"
    WriteRegStr HKLM "...\Uninstall\..." "UninstallString" "$INSTDIR\Uninstall Plugin.exe"
    WriteRegStr HKLM "...\Uninstall\..." "InstallLocation" "$INSTDIR"
SectionEnd

Section "Uninstall"
    RMDir /r "$INSTDIR\Plugin.vst3"
    Delete   "$INSTDIR\Uninstall Plugin.exe"
    RMDir    "$INSTDIR"
    DeleteRegKey HKLM "...\Uninstall\..."
SectionEnd

; WRONG — breaks if user changed directory
WriteUninstaller "$COMMONFILES64\VST3\Orient Plugins\Uninstall Plugin.exe"
RMDir /r "$COMMONFILES64\VST3\Orient Plugins\Plugin.vst3"
```

### VST3 is a directory, not a file

On Windows the `.vst3` artefact is a folder (bundle). Use `File /r` to copy it recursively.

```nsis
; CORRECT
File /r "build\...\Release\VST3\Plugin.vst3"

; WRONG — File without /r silently copies nothing for a directory
File "build\...\Release\VST3\Plugin.vst3"
```

### Installer version control

Always build into a versioned subfolder — never into the root `installer/` directory:

```
installer\
  v1.0.0\
    AtmoKick_Setup_v1.0.0.exe
    AtmoKick_v1.0.0.nsi
    eula.txt
    install-log-v1.0.0.md
  v1.1.0\
    AtmoKick_Setup_v1.1.0.exe
    AtmoKick_v1.1.0.nsi
    eula.txt
    install-log-v1.1.0.md
```

- **Filename format**: `[PluginName]_Setup_v[X.X.X].exe` — version always in filename
- **OutFile in .nsi**: `OutFile "PluginName_Setup_vX.X.X.exe"` — never bare `PluginName_Setup.exe`
- **Old versions kept permanently** — never delete previous subfolders; rollback capability requires all versions present
- **Never overwrite** — if `v[Version]\PluginName_Setup_v[Version].exe` already exists, stop and ask user to bump version

### Common NSIS mistakes

| Mistake | Consequence | Fix |
|---------|-------------|-----|
| `$COMMONFILES` instead of `$COMMONFILES64` | Installs to x86 path, DAW won't find plugin | Use `$COMMONFILES64` |
| Hardcoded path in `Section "Uninstall"` | Broken uninstall if user changed directory | Use `$INSTDIR` everywhere |
| Missing `MUI_PAGE_DIRECTORY` | User cannot change install path | Add between License and Install pages |
| `File` without `/r` for VST3 bundle | Plugin bundle not copied | Use `File /r` |
| `!undef` missing after page defines | Header text bleeds into next page | Always undef after each page macro |
| Installer built into root `installer/` | Overwrites previous version, no rollback | Always use `installer\v[X.X.X]\` subfolder |
| Version absent from OutFile name | Can't distinguish versions on disk | Always include version in filename |

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

### Slider textbox colours — all 4, directly on each Slider

LookAndFeel ColourId overrides do **not** propagate to Slider textboxes. Set them directly
on every Slider instance, typically inside a `setupKnob()` helper:

```cpp
knob.setColour(juce::Slider::textBoxTextColourId,       juce::Colour(0xff1C2035));
knob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xffECEEF3));
knob.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
knob.setColour(juce::Slider::textBoxHighlightColourId,  juce::Colour(0xff5B7BF8));
```

### TextEditor edit-state colours

When the user clicks a slider value to type a new one, JUCE activates an inline
`TextEditor`. Without these LookAndFeel overrides the edit box uses default colours
(white background, black text) — visible as a jarring flash on branded dark UIs.

Set all of these in the LookAndFeel constructor:

```cpp
setColour(juce::Label::textWhenEditingColourId,        juce::Colour(0xff1C2035));
setColour(juce::Label::backgroundWhenEditingColourId,  juce::Colour(0xffECEEF3));
setColour(juce::Label::outlineWhenEditingColourId,     juce::Colour(0xff5B7BF8));
setColour(juce::TextEditor::backgroundColourId,        juce::Colour(0xffECEEF3));
setColour(juce::TextEditor::textColourId,              juce::Colour(0xff1C2035));
setColour(juce::TextEditor::outlineColourId,           juce::Colour(0xff5B7BF8));
setColour(juce::CaretComponent::caretColourId,         juce::Colour(0xff5B7BF8));
```

Note: use `juce::CaretComponent::caretColourId` — `juce::TextEditor::caretColourId` does
not exist in JUCE 8 and causes a C2039 compile error.

### Common branding mistakes
- Forgetting `setLookAndFeel(nullptr)` in destructor → crash on plugin close
- Using `ImageCache::getFromFile` with relative path → not found in DAW context
- Drawing logo before background is painted → logo gets overdrawn
- Hardcoding colors instead of reading from LookAndFeel → breaks identity system
- **Slider textbox colours must be set directly on each `Slider` instance** — `LookAndFeel` alone does not override them. Call `slider.setColour(Slider::textBoxTextColourId, ...)` etc. inside `setupKnob()` immediately after construction, for every slider.
- **Logo path must use `File::getSpecialLocation(currentApplicationFile)` as base** — relative paths fail in DAW plugin context because the working directory is the DAW's, not the plugin's. Resolve `Assets/logo.png` relative to this file, then fall back to embedded `BinaryData` if the file is not found.
- **Always add a `POST_BUILD` copy command in `CMakeLists.txt`** to copy the `Assets/` folder into the VST3 bundle alongside the DLL. Target must be `[PluginName]_VST3`, not `[PluginName]` (the latter is SharedCode.lib, not the DLL):
  ```cmake
  add_custom_command(TARGET MyPlugin_VST3 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/Source/Assets"
      "$<TARGET_FILE_DIR:MyPlugin_VST3>/Assets")
  ```
- **POST_BUILD doesn't activate on first add** — after adding `add_custom_command` to CMakeLists.txt, always run `cmake -B build ...` (reconfigure) before rebuilding, otherwise the new command is not picked up.
- **`setLookAndFeel(nullptr)` must be the first line of the destructor** — not after attachment teardown. Order matters; skipping causes crash on plugin close in some DAWs.

---

## Audio QA Tools

### Plugin Doctor — effects only

Plugin Doctor can only analyze audio **effects** (FX plugins). It requires audio input to
measure noise floor, THD, aliasing, and CPU. It has no MIDI input and cannot trigger
instrument plugins (synths, samplers, drum machines).

**Do not use Plugin Doctor for instrument plugins.** It will not produce valid results.
For instruments, QA is limited to pluginval + manual listening tests in a DAW.

---

## Git Push Policy

- **All pushes go through backup skill only** — never run `git push` manually during active development
- **backup skill reads `qa-report.md` before pushing** — the gate check is the first thing it does
- **`qa-report.md` Status: PASS required** — no exceptions; FAIL or missing file blocks the push entirely
- **Failed QA = blocked push = debug-agent fires first** — fix the failure, re-run qa-tester, then backup triggers automatically
- Pattern: `qa-tester PASS → auto-trigger backup → version-check → push both repos`
- Both repos (Plugins + Skeleton) always pushed together — never one without the other

### Installer Build Gate

- **Installer built ONLY when version number changes** — not on every QA pass
- **Version tracked in `installer\installer-version.txt`** — one file per plugin, lives alongside the installer folders
- **version-check skill** compares `creative-brief.md` current version vs `installer-version.txt` last built version
- **Same version = source push only** — no installer built, commit message: `fix: [PluginName] v[version] — [date]`
- **New version = installer built first, then pushed** — commit message: `release: [PluginName] v[version] — installer included`
- **`installer-version.txt` updated only after installer Gate OUT confirmed** — never speculatively
- Never build installer on every QA pass — repo size and commit noise
