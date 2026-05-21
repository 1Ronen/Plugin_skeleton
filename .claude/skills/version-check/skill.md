---
name: version-check
description: >
  Compares the current plugin version in creative-brief.md against the last
  built installer version in installer/installer-version.txt. Returns CHANGED
  or UNCHANGED to gate whether installer-agent needs to run. Invoked
  automatically by the backup skill, or manually via
  "Run version-check for [PluginName]".
tools: Read, Write
model: sonnet
---

# version-check skill

Compares current plugin version against the last built installer version.
Used by backup skill to decide whether to build a new installer.

## Invocation

```
"Run version-check for [PluginName]"  |  triggered by backup skill
```

## Step 1 — Read current version

Read `D:\Dev\Plugins\[ActivePlugin]\.ideas\creative-brief.md`.
Extract the `Version:` field value (e.g. `1.2.0`).

If file not found: stop and report error.

## Step 2 — Read last installer version

Read `D:\Dev\Plugins\[ActivePlugin]\installer\installer-version.txt`.

If file does not exist: create it with content `0.0.0`, then use `0.0.0`
as the last version.

## Step 3 — Compare

| Condition | Output | Return |
|---|---|---|
| Current ≠ last installer | `VERSION CHANGED: [old] → [new]` | `CHANGED` |
| Current = last installer | `VERSION UNCHANGED: [version]` | `UNCHANGED` |

## Output

```
version-check complete
Plugin:           [PluginName]
Current version:  [version from creative-brief.md]
Last installer:   [version from installer-version.txt]
Result:           CHANGED | UNCHANGED
```

## Constraints

- Read-only except for creating `installer-version.txt` when missing
- Never modify `creative-brief.md`
- Never update `installer-version.txt` — that is the backup skill's job after a successful installer build
- If `creative-brief.md` has no Version field, stop and report: "Version field missing from creative-brief.md — add it before running version-check"
