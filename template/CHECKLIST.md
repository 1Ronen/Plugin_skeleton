# [PluginName] v[Version] — Development Checklist

> Update this file as each stage completes.
> Mark boxes with [x] only when the stage fully passes — not when work starts.

---

## STAGE 1 — Planning & Contracts

- [ ] `creative-brief.md` filled in — plugin name, type, description, target user
- [ ] `parameter-spec.md` filled in — all parameter IDs, types, ranges, defaults
- [ ] `architecture.md` filled in — signal chain, DSP components, parameter → DSP mapping
- [ ] `plan.md` filled in — complexity score, DSP stages, risk register
- [ ] Plugin ideation complete — all contracts reviewed and confirmed
- [ ] No open questions in `plan.md`

---

## STAGE 2 — Foundation

- [ ] CMakeLists.txt generated from template — `[PluginName]` replaced throughout
- [ ] Root CMakeLists.txt updated — `add_subdirectory(plugins/[PluginName])`
- [ ] JUCE_PATH configurable — no hardcoded path in root CMakeLists.txt
- [ ] POST_BUILD assets copy present — targets `[PluginName]_VST3`
- [ ] All parameters in `createParameterLayout()` — IDs match parameter-spec.md exactly
- [ ] `isBusesLayoutSupported` overridden — accepts mono and stereo
- [ ] `ScopedNoDenormals` first line of `processBlock`
- [ ] CMake configure passes — zero errors
- [ ] Parameter coverage 100% — all spec IDs present in source

---

## STAGE 3 — DSP Implementation

- [ ] `prepareToPlay` implemented — sample rate, block size, all DSP initialised
- [ ] `processBlock` implemented — full signal chain active
- [ ] All parameter pointers cached in `prepareToPlay`
- [ ] No allocations inside `processBlock`
- [ ] No `getParameter()` calls inside `processBlock` — use cached raw pointers
- [ ] Denormal protection on all feedback paths
- [ ] State save/load working — `getStateInformation` / `setStateInformation`
- [ ] Release build passes — zero compiler errors or warnings

---

## STAGE 4 — UI Implementation

- [ ] `PluginEditor.h` — all controls declared, attachments declared after controls
- [ ] All 4 textbox colours set directly on each Slider (not only in LookAndFeel)
- [ ] `setLookAndFeel(nullptr)` is first line of destructor
- [ ] Logo loading uses `File::getSpecialLocation(currentApplicationFile)`
- [ ] Logo fallback uses `BinaryData` if file not found
- [ ] Logo fallback text drawn in `paint()` if `logoImage_.isValid()` is false
- [ ] `OrientLookAndFeel` / custom LookAndFeel constructed and applied
- [ ] TextEditor edit-state colours set in LookAndFeel constructor
- [ ] Every parameter has a visible control with working APVTS attachment
- [ ] Window size appropriate for parameter count
- [ ] Studio name, version badge, active dot all present
- [ ] Release build passes — zero compiler errors or warnings

---

## STAGE 5 — QA (pluginval)

- [ ] VST3 installed to `C:\Program Files\Common Files\VST3\Orient Plugins\`
- [ ] pluginval run at strictness 10 — zero failures
- [ ] Parameter coverage check — all IDs in source match spec
- [ ] Parameter ranges check — all ranges match spec
- [ ] `isBusesLayoutSupported` confirmed present in source
- [ ] Real-time safety scan — no `new`, `delete`, `mutex`, `getParameter()` in processBlock
- [ ] `qa-report.md` written — `Status: PASS`

---

## STAGE 6 — Audio QA (Plugin Doctor)

- [ ] Plugin Doctor report exported to `qa\plugin-doctor-report.txt`
- [ ] Noise floor below −100 dB
- [ ] No aliasing detected
- [ ] THD at Drive=0% below 1%
- [ ] No zipper noise on any parameter
- [ ] CPU single instance below 5%
- [ ] Reported latency matches actual latency
- [ ] Stereo balance below 0.5 dB
- [ ] `qa\qa-audio-v[Version].md` written — all 7 metrics PASS
- [ ] `qa-report.md` updated — `Audio QA: PASS`

---

## STAGE 7 — Installer

- [ ] `installer\EULA.txt` present — [PluginName], vendor, year, version filled in
- [ ] `installer\README.txt` present — quick start, parameter reference, support info
- [ ] `installer\v[Version]\[PluginName]_v[Version].nsi` generated
- [ ] NSIS installer compiles — zero errors
- [ ] `installer\v[Version]\[PluginName]_Setup_v[Version].exe` exists and size > 0
- [ ] EULA page displayed during install
- [ ] Directory page present — user can change install path
- [ ] `$COMMONFILES64` used — not `$COMMONFILES`
- [ ] Uninstaller uses `$INSTDIR` — no hardcoded paths
- [ ] File + EULA.txt + README.txt all copied to install dir
- [ ] Uninstaller removes all three files cleanly
- [ ] `installer\installer-version.txt` updated to `[Version]`
- [ ] `installer\v[Version]\install-log-v[Version].md` written — `Status: PASS`

---

## STAGE 8 — Release

- [ ] Version bumped in `creative-brief.md`
- [ ] Version bumped in root `CMakeLists.txt`
- [ ] `releases\CHANGELOG.md` entry written for v[Version]
- [ ] `qa-report.md` `Status: PASS` confirmed before any push
- [ ] Both repos pushed — plugins + skeleton
- [ ] GitHub release created (if version tag pushed)
- [ ] All previous version installer subfolders still present — no rollback data deleted

---

## Notes

<!-- Add any per-version notes, known issues, or decisions here -->
