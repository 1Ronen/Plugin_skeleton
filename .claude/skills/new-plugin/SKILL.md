---
name: new-plugin
description: >
  Instantiate the template as a new named plugin. Copies template/ to plugins/[Name]/,
  replaces all placeholders, and updates root CMakeLists.txt. Use when the user says
  "create a new plugin", "new plugin called X", or "instantiate the template".
allowed-tools:
  - Read
  - Write
  - Edit
  - Bash
---

# new-plugin

Creates a new plugin from the skeleton template.

## Inputs Required

Before proceeding, collect from the user or from `.ideas/` contracts:

| Field | Example |
|-------|---------|
| `[PluginName]` | `SimpleDelay` |
| `[ProductName]` | `Simple Delay` |
| `[PluginCode]` | `SdLy` (4 chars, unique) |
| `[Category]` | `Fx` or `Fx|Delay` |

If `.ideas/creative-brief.md` and `.ideas/parameter-spec.md` are filled in, read them
to extract these values. Otherwise ask the user.

## Workflow

### 1. Validate inputs

- `[PluginName]`: CamelCase, no spaces, no special chars — valid C++ identifier
- `[PluginCode]`: exactly 4 chars
- Check that `plugins/[PluginName]/` does not already exist

### 2. Create plugin directory

```powershell
New-Item -ItemType Directory -Path "plugins\[PluginName]\Source" -Force
```

### 3. Copy and fill template files

Copy each template file, replacing all placeholders:

| Template file | Destination |
|--------------|-------------|
| `template\CMakeLists.txt` | `plugins\[PluginName]\CMakeLists.txt` |
| `template\Source\PluginProcessor.h` | `plugins\[PluginName]\Source\PluginProcessor.h` |
| `template\Source\PluginProcessor.cpp` | `plugins\[PluginName]\Source\PluginProcessor.cpp` |
| `template\Source\PluginEditor.h` | `plugins\[PluginName]\Source\PluginEditor.h` |
| `template\Source\PluginEditor.cpp` | `plugins\[PluginName]\Source\PluginEditor.cpp` |

Replacements (case-sensitive):

```
[PluginName]   → e.g. SimpleDelay
[ProductName]  → e.g. Simple Delay
[PluginCode]   → e.g. SdLy
[Category]     → e.g. Fx
```

### 4. Copy .ideas contracts (if filled in)

```powershell
New-Item -ItemType Directory -Path "plugins\[PluginName]\.ideas" -Force
Copy-Item ".ideas\creative-brief.md"  "plugins\[PluginName]\.ideas\"
Copy-Item ".ideas\parameter-spec.md"  "plugins\[PluginName]\.ideas\"
Copy-Item ".ideas\architecture.md"    "plugins\[PluginName]\.ideas\"
Copy-Item ".ideas\plan.md"            "plugins\[PluginName]\.ideas\"
```

If the contracts are already filled in, they become the plugin's contracts.
If they are blank templates, the user fills them in later.

### 5. Update root CMakeLists.txt

Change the `add_subdirectory` line to point to the new plugin:

```cmake
# Before:
add_subdirectory(plugins/SimpleGain)

# After:
add_subdirectory(plugins/[PluginName])
```

### 6. Confirm and next steps

Report:

```
Created: plugins/[PluginName]/
  CMakeLists.txt
  Source/PluginProcessor.h
  Source/PluginProcessor.cpp
  Source/PluginEditor.h
  Source/PluginEditor.cpp

Root CMakeLists.txt updated → active plugin: [PluginName]

Next steps:
  1. Fill in .ideas/parameter-spec.md with your parameters
  2. Use /plugin-ideation to refine the concept
  3. Use foundation-agent to implement APVTS from parameter-spec
  4. Run /build-compile to verify
```

## Notes

- Do not add parameters to PluginProcessor yet — wait for parameter-spec.md to be finalized
- Do not modify the template files themselves — always work in `plugins/[PluginName]/`
- The placeholder knob in PluginEditor is intentionally disconnected (no APVTS attachment) until parameters are defined
