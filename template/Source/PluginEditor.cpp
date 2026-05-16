#include "PluginEditor.h"

[PluginName]AudioProcessorEditor::[PluginName]AudioProcessorEditor([PluginName]AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Setup controls
    setupKnob(placeholderKnob, placeholderLabel, "PARAM");

    // Attach to APVTS — replace "param_id" with actual parameter ID
    // placeholderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    //     audioProcessor.apvts, "param_id", placeholderKnob);

    setSize(400, 300);
}

[PluginName]AudioProcessorEditor::~[PluginName]AudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void [PluginName]AudioProcessorEditor::setupKnob(
    juce::Slider& knob, juce::Label& label, const juce::String& text)
{
    knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(knob);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::FontOptions().withHeight(11.0f).withStyle("Bold")));
    label.setColour(juce::Label::textColourId, juce::Colour(0xff777777));
    addAndMakeVisible(label);
}

void [PluginName]AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    g.setColour(juce::Colour(0xffe0e0e0));
    g.setFont(juce::Font(juce::FontOptions().withHeight(18.0f).withStyle("Bold")));
    g.drawText("[ProductName]", getLocalBounds().removeFromTop(44), juce::Justification::centred);

    g.setColour(juce::Colour(0xff333333));
    g.drawLine(0.0f, 44.0f, (float)getWidth(), 44.0f, 0.5f);
}

void [PluginName]AudioProcessorEditor::resized()
{
    // Replace this with layout for your controls
    const int knobSize = 100;
    const int labelH   = 20;
    const int knobX    = (getWidth()  - knobSize) / 2;
    const int knobY    = 60;

    placeholderKnob .setBounds(knobX, knobY,            knobSize, knobSize);
    placeholderLabel.setBounds(knobX, knobY + knobSize, knobSize, labelH);
}
