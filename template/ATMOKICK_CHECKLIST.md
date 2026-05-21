# AtmoKick — Plugin Development Checklist
**Version:** 1.2.0 | **Studio:** Orient Plugins | **Status:** Live on Gumroad

---

## PHASE 1 — PLANNING
- [x] Plugin concept defined
- [x] Creative brief written (`.ideas/creative-brief.md`)
- [x] Parameter spec written (`.ideas/parameter-spec.md`)
- [x] Architecture designed (`.ideas/architecture.md`)
- [x] Plan written (`.ideas/plan.md`)

## PHASE 2 — BUILD
- [x] CMakeLists.txt scaffolded
- [x] PluginProcessor.h/cpp generated
- [x] PluginEditor.h/cpp generated
- [x] CMake configure passes
- [x] DSP implementation complete
  - [x] Sine oscillator with pitch sweep
  - [x] Noise burst click layer
  - [x] Exponential amplitude envelope
  - [x] tanh soft-clip waveshaper
  - [x] 80Hz low-shelf sub boost
  - [x] Mid-band punch EQ (200-800Hz)
  - [x] Atmosphere engine (Reverb/Space/Shimmer)
  - [x] Chromatic MIDI pitch response
- [x] UI implementation complete
  - [x] All 12 parameters wired to APVTS
  - [x] Double-ring knob style
  - [x] Plugin name + Studio name labels
  - [x] Version badge
  - [x] Value readouts below knobs
  - [x] Active indicator dot
  - [x] Orient Plugins logo bottom-right
- [x] Compile passes (Release)

## PHASE 3 — QA
- [x] pluginval — strictness 10 — PASS (2026-05-21)
- [x] All 12 parameters validated
- [x] Parameter ranges match spec
- [x] Real-time safety confirmed
- [x] qa-report.md written — Status: PASS
- [ ] Plugin Doctor analysis
  - [ ] Noise floor < -100dB
  - [ ] Aliasing — none detected
  - [ ] THD at Drive=0 < 1%
  - [ ] Parameter smoothing — no zipper noise
  - [ ] CPU < 5% single instance
  - [ ] Latency reported correctly
  - [ ] Stereo balance < 0.5dB
  - [ ] qa-audio-v1.2.0.md written
- [ ] Ableton load test — logo confirmed rendering
- [ ] UI contrast confirmed — all states correct
  - [ ] Default state — dark text on light background
  - [ ] Click to edit — blue outline visible
  - [ ] Typed value — text and caret visible

## PHASE 4 — BRANDING
- [x] Orient Plugins palette applied (#5B7BF8 accent)
- [x] studio-identity.md referenced
- [x] ui-design-spec.md generated
- [x] Logo asset copied to Source\Assets\
- [ ] Logo rendering confirmed in DAW context
- [ ] All slider textbox colours confirmed correct

## PHASE 5 — INSTALLER
- [x] EULA.txt generated
- [x] README.txt generated
- [x] NSIS script created
- [x] Directory selection page added
- [x] Default path: C:\Program Files\Common Files\VST3\Orient Plugins\
- [x] Versioned installer folder structure (v1.0.0, v1.1.0, v1.2.0)
- [x] installer-version.txt = 1.2.0
- [ ] v1.2.0 installer recompiled with README + EULA inside
- [ ] Install test — plugin appears in DAW after install

## PHASE 6 — RELEASE NOTES
- [ ] releases\v1.2.0-release-notes.md written
- [ ] releases\CHANGELOG.md updated
- [ ] v1.0.0 entry documented
- [ ] v1.1.0 entry documented (chromatic MIDI + UI fix)
- [ ] v1.2.0 entry documented (skeleton fixes)

## PHASE 7 — GIT + CI
- [x] D:\Dev\Plugins\ git repo initialized
- [x] github.com/1Ronen/my-plugins — connected
- [x] qa-report.md Status: PASS — backup unblocked
- [x] Both repos pushed — up to date
- [x] .github\workflows\build.yml deployed
- [x] GitHub Actions — workflow visible in Actions tab
- [ ] GitHub Actions — first run triggered
- [ ] Windows build — PASS
- [ ] Mac build — PASS (VST3 + AU)
- [ ] GitHub Release created with both installers
- [ ] Mac installer (.pkg) downloaded + tested

## PHASE 8 — GUMROAD
- [x] Gumroad account configured (Orient Plugins)
- [x] Product listing created
- [x] Description — full copy pasted
- [x] Bio — Orient Plugins brand bio
- [x] Thumbnail — 600x600 logo uploaded
- [x] Price — $0 min / $9 suggested
- [x] Gumroad Discover — enabled
- [x] Category — Music
- [x] Tags — 13 tags added
- [x] Published — LIVE
- [ ] v1.2.0 installer uploaded (replace v1.1.0)
- [ ] Version note added to description top

## PHASE 9 — MARKETING
- [ ] 60-second demo video recorded in Ableton
- [ ] YouTube — video uploaded
- [ ] Reddit — r/edmproduction post
- [ ] Reddit — r/WeAreTheMusicMakers post
- [ ] Reddit — r/synthesizers post
- [ ] Reddit — r/audioengineering post
- [ ] KVR Audio — listing submitted
- [ ] KVR Forum — New Releases post
- [ ] Plugin Boutique — submitted
- [ ] VI-Control — Freebies post
- [ ] Instagram/TikTok — 30 sec Shimmer demo
- [ ] Splice — free plugin directory

## PHASE 10 — POST-SHIP
- [ ] User feedback collected (KVR comments, Reddit)
- [ ] v1.3.0 backlog defined (velocity sensitivity etc.)
- [ ] Plugin Doctor automation — run-qa-audio tested
- [ ] Skeleton improvements from this build applied
- [ ] BACKLOG.md updated

---

## Open Issues
| # | Issue | Priority | Status |
|---|---|---|---|
| 1 | Logo not confirmed rendering in Ableton | High | Open |
| 2 | Slider textbox colours — confirm fix applied | High | Open |
| 3 | GitHub Actions first run — not yet triggered | High | Open |
| 4 | Plugin Doctor analysis not done | Medium | Open |
| 5 | v1.2.0 installer not rebuilt with README+EULA | Medium | Open |
| 6 | Release notes not written | Medium | Open |
| 7 | No marketing posts live yet | Medium | Open |
| 8 | Gumroad — old installer still uploaded | Medium | Open |

---

## Version History
| Version | Date | Changes |
|---|---|---|
| 1.0.0 | 2026-05 | Initial release — basic kick synth + atmosphere |
| 1.1.0 | 2026-05 | Chromatic MIDI pitch + UI contrast fix |
| 1.2.0 | 2026-05 | Skeleton bug fixes — slider colours, logo path, JUCE configurable |

---

*Update this file after every completed phase or resolved issue.*
*Stored at: D:\Dev\Plugins\AtmoKick\CHECKLIST.md*
