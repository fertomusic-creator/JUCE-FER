#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TrackspacerAudioProcessor::TrackspacerAudioProcessor()
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
                     .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, juce::Identifier ("Trackspacer"), createParameterLayout())
{
    // Get parameter pointers
    amountParam = parameters.getRawParameterValue ("amount");
    attackParam = parameters.getRawParameterValue ("attack");
    releaseParam = parameters.getRawParameterValue ("release");
    lowFreqParam = parameters.getRawParameterValue ("lowfreq");
    highFreqParam = parameters.getRawParameterValue ("highfreq");
    smoothParam = parameters.getRawParameterValue ("smooth");
    soloParam = parameters.getRawParameterValue ("solo");
    bypassParam = parameters.getRawParameterValue ("bypass");
}

TrackspacerAudioProcessor::~TrackspacerAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout TrackspacerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Amount (0-100%)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "amount", 1 },
        "Amount",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " %"; }));

    // Attack (1-1000ms)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "attack", 1 },
        "Attack",
        juce::NormalisableRange<float> (1.0f, 1000.0f, 1.0f, 0.3f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 0) + " ms"; }));

    // Release (10-5000ms)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "release", 1 },
        "Release",
        juce::NormalisableRange<float> (10.0f, 5000.0f, 1.0f, 0.3f),
        500.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 0) + " ms"; }));

    // Low Freq (20-20000Hz)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "lowfreq", 1 },
        "Low Freq",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f),
        20.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value >= 1000.0f)
                return juce::String (value / 1000.0f, 1) + " kHz";
            return juce::String (value, 0) + " Hz";
        }));

    // High Freq (20-20000Hz)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "highfreq", 1 },
        "High Freq",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f),
        20000.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value >= 1000.0f)
                return juce::String (value / 1000.0f, 1) + " kHz";
            return juce::String (value, 0) + " Hz";
        }));

    // Smoothing (0-100%)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "smooth", 1 },
        "Smooth",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " %"; }));

    // Solo sidechain
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "solo", 1 },
        "Solo",
        false));

    // Bypass
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "bypass", 1 },
        "Bypass",
        false));

    return layout;
}

//==============================================================================
const juce::String TrackspacerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TrackspacerAudioProcessor::acceptsMidi() const
{
    return false;
}

bool TrackspacerAudioProcessor::producesMidi() const
{
    return false;
}

bool TrackspacerAudioProcessor::isMidiEffect() const
{
    return false;
}

double TrackspacerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TrackspacerAudioProcessor::getNumPrograms()
{
    return 1;
}

int TrackspacerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TrackspacerAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String TrackspacerAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void TrackspacerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void TrackspacerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spectralEngine.prepare (sampleRate, samplesPerBlock);
}

void TrackspacerAudioProcessor::releaseResources()
{
    spectralEngine.reset();
}

bool TrackspacerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Main input/output must be stereo and match
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo() ||
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Sidechain input must be stereo
    if (layouts.getChannelSet (true, 1) != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void TrackspacerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Check if bypassed
    if (bypassParam->load() > 0.5f)
        return;

    // Get buffers
    auto mainBuffer = getBusBuffer (buffer, true, 0);
    auto sidechainBuffer = getBusBuffer (buffer, true, 1);

    // Check for valid buffers
    if (mainBuffer.getNumChannels() == 0 || sidechainBuffer.getNumChannels() == 0)
        return;

    // Solo mode - output sidechain directly
    if (soloParam->load() > 0.5f)
    {
        for (int ch = 0; ch < mainBuffer.getNumChannels(); ++ch)
        {
            if (ch < sidechainBuffer.getNumChannels())
                mainBuffer.copyFrom (ch, 0, sidechainBuffer, ch, 0, buffer.getNumSamples());
            else
                mainBuffer.clear (ch, 0, buffer.getNumSamples());
        }
        return;
    }

    // Get current parameters
    float amount = amountParam->load() / 100.0f;
    float attack = attackParam->load();
    float release = releaseParam->load();
    float lowFreq = lowFreqParam->load();
    float highFreq = highFreqParam->load();
    float smooth = smoothParam->load() / 100.0f;

    // Process spectral ducking
    spectralEngine.process (mainBuffer, sidechainBuffer, amount, attack, release,
                           lowFreq, highFreq, smooth);
}

//==============================================================================
bool TrackspacerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TrackspacerAudioProcessor::createEditor()
{
    return new TrackspacerAudioProcessorEditor (*this);
}

//==============================================================================
void TrackspacerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TrackspacerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TrackspacerAudioProcessor();
}
