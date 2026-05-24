---
name: version-bump
description: >
  Updates the version string in creative-brief.md and CMakeLists.txt for the
  active plugin. Prints old -> new for both files. Invoke with
  "Bump version to [x.x.x]".
allowed-tools:
  - Read
  - Edit
---

# version-bump

Updates the version number in both the contract file and the build system.

## Reads From

- `plugins/[ActivePlugin]/.ideas/creative-brief.md` - current version string
- `plugins/[ActivePlugin]/CMakeLists.txt` - VERSION field

## Writes To

- `plugins/[ActivePlugin]/.ideas/creative-brief.md` - updated version
- `plugins/[ActivePlugin]/CMakeLists.txt` - updated version

## Inputs Required

From the invocation: `Bump version to [x.x.x]`

- `NewVersion` - the target version string, e.g. `1.1.0`

## Workflow

### 1. Validate new version

`NewVersion` must match `^\d+\.\d+\.\d+$` (semver: major.minor.patch).
If it does not match, stop and report:
```
version-bump: invalid version string "[input]". Expected format: x.x.x (e.g. 1.1.0)
```

### 2. Read active plugin

Read `.claude/CLAUDE.md`, extract active plugin Name and Path.

### 3. Read current version from creative-brief.md

Find the version line in `plugins/[ActivePlugin]/.ideas/creative-brief.md`.
Expected format (any of these are accepted):
```
## Version
1.0.0

Version: 1.0.0

**Version:** 1.0.0
```

Store as `OldVersion`.

### 4. Read current version from CMakeLists.txt

Find the version in `plugins/[ActivePlugin]/CMakeLists.txt`.
Expected pattern: `VERSION [version-string]` in the `project(...)` call or as a standalone variable:
```
VERSION 1.0.0
```

Store as `CmakeOldVersion` (may differ from brief if previously out of sync — update both).

### 5. Apply updates

Use Edit to update each file precisely:

**creative-brief.md** - replace the version value in-place, do not touch surrounding content.

**CMakeLists.txt** - replace the version string wherever it appears as a version literal.
Common locations:
- `project([Name] VERSION 1.0.0)` - update the `VERSION` argument
- `VIProductVersion "1.0.0"` - if present, update this too
- `VERSION 1.0.0` standalone variable - if present, update

### 6. Confirm

```
version-bump complete
Plugin: [Name]

creative-brief.md:  [OldVersion] -> [NewVersion]
CMakeLists.txt:     [CmakeOldVersion] -> [NewVersion]

Next step: Create a Linear issue for this version cycle
           and add the issue ID to linear-issue.txt:
           D:\Dev\Plugins\[Name]\linear-issue.txt
```

If either file had no version field found:
```
version-bump: WARNING — no version field found in [filename]. Add manually:
  creative-brief.md: add a line "Version: [NewVersion]"
  CMakeLists.txt:    add VERSION [NewVersion] to the project() call
```

## Constraints

- Never changes anything other than the version string — no reformatting, no other edits
- If creative-brief.md version and CMakeLists.txt version are already in sync, still update both
- Does not trigger a rebuild — user must run /build-compile after bumping
