#include "PluginProcessor.h"
#include "PluginEditor.h"

SimpleGainAudioProcessor::SimpleGainAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{}

SimpleGainAudioProcessor::~SimpleGainAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleGainAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "gain", "Gain",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float v, int) -> juce::String {
                if (v <= -60.0f) return "-inf dB";
                return (v >= 0.0f ? "+" : "") + juce::String(v, 1) + " dB";
            })));

    return layout;
}

bool SimpleGainAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;

    return in == out;
}

void SimpleGainAudioProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/)
{
    gainParam = apvts.getRawParameterValue("gain");
}

void SimpleGainAudioProcessor::releaseResources() {}

void SimpleGainAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const float gainDb     = gainParam != nullptr ? gainParam->load() : 0.0f;
    const float gainLinear = (gainDb <= -60.0f) ? 0.0f : std::pow(10.0f, gainDb / 20.0f);
    buffer.applyGain(gainLinear);
}

void SimpleGainAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SimpleGainAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* SimpleGainAudioProcessor::createEditor()
{
    return new SimpleGainAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleGainAudioProcessor();
}
