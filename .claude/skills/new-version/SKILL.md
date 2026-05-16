---
name: new-version
description: >
  Archives the current .ideas/ contracts as a versioned snapshot, resets the
  plugin to planning stage with a fresh .ideas/ pre-filled from the archive,
  and bumps the version string. Invoke with "Start new version [x.x.x] of [PluginName]".
allowed-tools:
  - Read
  - Write
  - Edit
  - Bash
---

# new-version

Starts a new development version of an existing plugin. Archives the current
contracts, creates a clean planning baseline, and resets the pipeline stage.

## Reads From

- `.claude/CLAUDE.md` - active plugin name and path
- `plugins/[ActivePlugin]/.ideas/` - all current contract files (to archive)
- `plugins/[ActivePlugin]/.ideas/creative-brief.md` - current version number

## Writes To

- `plugins/[ActivePlugin]/versions/v[N]/` - archived snapshot of current .ideas/
- `plugins/[ActivePlugin]/.ideas/` - fresh contracts with v[N] as baseline
- `.claude/CLAUDE.md` - stage reset to planning, gate reset to none

## Inputs Required

From the invocation: `Start new version [x.x.x] of [PluginName]`

- `PluginName` - must match the active plugin in CLAUDE.md
- `NewVersion` - target version string, e.g. `1.2.0`

## Workflow

### 1. Validate inputs

- `NewVersion` must match `^\d+\.\d+\.\d+$`
- `PluginName` must match the Name in CLAUDE.md Active Plugin section
- Plugin directory must exist at the path specified in CLAUDE.md

If any check fails, stop and report.

### 2. Determine archive slot

Scan `plugins/[ActivePlugin]/versions/` for existing `v1\`, `v2\` etc.
Next slot `N` = highest existing number + 1. If `versions/` does not exist, N = 1.

```powershell
$versionsDir = "plugins\$PluginName\versions"
$existing = Get-ChildItem -Path $versionsDir -Directory -Filter "v*" -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match '^v\d+$' } |
            ForEach-Object { [int]($_.Name.TrimStart('v')) }
$N = if ($existing) { ($existing | Measure-Object -Maximum).Maximum + 1 } else { 1 }
$archivePath = "$versionsDir\v$N"
```

### 3. Archive current .ideas/

```powershell
New-Item -ItemType Directory -Path $archivePath -Force | Out-Null
Copy-Item -Path "plugins\$PluginName\.ideas\*" -Destination $archivePath -Recurse
```

Confirm all four contract files were copied:
- `creative-brief.md`
- `parameter-spec.md`
- `plan.md`
- `architecture.md`

If any file is missing from the source `.ideas/`, copy what exists and log a warning.

### 4. Reset .ideas/ with baseline from archive

Overwrite the current `.ideas/` files with fresh templates, but pre-fill
`creative-brief.md` from the archived version as a starting baseline.

**fresh creative-brief.md** — copy the archived version verbatim, then:
- Update the version field to `NewVersion`
- Add a note at the top:

```markdown
<!-- Baseline: v[N] archived at versions/v[N]/creative-brief.md -->
<!-- Update fields below for the new version. Clear sections that change. -->
```

**fresh parameter-spec.md** — blank template (same as new-plugin creates):
```markdown
# Parameter Spec

| ID | Type | Min | Max | Default | Unit | DSP Role |
|----|------|-----|-----|---------|------|----------|

## Parameter Details
```

**fresh plan.md** — blank template:
```markdown
# Plan

## Complexity Score

## DSP Stages

## Parameter Count

## Risk Register

| Risk | Likelihood | Mitigation |
|------|-----------|------------|

## Open Questions
```

**fresh architecture.md** — blank template:
```markdown
# Architecture

## Signal Chain

## DSP Components

## Parameter -> DSP Mapping

| Parameter ID | Component | Setter / Effect |
|-------------|-----------|-----------------|

## Member Declaration Order in PluginProcessor.h
```

### 5. Reset CLAUDE.md

In `.claude/CLAUDE.md`, update the Active Plugin table:

```markdown
| Current stage | planning |
| Last completed | none |
```

Leave Name and Path unchanged.

### 6. Run version-bump

Invoke the `version-bump` skill with `NewVersion` to update:
- `plugins/[ActivePlugin]/.ideas/creative-brief.md` (already has new version from step 4)
- `plugins/[ActivePlugin]/CMakeLists.txt`

### 7. Confirm

```
new-version complete
Plugin:   [PluginName]
Archived: plugins/[PluginName]/versions/v[N]/
  creative-brief.md  (v[OldVersion])
  parameter-spec.md
  plan.md
  architecture.md

Fresh .ideas/ ready with v[N] as baseline.
Version bumped: [OldVersion] -> [NewVersion]
Stage reset: planning | Gate reset: none

Next steps:
  1. Review and update .ideas/creative-brief.md for the new version scope
  2. Fill in .ideas/parameter-spec.md with any new or changed parameters
  3. Run plugin-planner or fill contracts manually
  4. Run /resume when contracts are confirmed
```

## Constraints

- Never deletes or overwrites the archive once written
- Never resets the stage if PluginName does not match the active plugin in CLAUDE.md
- Never modifies source files (Source/, build/) - only .ideas/, versions/, and CLAUDE.md
- If the current .ideas/ has incomplete contracts, archive what exists and log which files were missing
- version-bump must run as the final step — not before the archive is confirmed complete
