# Troubleshooting Index

Each row links to a full incident entry. Entries are written by knowledge-agent after debug-agent resolves an error.

## Index

| Date | Slug | Category | Summary |
|---|---|---|---|
| — | — | — | *(no entries yet)* |

## Categories

| Category | Description |
|---|---|
| `cmake` | CMake configure errors, generator issues, target problems |
| `compiler` | MSVC compile errors (C-prefixed), linker errors (LNK-prefixed) |
| `dsp` | Audio processing bugs, parameter not affecting audio, denormals |
| `gui` | Editor crashes, control not visible, attachment failures |
| `pluginval` | pluginval test failures at any strictness level |
| `runtime` | Crash on DAW load, crash on parameter change, state corruption |

## Entry Format

Each entry file at `troubleshooting/[slug].md` contains:
- Error (verbatim)
- Context (stage, agent, plugin)
- Root Cause (one sentence)
- Fix Applied (code change or config change)
- Prevention (what to check on future plugins)
