#pragma once

#include "PluginProcessor.h"
#include "SpectrumAnalyzer.h"

//==============================================================================
class TrackspacerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit TrackspacerAudioProcessorEditor (TrackspacerAudioProcessor&);
    ~TrackspacerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    TrackspacerAudioProcessor& processorRef;

    // UI Components
    SpectrumAnalyzer spectrumAnalyzer;

    // Sliders
    juce::Slider amountSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider lowFreqSlider;
    juce::Slider highFreqSlider;
    juce::Slider smoothSlider;

    // Labels
    juce::Label amountLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label lowFreqLabel;
    juce::Label highFreqLabel;
    juce::Label smoothLabel;

    // Buttons
    juce::TextButton soloButton;
    juce::TextButton bypassButton;

    // Title
    juce::Label titleLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> smoothAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    //==============================================================================
    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& labelText);
    void setupButton (juce::TextButton& button);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackspacerAudioProcessorEditor)
};
