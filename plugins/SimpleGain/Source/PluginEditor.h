#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SimpleGainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SimpleGainAudioProcessorEditor(SimpleGainAudioProcessor&);
    ~SimpleGainAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SimpleGainAudioProcessor& audioProcessor;

    juce::Slider gainKnob;
    juce::Label  gainLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleGainAudioProcessorEditor)
};
