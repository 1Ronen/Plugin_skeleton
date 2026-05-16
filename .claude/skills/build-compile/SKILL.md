---
name: build-compile
description: >
  Build the active plugin with CMake/MSVC on Windows. Handles configure + build,
  reports VST3 output path, and surfaces errors clearly. Use when the user says
  "build", "compile", or after source changes that require a rebuild.
allowed-tools:
  - Bash
  - Read
---

# build-compile

Builds the plugin currently active in root `CMakeLists.txt`.

## Workflow

### 1. Verify prerequisites

Check that the root `CMakeLists.txt` has an `add_subdirectory(plugins/...)` line:

```powershell
Select-String -Path CMakeLists.txt -Pattern "add_subdirectory\(plugins"
```

If missing, tell the user which plugin to activate and stop.

### 2. Configure

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
```

If configuration fails:
- Show the CMake error (last 30 lines of stderr)
- Common causes: JUCE path wrong, missing `add_subdirectory`, syntax error in CMakeLists.txt
- Do not proceed to build step

### 3. Build Release

```powershell
cmake --build build --config Release --parallel
```

Capture output. On failure:
1. Show the compiler error (file, line, message)
2. Identify the cause (missing header, type mismatch, linker error)
3. Fix the source file if the error is clear; otherwise report it to the user

### 4. Locate and report VST3

```powershell
Get-ChildItem -Path build -Recurse -Filter "*.vst3" -Directory | Select-Object FullName
```

Report the full path. Typical location:

```
build\plugins\[PluginName]\[PluginName]_artefacts\Release\VST3\[ProductName].vst3
```

### 5. Optional install

If the user says "install" or "copy to VST3 folder", copy with:

```powershell
$dest = "C:\Program Files\Common Files\VST3"
Copy-Item -Path $vst3Path -Destination $dest -Recurse -Force
Write-Host "Installed to $dest"
```

Requires admin rights — warn if likely to fail.

## Error Reference

| Symptom | Likely Cause |
|---------|-------------|
| `JuceHeader.h: No such file` | `juce_generate_juce_header` before `target_link_libraries` |
| `createPluginFilter` unresolved | Missing factory function in PluginProcessor.cpp |
| CRT mismatch in DAW | Missing `CMAKE_MSVC_RUNTIME_LIBRARY` in root CMakeLists.txt |
| `ParameterID not a member of juce` | JUCE version < 8 or wrong include |

See `juce8-critical-patterns.md` for full reference.

## Retry on Failure

Fix the error and re-run only the build step (skip configure if CMakeLists.txt unchanged):

```powershell
cmake --build build --config Release --parallel
```
