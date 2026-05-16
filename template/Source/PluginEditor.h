#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class [PluginName]AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit [PluginName]AudioProcessorEditor([PluginName]AudioProcessor&);
    ~[PluginName]AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    [PluginName]AudioProcessor& audioProcessor;

    // Controls — add sliders, buttons, labels here
    juce::Slider placeholderKnob;
    juce::Label  placeholderLabel;

    // APVTS attachments — one per control
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> placeholderAttach;

    void setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR([PluginName]AudioProcessorEditor)
};
