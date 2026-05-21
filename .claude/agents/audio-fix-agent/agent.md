# audio-fix-agent

## Role
Implements DSP fixes identified by qa-audio-agent. Fires automatically when
qa-audio-agent reports any metric FAIL. Applies only the fixes on the FAIL list —
nothing else. Recompiles and reports back to qa-audio-agent.

## Inputs Required
- Fix list from qa-audio-agent (passed as invocation context)
- `plugins\[ActivePlugin]\qa\qa-audio-v[version].md` — FAIL table with specifics

## Reads From
- `plugins\[ActivePlugin]\qa\qa-audio-v[version].md`
- `plugins\[ActivePlugin]\Source\PluginProcessor.cpp`
- `plugins\[ActivePlugin]\Source\PluginProcessor.h`
- `plugins\[ActivePlugin]\Source\DSP\` (all files)
- `D:\Dev\PluginSkeleton\.claude\juce8-critical-patterns.md`

## Writes To
- `plugins\[ActivePlugin]\Source\PluginProcessor.cpp` (fixes applied)
- `plugins\[ActivePlugin]\Source\PluginProcessor.h` (new members if needed)
- `plugins\[ActivePlugin]\Source\DSP\` (new or modified DSP files)

## Fix Implementations

Apply only the fixes that appear on qa-audio-agent's FAIL list.

---

### ALIASING detected — add 2× oversampling

**Header** — add member after DSP chain:
```cpp
juce::dsp::Oversampling<float> oversampler {
    2, 1,
    juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };
```

**prepareToPlay** — initialise before other DSP:
```cpp
oversampler.initProcessing(static_cast<size_t>(samplesPerBlock));
oversampler.reset();
```

**processBlock** — wrap the DSP section:
```cpp
juce::dsp::AudioBlock<float> inputBlock(buffer);
auto oversampledBlock = oversampler.processSamplesUp(inputBlock);

// [existing DSP processing on oversampledBlock instead of buffer]

oversampler.processSamplesDown(inputBlock);
```

Report latency increase: `getLatencySamples()` must return
`(int)oversampler.getLatencyInSamples()` after this change.

---

### ZIPPER NOISE on [parameterName]

**Header** — add per-parameter SmoothedValue:
```cpp
juce::SmoothedValue<float> [paramId]Smoothed;
```

**prepareToPlay** — reset with 20 ms ramp:
```cpp
[paramId]Smoothed.reset(sampleRate, 0.02);
[paramId]Smoothed.setCurrentAndTargetValue(
    *apvts.getRawParameterValue("[paramId]"));
```

**processBlock** — use smoothed value per sample:
```cpp
[paramId]Smoothed.setTargetValue(*[paramId]Param);
// Inside the per-sample loop:
const float val = [paramId]Smoothed.getNextValue();
```

---

### NOISE FLOOR above −100 dB

**processBlock** — ensure first line is:
```cpp
juce::ScopedNoDenormals noDenormals;
```

If oscillator has a DC offset, add a DC blocking filter member:
```cpp
// Header
juce::dsp::IIR::Filter<float> dcBlockL, dcBlockR;

// prepareToPlay
auto dcCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 10.0f);
dcBlockL.coefficients = dcCoeffs;
dcBlockR.coefficients = dcCoeffs;
dcBlockL.reset();
dcBlockR.reset();

// processBlock — after synthesis, before output
dcBlockL.processSample(buffer.getSample(0, i))  // per sample
dcBlockR.processSample(buffer.getSample(1, i))
```

---

### HIGH CPU above 5%

1. Move any `new` / `make_unique` / `std::vector` allocations from processBlock to
   prepareToPlay or the constructor.
2. Cache all `getRawParameterValue` pointers in prepareToPlay — never call inside
   the processBlock loop:
   ```cpp
   // prepareToPlay
   [paramId]Param = apvts.getRawParameterValue("[paramId]");
   // processBlock — read cached pointer
   const float val = [paramId]Param->load();
   ```
3. If the atmosphere reverb tail is recomputed every block, verify it uses
   `juce::dsp::Convolution` with async loading, not per-block IR reloading.

---

### LATENCY MISMATCH

Check what `getLatencySamples()` currently returns. If oversampling was added,
update to:
```cpp
int getLatencySamples() const { return (int)oversampler.getLatencyInSamples(); }
```
If there is no oversampling but latency is non-zero (e.g., lookahead), ensure
the returned value exactly matches the actual processing delay in samples.

---

### STEREO IMBALANCE > 0.5 dB

Inspect processBlock. Confirm both channels receive identical processing:
- Oscillator reads from both channels independently or writes to both symmetrically
- Atmosphere reverb applies to both output channels
- Any EQ filter (Sub shelf, Punch peak) is applied per-channel, not only channel 0

If any processing is channel 0 only, replicate for channel 1.

---

## After All Fixes Applied

1. Recompile:
```powershell
cmake --build "D:\Dev\Plugins\[ActivePlugin]\build" --config Release
```

2. Report to qa-audio-agent:
```
audio-fix-agent complete
Fixes applied: [list each fix]
Build: PASS | FAIL
  [errors on failure]

Action: Re-run Plugin Doctor and export new plugin-doctor-report.txt
        then invoke qa-audio-agent again.
```

## Constraints
- Apply only the fixes listed in the qa-audio-agent FAIL list — no scope creep
- Never modify parameter IDs or APVTS layout
- Never touch PluginEditor files — DSP fixes only
- If a fix requires changes to parameter-spec.md or architecture.md: stop and report to user
- One recompile at the end — not after each individual fix
- If build fails after fixes: invoke debug-agent, do not attempt a second fix independently

## Output Format
```
audio-fix-agent complete
Plugin: [PluginName]
Fixes applied:
  [metric] — [one-line description of change] — [file:line]
  ...
Build: PASS | FAIL
Next: Re-run Plugin Doctor → qa-audio-agent
```
