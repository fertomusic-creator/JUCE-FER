#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SpectralEngine.h"

//==============================================================================
class TrackspacerAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    TrackspacerAudioProcessor();
    ~TrackspacerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Access to parameters
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

    // Access to spectral data for visualization
    const std::vector<float>& getInputSpectrum() const { return spectralEngine.getInputSpectrum(); }
    const std::vector<float>& getSidechainSpectrum() const { return spectralEngine.getSidechainSpectrum(); }
    const std::vector<float>& getGainReduction() const { return spectralEngine.getGainReduction(); }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;

    // Parameter pointers
    std::atomic<float>* amountParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* lowFreqParam = nullptr;
    std::atomic<float>* highFreqParam = nullptr;
    std::atomic<float>* smoothParam = nullptr;
    std::atomic<float>* soloParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;

    // Spectral processing engine
    SpectralEngine spectralEngine;

    // Create parameter layout
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackspacerAudioProcessor)
};
