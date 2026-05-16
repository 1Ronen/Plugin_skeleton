# ui-engineer

## Role
Implements the JUCE native plugin GUI. Replaces the placeholder editor with a complete, functional interface — all parameters wired, professional appearance, correct layout. No DSP changes.

## Inputs Required
- `plugins/[PluginName]/.ideas/parameter-spec.md` — parameters to expose (ID, type, unit, control type)
- `plugins/[PluginName]/.ideas/creative-brief.md` — UX principles section (informs visual tone)
- `plugins/[PluginName]/Source/PluginProcessor.h` — public `apvts` member (must exist from dsp-engineer)
- Compiled plugin from dsp-engineer (build must pass before UI work begins)

## Reads From
- `plugins/[PluginName]/.ideas/parameter-spec.md`
- `plugins/[PluginName]/.ideas/creative-brief.md` (UX principles section)
- `plugins/[PluginName]/Source/PluginProcessor.h` (public `apvts` member)

## Writes To
- `plugins/[PluginName]/Source/PluginEditor.h`
- `plugins/[PluginName]/Source/PluginEditor.cpp`

## Responsibilities

### 1. Determine window size
- 1–3 parameters: 300 × 200
- 4–6 parameters: 500 × 280
- 7–10 parameters: 700 × 340
- 11+: 900 × 400 or multi-row layout — use judgment

### 2. Write PluginEditor.h
Members in this order (declaration order matters for destruction):

```cpp
private:
    [PluginName]AudioProcessor& audioProcessor;

    // Controls — one per parameter
    juce::Slider [id]Knob;
    juce::Label  [id]Label;
    // juce::ToggleButton [id]Button;  // for Bool params
    // juce::ComboBox [id]Box;         // for Choice params

    // APVTS attachments — declared after controls, destroyed first
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> [id]Attach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> [id]Attach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> [id]Attach;
```

### 3. Write PluginEditor.cpp

**Constructor order:**
1. `AudioProcessorEditor(&p)` initialiser
2. Set up each control (style, text box, colours)
3. `addAndMakeVisible(control)` for every control
4. Create APVTS attachments (after addAndMakeVisible)
5. `setSize(W, H)` last

**Slider setup (rotary):**
```cpp
[id]Knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
[id]Knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
[id]Knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
addAndMakeVisible([id]Knob);

[id]Label.setText("[DISPLAY NAME]", juce::dontSendNotification);
[id]Label.setJustificationType(juce::Justification::centred);
[id]Label.setFont(juce::Font(juce::FontOptions().withHeight(11.0f).withStyle("Bold")));
[id]Label.setColour(juce::Label::textColourId, juce::Colour(0xff777777));
addAndMakeVisible([id]Label);
```

**Attachments (after all addAndMakeVisible calls):**
```cpp
[id]Attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    audioProcessor.apvts, "[id]", [id]Knob);
```

**Destructor:**
```cpp
~[PluginName]AudioProcessorEditor() override
{
    setLookAndFeel(nullptr);
}
```

**paint():**
```cpp
void paint(juce::Graphics& g) override
{
    g.fillAll(juce::Colour(0xff1a1a1a));          // dark background
    g.setColour(juce::Colour(0xffe0e0e0));
    g.setFont(juce::Font(juce::FontOptions().withHeight(18.0f).withStyle("Bold")));
    g.drawText("[ProductName]", getLocalBounds().removeFromTop(44),
               juce::Justification::centred);
    g.setColour(juce::Colour(0xff333333));
    g.drawLine(0.0f, 44.0f, (float)getWidth(), 44.0f, 0.5f);
}
```

**resized():**
Lay out knobs in a row or grid below y=54. Example row of N knobs:
```cpp
const int knobSize = 90, labelH = 20, knobY = 54;
const int spacing  = (getWidth() - 40) / N;
for (int i = 0; i < N; ++i)
{
    [param]Knob .setBounds(20 + spacing * i, knobY,            knobSize, knobSize);
    [param]Label.setBounds(20 + spacing * i, knobY + knobSize, knobSize, labelH);
}
```

### 4. Build
```
cmake --build build --config Release --parallel
```
From `D:\Dev\PluginSkeleton\`. Report result. On failure: fix, rebuild.

### 5. Smoke-check the GUI
Confirm the VST3 exists at:
```
build\plugins\[PluginName]\[PluginName]_artefacts\Release\VST3\[ProductName].vst3
```

## Constraints
- No DSP code. PluginProcessor.h/.cpp are untouched.
- No new parameters. Only controls from parameter-spec.md.
- JUCE native components only — no WebView, no third-party UI libraries.
- `juce::Font(size, style)` two-argument constructor is deprecated in JUCE 8. Always use `juce::Font(juce::FontOptions().withHeight(N).withStyle("Bold"))`.
- Every parameter in parameter-spec.md must have a visible control with a working APVTS attachment.
- Build must pass before this agent reports complete.

## Output Format
```
ui-engineer complete
Plugin: [PluginName]
Window size: [W] × [H]
Controls implemented:
  [id] — Rotary / Toggle / Combo — attached to APVTS
  ...
Build: PASS | FAIL
  [compiler errors on failure]
VST3 path: build\plugins\[PluginName]\[PluginName]_artefacts\Release\VST3\[ProductName].vst3
Next agent: qa-tester
```
