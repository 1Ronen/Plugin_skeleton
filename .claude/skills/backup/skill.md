---
name: backup
description: >
  Pushes both plugin and skeleton repos to GitHub. Reads qa-report.md first
  and blocks all git operations if status is not PASS. Runs version-check to
  decide whether to build a new installer before committing. Invoked
  automatically by qa-tester after an all-pass run, or manually via
  "backup" or "/backup".
tools: Read, Write, Bash
model: sonnet
---

# backup skill

Pushes D:\Dev\Plugins\ and D:\Dev\PluginSkeleton\ to GitHub.
Blocked entirely if qa-report.md is missing or Status is not PASS.
Runs version-check to gate installer builds — only builds when version changed.

## Invocation

```
"backup"  |  "/backup"  |  triggered automatically by qa-tester on PASS
```

## Step 1 — Gate check (REQUIRED — runs before any git command)

Read the active plugin's `qa-report.md`:
```
D:\Dev\Plugins\[ActivePlugin]\qa-report.md
```

Determine active plugin from `D:\Dev\PluginSkeleton\.claude\CLAUDE.md`
(Active Plugin section) if not passed in as context.

### File missing
```
BLOCKED: qa-report.md not found for [PluginName].
Run qa-tester before pushing to git.
```
Stop. Do not run any git command.

### Status: FAIL
```
BLOCKED: QA status is FAIL for [PluginName].
Fix all pluginval errors and re-run qa-tester before pushing to git.
```
Stop. Do not run any git command.

### Status: PASS
Proceed to Step 2.

## Step 2 — Read version

Read `D:\Dev\Plugins\[ActivePlugin]\.ideas\creative-brief.md`.
Extract `Version:` field (e.g. `1.2.0`).
Use today's date in `YYYY-MM-DD` format.

## Step 3 — Run version-check

Run version-check skill for [ActivePlugin].

version-check reads:
- `creative-brief.md` → current version
- `installer\installer-version.txt` → last built installer version

Proceed based on result:

---

### Branch A — VERSION CHANGED

```
VERSION CHANGED: [old] → [new]
```

**a. Run installer-agent for [ActivePlugin]**

Invoke installer-agent. Pass:
- PluginName
- Version (from creative-brief.md)

Wait for Gate OUT:
- `installer\v[version]\[PluginName]_Setup_v[version].exe` exists
- `installer\v[version]\install-log-v[version].md` Status: PASS

If installer-agent fails: stop, report error, do not push.

**b. Update installer-version.txt**

Write current version to `D:\Dev\Plugins\[ActivePlugin]\installer\installer-version.txt`:
```
[version]
```

**c. Git add + commit + push — plugins repo**

```powershell
cd D:\Dev\Plugins
git add -A
git commit -m "release: [PluginName] v[version] — installer included"
git push origin main
```

**d. Git add + commit + push — skeleton repo**

```powershell
cd D:\Dev\PluginSkeleton
git add -A
git commit -m "skeleton: updated alongside [PluginName] v[version]"
git push origin main
```

---

### Branch B — VERSION UNCHANGED

```
VERSION UNCHANGED: [version]
```

No installer built.

**a. Git add + commit + push — plugins repo**

```powershell
cd D:\Dev\Plugins
git add -A
git commit -m "fix: [PluginName] v[version] — [YYYY-MM-DD]"
git push origin main
```

If working tree is clean: skip commit, push only if ahead of remote.

**b. Git add + commit + push — skeleton repo**

```powershell
cd D:\Dev\PluginSkeleton
git add -A
git commit -m "skeleton: updated alongside [PluginName] v[version]"
git push origin main
```

If working tree is clean: skip commit, push only if ahead of remote.

---

## Step 4 — Report

### CHANGED path:
```
backup complete
QA gate:          PASS
Version check:    CHANGED [old] → [new]
Installer built:  installer\v[version]\[PluginName]_Setup_v[version].exe
installer-version.txt updated to [version]

Plugins repo:  [commit hash] — [N] files changed
               pushed → github.com/1Ronen/my-plugins

Skeleton repo: [commit hash] — [N] files changed
               pushed → github.com/1Ronen/plugin-skeleton

Both repos: confirmed pushed.

⚠  Update CHECKLIST.md
   Mark Stage 5 (QA) and Stage 7 (Installer) complete in:
   D:\Dev\Plugins\[PluginName]\CHECKLIST.md
```

### UNCHANGED path:
```
backup complete
QA gate:          PASS
Version check:    UNCHANGED [version]
Installer:        not built (version unchanged)

Plugins repo:  [commit hash] — [N] files changed
               pushed → github.com/1Ronen/my-plugins

Skeleton repo: [commit hash] — [N] files changed
               pushed → github.com/1Ronen/plugin-skeleton

Both repos: confirmed pushed.

⚠  Update CHECKLIST.md
   Mark Stage 5 (QA) complete in:
   D:\Dev\Plugins\[PluginName]\CHECKLIST.md
```

## Constraints

- Step 1 gate check is mandatory — never skip it, even when triggered manually
- Never commit or push if qa-report.md is missing or Status is FAIL
- Never build installer if version-check returns UNCHANGED
- Never skip installer if version-check returns CHANGED
- Update `installer-version.txt` only after installer Gate OUT confirmed
- Both repos must be pushed in a single run — never push one without the other
- If installer-agent fails: stop before any git push, report error
- If either push fails (network, auth): report the error and stop — do not retry silently
- Version must come from `creative-brief.md` — never hardcoded
