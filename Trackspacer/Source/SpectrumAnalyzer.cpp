#include "SpectrumAnalyzer.h"

//==============================================================================
SpectrumAnalyzer::SpectrumAnalyzer()
{
    startTimerHz (60);  // 60 fps refresh rate
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    stopTimer();
}

//==============================================================================
void SpectrumAnalyzer::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Fill background
    g.fillAll (juce::Colour (0xff1a1a1a));

    // Draw grid lines
    g.setColour (juce::Colour (0xff2a2a2a));

    // Horizontal lines (dB grid)
    for (int db = -60; db <= 0; db += 10)
    {
        float y = bounds.getHeight() * (1.0f - (db + 60.0f) / 60.0f);
        g.drawLine (bounds.getX(), y, bounds.getRight(), y, 1.0f);
    }

    // Vertical lines (frequency grid)
    const float frequencies[] = { 100.0f, 1000.0f, 10000.0f };
    for (float freq : frequencies)
    {
        float normFreq = std::log10 (freq / 20.0f) / std::log10 (20000.0f / 20.0f);
        float x = bounds.getX() + bounds.getWidth() * normFreq;
        g.drawLine (x, bounds.getY(), x, bounds.getBottom(), 1.0f);

        // Draw frequency labels
        g.setColour (juce::Colour (0xff4a4a4a));
        juce::String label = freq >= 1000.0f ? juce::String (freq / 1000.0f, 1) + "k"
                                              : juce::String ((int) freq);
        g.drawText (label, (int) x - 20, (int) bounds.getBottom() - 20, 40, 15,
                   juce::Justification::centred);
    }

    // Draw spectra
    if (!sidechainSpectrum.empty())
        drawSpectrum (g, sidechainSpectrum, sidechainColour, 0.6f);

    if (!inputSpectrum.empty())
        drawSpectrum (g, inputSpectrum, inputColour, 0.8f);

    if (!gainReduction.empty())
        drawSpectrum (g, gainReduction, reductionColour, 0.9f);

    // Draw border
    g.setColour (juce::Colour (0xff3a3a3a));
    g.drawRect (bounds, 2.0f);
}

void SpectrumAnalyzer::resized()
{
}

//==============================================================================
void SpectrumAnalyzer::setInputSpectrum (const std::vector<float>& spectrum)
{
    inputSpectrum = spectrum;
}

void SpectrumAnalyzer::setSidechainSpectrum (const std::vector<float>& spectrum)
{
    sidechainSpectrum = spectrum;
}

void SpectrumAnalyzer::setGainReduction (const std::vector<float>& reduction)
{
    gainReduction = reduction;
}

//==============================================================================
void SpectrumAnalyzer::timerCallback()
{
    repaint();
}

//==============================================================================
void SpectrumAnalyzer::drawSpectrum (juce::Graphics& g,
                                    const std::vector<float>& spectrum,
                                    juce::Colour colour,
                                    float alpha)
{
    if (spectrum.empty())
        return;

    auto bounds = getLocalBounds().toFloat();
    juce::Path path;

    bool firstPoint = true;
    const int numBins = (int) spectrum.size();

    for (int bin = 1; bin < numBins; ++bin)  // Start from 1 to skip DC
    {
        float x = binToX (bin, numBins);

        // Convert magnitude to dB
        float magnitude = spectrum[bin];
        float db = magnitude > 0.0001f ? 20.0f * std::log10 (magnitude) : -60.0f;
        db = juce::jlimit (-60.0f, 0.0f, db);

        float y = magnitudeToY (db);

        if (firstPoint)
        {
            path.startNewSubPath (bounds.getX() + x * bounds.getWidth(),
                                 bounds.getY() + y * bounds.getHeight());
            firstPoint = false;
        }
        else
        {
            path.lineTo (bounds.getX() + x * bounds.getWidth(),
                        bounds.getY() + y * bounds.getHeight());
        }
    }

    // Draw the spectrum line
    g.setColour (colour.withAlpha (alpha));
    g.strokePath (path, juce::PathStrokeType (2.0f));

    // Fill below the curve
    path.lineTo (bounds.getRight(), bounds.getBottom());
    path.lineTo (bounds.getX(), bounds.getBottom());
    path.closeSubPath();

    g.setColour (colour.withAlpha (alpha * 0.2f));
    g.fillPath (path);
}

//==============================================================================
float SpectrumAnalyzer::binToX (int bin, int numBins) const
{
    // Logarithmic frequency scale
    float minFreq = 20.0f;
    float maxFreq = 20000.0f;

    // Assuming sample rate of 44100, Nyquist = 22050
    float freq = (bin / (float) numBins) * 22050.0f;

    if (freq < minFreq)
        freq = minFreq;
    if (freq > maxFreq)
        freq = maxFreq;

    float normFreq = std::log10 (freq / minFreq) / std::log10 (maxFreq / minFreq);
    return juce::jlimit (0.0f, 1.0f, normFreq);
}

float SpectrumAnalyzer::magnitudeToY (float db) const
{
    // Map -60dB to 0dB range to 0.0 to 1.0
    float normalized = (db + 60.0f) / 60.0f;
    return 1.0f - juce::jlimit (0.0f, 1.0f, normalized);
}
