#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
    SpectrumAnalyzer is a component that displays frequency spectrum data.
    It shows the input signal, sidechain signal, and the gain reduction curve.
*/
class SpectrumAnalyzer : public juce::Component,
                         private juce::Timer
{
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void setInputSpectrum (const std::vector<float>& spectrum);
    void setSidechainSpectrum (const std::vector<float>& spectrum);
    void setGainReduction (const std::vector<float>& reduction);

private:
    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    std::vector<float> inputSpectrum;
    std::vector<float> sidechainSpectrum;
    std::vector<float> gainReduction;

    juce::Colour inputColour { juce::Colours::cyan };
    juce::Colour sidechainColour { juce::Colours::orange };
    juce::Colour reductionColour { juce::Colours::red };

    //==============================================================================
    void drawSpectrum (juce::Graphics& g,
                      const std::vector<float>& spectrum,
                      juce::Colour colour,
                      float alpha = 1.0f);

    float binToX (int bin, int numBins) const;
    float magnitudeToY (float magnitude) const;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzer)
};
