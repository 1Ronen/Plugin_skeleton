#include "PluginProcessor.h"
#include "PluginEditor.h"

[PluginName]AudioProcessor::[PluginName]AudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{}

[PluginName]AudioProcessor::~[PluginName]AudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout [PluginName]AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Add parameters from parameter-spec.md
    // Example float:
    //   layout.add(std::make_unique<juce::AudioParameterFloat>(
    //       "gain", "Gain",
    //       juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),
    //       0.0f,
    //       juce::AudioParameterFloatAttributes().withStringFromValueFunction(
    //           [](float v, int) -> juce::String { return juce::String(v, 1) + " dB"; })));
    //
    // Example bool:
    //   layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
    //
    // Example choice:
    //   layout.add(std::make_unique<juce::AudioParameterChoice>(
    //       "mode", "Mode", juce::StringArray { "A", "B", "C" }, 0));

    return layout;
}

bool [PluginName]AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;

    return in == out;
}

void [PluginName]AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    // Prepare DSP components here
}

void [PluginName]AudioProcessor::releaseResources()
{
    // Reset/release DSP components here
}

void [PluginName]AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Read parameters (real-time safe atomic reads)
    // const float gainDb = apvts.getRawParameterValue("gain")->load();

    // Process audio
    // buffer.applyGain(...);
}

void [PluginName]AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void [PluginName]AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* [PluginName]AudioProcessor::createEditor()
{
    return new [PluginName]AudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new [PluginName]AudioProcessor();
}
