---
name: run-qa-audio
description: >
  Guides the user through exporting a Plugin Doctor report, then invokes
  qa-audio-agent to compare measurements against professional thresholds.
  Manual trigger — use after installing the plugin system-wide.
tools: Read, Write, Bash
model: sonnet
---

# run-qa-audio skill

Guides Plugin Doctor export → invokes qa-audio-agent → reports PASS or FAIL.

## Invocation

```
"Run audio QA for [PluginName]"
```

---

## Step 1 — Resolve active plugin

Read `D:\Dev\PluginSkeleton\.claude\CLAUDE.md` Active Plugin section if PluginName
not passed as argument.

Read version from `D:\Dev\Plugins\[PluginName]\.ideas\creative-brief.md`.

---

## Step 2 — Create qa\ directory

```powershell
New-Item -ItemType Directory -Force "D:\Dev\Plugins\[PluginName]\qa" | Out-Null
```

---

## Step 3 — Print Plugin Doctor instructions

Print the following verbatim (substitute [PluginName]):

```
┌─────────────────────────────────────────────────────────────┐
│  PLUGIN DOCTOR — EXPORT INSTRUCTIONS                        │
│                                                             │
│  1. Open Plugin Doctor                                      │
│                                                             │
│  2. File → Load Plugin                                      │
│     Navigate to:                                            │
│     C:\Program Files\Common Files\VST3\Orient Plugins\      │
│     Select: [PluginName].vst3                               │
│                                                             │
│  3. Click "Run All Tests" and wait for completion           │
│                                                             │
│  4. File → Export → Text Report                             │
│     Save to exactly this path:                              │
│     D:\Dev\Plugins\[PluginName]\qa\plugin-doctor-report.txt │
│                                                             │
│  5. Type "done" when the export is complete                 │
└─────────────────────────────────────────────────────────────┘
```

---

## Step 4 — Wait for "done"

Wait for the user to type "done".

---

## Step 5 — Verify file exists

```powershell
$path = "D:\Dev\Plugins\[PluginName]\qa\plugin-doctor-report.txt"
Test-Path $path
```

If the file does not exist:

```
File not found at:
  D:\Dev\Plugins\[PluginName]\qa\plugin-doctor-report.txt

Please re-export the Plugin Doctor report to that exact path,
then type "done" again.
```

Wait for "done" again. Repeat verification once more. If still missing after
two attempts, stop and ask the user to verify the export path manually.

---

## Step 6 — Invoke qa-audio-agent

Invoke qa-audio-agent with:
- PluginName
- Version (from creative-brief.md)
- Report path: `D:\Dev\Plugins\[PluginName]\qa\plugin-doctor-report.txt`

---

## Step 7 — Report result

On PASS from qa-audio-agent:
```
run-qa-audio complete
Plugin: [PluginName] v[version]
Result: ALL PASS

[7-row metrics table from qa-audio-agent]

qa-audio-v[version].md written to:
  D:\Dev\Plugins\[PluginName]\qa\qa-audio-v[version].md

qa-report.md updated: Audio QA: PASS
```

On FAIL from qa-audio-agent:
```
run-qa-audio complete
Plugin: [PluginName] v[version]
Result: [N] FAILURES

[metrics table showing FAIL rows]

audio-fix-agent invoked with fix list.
Re-run Plugin Doctor after fixes are applied,
then type: Run audio QA for [PluginName]
```

## Constraints
- Never invoke qa-audio-agent before verifying the report file exists
- Never modify plugin source — this skill is read-only infrastructure
- If Plugin Doctor is not installed, print:
  "Plugin Doctor required. Download from: https://www.ddmf.eu/plugindoctor/"
  and stop.
