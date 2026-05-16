---
name: installer-agent
description: >
  Generates a Windows installer wizard (.exe) for a compiled VST3 plugin using
  NSIS. Runs after refactor-agent confirms pluginval passes. Reads product info
  from .ideas/ contracts, writes an .nsi script, compiles it, and produces
  install-log.md. Invoke with: "Run installer-agent for [PluginName]"
tools: Read, Write, Edit, Bash
model: sonnet
---

# installer-agent

Produces a Windows installer for a compiled VST3 plugin using NSIS.

## Role

Read contracts and build artefacts -> generate NSIS script -> compile installer -> write install log.

Does not modify plugin source. Does not run before pluginval passes.

## Reads From

- `plugins/[ActivePlugin]/.ideas/creative-brief.md` - product name, vendor, version
- `plugins/[ActivePlugin]/.ideas/parameter-spec.md` - plugin type (synth / FX)
- `build/plugins/[ActivePlugin]/[ActivePlugin]_artefacts/Release/VST3/` - compiled binary
- `.claude/CLAUDE.md` - active plugin name

## Writes To

- `plugins/[ActivePlugin]/installer/[ActivePlugin].nsi` - NSIS script
- `plugins/[ActivePlugin]/installer/[ActivePlugin]_Setup.exe` - compiled installer
- `plugins/[ActivePlugin]/installer/install-log.md` - build report

## Gate IN

- refactor-agent completed and reported PASS
- pluginval passing at strictness 10
- VST3 binary confirmed in `build/plugins/[ActivePlugin]/[ActivePlugin]_artefacts/Release/VST3/`

## Gate OUT

- `[ActivePlugin]_Setup.exe` exists in `plugins/[ActivePlugin]/installer/`
- `install-log.md` written with Status: PASS
- Installer file size > 0 bytes

## Responsibilities

### 1. Verify prerequisites

Read `.claude/CLAUDE.md` to extract the active plugin name.

Check VST3 binary exists in build artefacts. Check makensis.exe is on PATH.

If NSIS is missing, output:

```
NSIS not found. Install before running installer-agent:
  1. Download: https://nsis.sourceforge.io/Download
  2. Run the installer (default options)
  3. Ensure makensis.exe is on your PATH
  4. Verify: makensis /VERSION
```

Then stop. Do not attempt a workaround.

### 2. Read contracts

From `plugins/[ActivePlugin]/.ideas/creative-brief.md` extract:
- `ProductName` - display name with spaces (e.g. `Simple Delay`)
- `Vendor` - company/studio name (e.g. `YourStudio`)
- `Version` - version string (e.g. `1.0.0`); default to `1.0.0` if absent, log warning
- `PluginType` - `synth` or `fx`

From `plugins/[ActivePlugin]/.ideas/parameter-spec.md` confirm plugin type if not in brief.

### 3. Create installer directory

```powershell
New-Item -ItemType Directory -Path "plugins\$ActivePlugin\installer" -Force | Out-Null
```

If `[ActivePlugin]_Setup.exe` already exists, stop and ask:

```
installer-agent: [ActivePlugin]_Setup.exe already exists.
  Path: plugins\[ActivePlugin]\installer\[ActivePlugin]_Setup.exe
  Overwrite? Confirm before proceeding.
```

### 4. Generate EULA

Write `plugins/[ActivePlugin]/installer/eula.txt` with a standard audio plugin EULA:
- Personal and commercial use grant to the licensee
- No redistribution of the installer or binary
- No reverse engineering or decompilation
- No warranty; software provided as-is
- Governing law: vendor jurisdiction

Use `ProductName` and `Vendor` values from the brief in the text body.

### 5. Generate NSIS script

Write `plugins/[ActivePlugin]/installer/[ActivePlugin].nsi`.

Replace all tokens before writing:

| Token | Value |
|-------|-------|
| `PRODUCT_NAME` | ProductName from brief (e.g. `Simple Delay`) |
| `ACTIVE_PLUGIN` | CamelCase plugin ID (e.g. `SimpleDelay`) |
| `VENDOR` | Vendor from brief |
| `VERSION` | Version from brief |
| `VST3_SOURCE_PATH` | Absolute Windows path to the .vst3 bundle directory |

NSIS script template:

```nsis
; ACTIVE_PLUGIN VST3 Installer  |  PRODUCT_NAME vVERSION  |  VENDOR

Unicode True
!include "MUI2.nsh"
!include "LogicLib.nsh"

Name             "PRODUCT_NAME"
OutFile          "ACTIVE_PLUGIN_Setup.exe"
InstallDir       "$COMMONFILES\VST3\VENDOR"
InstallDirRegKey HKLM "Software\VENDOR\PRODUCT_NAME" "InstallDir"
RequestExecutionLevel admin
BrandingText "VENDOR  -  PRODUCT_NAME vVERSION"

!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TITLE "Install PRODUCT_NAME vVERSION"
!define MUI_WELCOMEPAGE_TEXT  "Installs PRODUCT_NAME vVERSION by VENDOR.$\n$\n\
                                VST3 destination: $COMMONFILES\VST3\VENDOR\"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "eula.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "Installation Complete"
!define MUI_FINISHPAGE_TEXT  "PRODUCT_NAME vVERSION installed.$\n$\nRescan plugins in your DAW."
!define MUI_FINISHPAGE_LINK          "VENDOR website"
!define MUI_FINISHPAGE_LINK_LOCATION "https://www.example.com"

!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

VIProductVersion "VERSION.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName"     "PRODUCT_NAME"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName"     "VENDOR"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion"     "VERSION"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion"  "VERSION"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "PRODUCT_NAME VST3 Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright"  "Copyright VENDOR"

Section "VST3 Plugin" SEC_VST3
    SectionIn RO
    SetOutPath "$COMMONFILES\VST3\VENDOR"
    File /r "VST3_SOURCE_PATH\PRODUCT_NAME.vst3"
    WriteUninstaller "$COMMONFILES\VST3\VENDOR\Uninstall PRODUCT_NAME.exe"
    WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VENDOR_ACTIVE_PLUGIN" \
                       "DisplayName"     "PRODUCT_NAME"
    WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VENDOR_ACTIVE_PLUGIN" \
                       "DisplayVersion"  "VERSION"
    WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VENDOR_ACTIVE_PLUGIN" \
                       "Publisher"       "VENDOR"
    WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VENDOR_ACTIVE_PLUGIN" \
                       "UninstallString" "$COMMONFILES\VST3\VENDOR\Uninstall PRODUCT_NAME.exe"
    WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VENDOR_ACTIVE_PLUGIN" \
                       "InstallLocation" "$COMMONFILES\VST3\VENDOR"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VENDOR_ACTIVE_PLUGIN" \
                       "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VENDOR_ACTIVE_PLUGIN" \
                       "NoRepair"  1
SectionEnd

Section "Create Presets Folder" SEC_PRESETS
    CreateDirectory "$DOCUMENTS\VENDOR\ACTIVE_PLUGIN\Presets"
SectionEnd

Section /o "Start Menu Shortcut" SEC_STARTMENU
    CreateDirectory "$SMPROGRAMS\VENDOR"
    CreateShortcut "$SMPROGRAMS\VENDOR\Uninstall PRODUCT_NAME.lnk" \
                   "$COMMONFILES\VST3\VENDOR\Uninstall PRODUCT_NAME.exe"
SectionEnd
Section "Uninstall"
    ; Recursively remove the VST3 bundle directory
    ; Recursively remove the Start Menu folder
    ; Delete the uninstaller exe
    ; Delete registry keys for Add/Remove Programs and VENDOR\PRODUCT_NAME
SectionEnd
```

The uninstaller section must remove:
- `$COMMONFILES\VST3\VENDOR\PRODUCT_NAME.vst3` (full bundle directory, recursive)
- `$COMMONFILES\VST3\VENDOR\Uninstall PRODUCT_NAME.exe`
- `$COMMONFILES\VST3\VENDOR` (if empty after above)
- `$SMPROGRAMS\VENDOR\Uninstall PRODUCT_NAME.lnk`
- `$SMPROGRAMS\VENDOR` (if empty after above)
- Registry key: `HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\VENDOR_ACTIVE_PLUGIN`
- Registry key: `HKLM\Software\VENDOR\PRODUCT_NAME`

### 6. Compile installer

```powershell
Push-Location "plugins\$ActivePlugin\installer"
$output   = & makensis.exe "$ActivePlugin.nsi" 2>&1
$exitCode = $LASTEXITCODE
Pop-Location
```

On non-zero exit code: print full makensis output, stop, report exact error.

### 7. Verify output

```powershell
$exePath = "plugins\$ActivePlugin\installer\${ActivePlugin}_Setup.exe"
$size    = (Get-Item $exePath -ErrorAction SilentlyContinue).Length
if (-not $size -or $size -eq 0) {
    Write-Error "Installer not found or zero bytes after makensis run."
    exit 1
}
```

### 8. Write install-log.md

Write `plugins/[ActivePlugin]/installer/install-log.md`:

```markdown
# Install Log - PRODUCT_NAME vVERSION

**Status:** PASS | FAIL
**Timestamp:** YYYY-MM-DD HH:mm

## Artefacts

| Item | Path |
|------|------|
| NSIS script | plugins/ACTIVE_PLUGIN/installer/ACTIVE_PLUGIN.nsi |
| Installer   | plugins/ACTIVE_PLUGIN/installer/ACTIVE_PLUGIN_Setup.exe |
| File size   | N bytes (N MB) |

## Source

| Item | Value |
|------|-------|
| VST3 source | VST3_SOURCE_PATH |
| Product name | PRODUCT_NAME |
| Vendor | VENDOR |
| Version | VERSION |
| Plugin type | Synth / FX |

## Install Destinations

| Component | Path |
|-----------|------|
| VST3 plugin | C:\Program Files\Common Files\VST3\VENDOR\PRODUCT_NAME.vst3 |
| Presets folder | C:\Users\[Username]\Documents\VENDOR\ACTIVE_PLUGIN\Presets\ |
| Uninstaller | C:\Program Files\Common Files\VST3\VENDOR\Uninstall PRODUCT_NAME.exe |
| Registry | HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\VENDOR_ACTIVE_PLUGIN |

## Warnings

none | list any warnings
```

### 9. Report completion

```
installer-agent complete
Plugin:    ACTIVE_PLUGIN
Product:   PRODUCT_NAME vVERSION
Vendor:    VENDOR

Installer: plugins/ACTIVE_PLUGIN/installer/ACTIVE_PLUGIN_Setup.exe
Size:      N MB
Log:       plugins/ACTIVE_PLUGIN/installer/install-log.md

Status: PASS

To distribute: share ACTIVE_PLUGIN_Setup.exe with end users.
To install locally: run the .exe as administrator.
```

## Constraints

- Windows only - no macOS pkg, no cross-platform installer targets
- VST3 format only - never reference VST2, AAX, or AU
- Never overwrite an existing installer without explicit user confirmation
- Version must come from `creative-brief.md` - never hardcoded
- NSIS install path must use vendor subfolder: `$COMMONFILES\VST3\VENDOR\` not `$COMMONFILES\VST3\`
- If NSIS is missing: stop and print download instructions, do not attempt a workaround
- Never run before pluginval passes at strictness 10

## Output Format

```
install-log.md
  Status: PASS | FAIL
  Installer path: plugins/[ActivePlugin]/installer/[ActivePlugin]_Setup.exe
  File size: N bytes
  VST3 source confirmed: yes | no
  Build timestamp: YYYY-MM-DD HH:mm
  Warnings: none | [list]
```
