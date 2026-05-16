#include "PluginEditor.h"

SimpleGainAudioProcessorEditor::SimpleGainAudioProcessorEditor(SimpleGainAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    gainKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    gainKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(gainKnob);

    gainLabel.setText("GAIN", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setFont(juce::Font(juce::FontOptions().withHeight(11.0f).withStyle("Bold")));
    gainLabel.setColour(juce::Label::textColourId, juce::Colour(0xff777777));
    addAndMakeVisible(gainLabel);

    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "gain", gainKnob);

    setSize(300, 200);
}

SimpleGainAudioProcessorEditor::~SimpleGainAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void SimpleGainAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    g.setColour(juce::Colour(0xffe0e0e0));
    g.setFont(juce::Font(juce::FontOptions().withHeight(18.0f).withStyle("Bold")));
    g.drawText("SIMPLE GAIN", getLocalBounds().removeFromTop(44), juce::Justification::centred);

    g.setColour(juce::Colour(0xff333333));
    g.drawLine(0.0f, 44.0f, (float)getWidth(), 44.0f, 0.5f);
}

void SimpleGainAudioProcessorEditor::resized()
{
    const int knobSize = 100;
    const int labelH   = 20;
    const int knobX    = (getWidth()  - knobSize) / 2;
    const int knobY    = 54;

    gainKnob .setBounds(knobX, knobY,            knobSize, knobSize);
    gainLabel.setBounds(knobX, knobY + knobSize, knobSize, labelH);
}
