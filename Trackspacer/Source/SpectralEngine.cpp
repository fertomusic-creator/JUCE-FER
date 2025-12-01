#include "SpectralEngine.h"

//==============================================================================
SpectralEngine::SpectralEngine()
{
    inputSpectrum.resize (fftSize / 2, 0.0f);
    sidechainSpectrum.resize (fftSize / 2, 0.0f);
    gainReduction.resize (fftSize / 2, 1.0f);
    envelopeFollowers.resize (fftSize / 2, 1.0f);
}

SpectralEngine::~SpectralEngine()
{
}

//==============================================================================
void SpectralEngine::prepare (double newSampleRate, int newSamplesPerBlock)
{
    sampleRate = newSampleRate;
    samplesPerBlock = newSamplesPerBlock;

    // Create processors for stereo (2 channels)
    mainProcessors.resize (2);
    sidechainProcessors.resize (2);

    reset();
}

void SpectralEngine::reset()
{
    for (auto& proc : mainProcessors)
        proc.reset();

    for (auto& proc : sidechainProcessors)
        proc.reset();

    std::fill (inputSpectrum.begin(), inputSpectrum.end(), 0.0f);
    std::fill (sidechainSpectrum.begin(), sidechainSpectrum.end(), 0.0f);
    std::fill (gainReduction.begin(), gainReduction.end(), 1.0f);
    std::fill (envelopeFollowers.begin(), envelopeFollowers.end(), 1.0f);
}

//==============================================================================
void SpectralEngine::process (juce::AudioBuffer<float>& mainBuffer,
                              juce::AudioBuffer<float>& sidechainBuffer,
                              float amount,
                              float attackMs,
                              float releaseMs,
                              float lowFreq,
                              float highFreq,
                              float smoothing)
{
    const int numSamples = mainBuffer.getNumSamples();
    const int numChannels = juce::jmin (mainBuffer.getNumChannels(), 2);

    // Calculate attack and release coefficients
    float attackCoeff = 1.0f - std::exp (-1.0f / (attackMs * 0.001f * (float) sampleRate));
    float releaseCoeff = 1.0f - std::exp (-1.0f / (releaseMs * 0.001f * (float) sampleRate));

    // Convert frequency range to bin indices
    int lowBin = juce::roundToInt ((lowFreq / (float) sampleRate) * fftSize);
    int highBin = juce::roundToInt ((highFreq / (float) sampleRate) * fftSize);
    lowBin = juce::jlimit (0, fftSize / 2, lowBin);
    highBin = juce::jlimit (lowBin + 1, fftSize / 2, highBin);

    // Process each channel
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* mainData = mainBuffer.getWritePointer (ch);
        auto* scData = sidechainBuffer.getReadPointer (ch);

        auto& mainProc = mainProcessors[ch];
        auto& scProc = sidechainProcessors[ch];

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Store input samples
            mainProc.timeDomainBuffer[mainProc.inputPos] = mainData[sample];
            scProc.timeDomainBuffer[scProc.inputPos] = scData[sample];

            mainProc.inputPos++;

            // Process when we have enough samples
            if (mainProc.inputPos >= hopSize)
            {
                mainProc.inputPos = 0;

                // Copy to FFT buffer and apply window
                for (int i = 0; i < fftSize; ++i)
                {
                    mainProc.fftData[i] = mainProc.timeDomainBuffer[i];
                    scProc.fftData[i] = scProc.timeDomainBuffer[i];
                }

                mainProc.window->multiplyWithWindowingTable (mainProc.fftData.data(), fftSize);
                scProc.window->multiplyWithWindowingTable (scProc.fftData.data(), fftSize);

                // Perform forward FFT
                mainProc.fft->performFrequencyOnlyForwardTransform (mainProc.fftData.data());
                scProc.fft->performFrequencyOnlyForwardTransform (scProc.fftData.data());

                // Process the FFT frame
                processFFTFrame (mainProc, scProc, amount, attackCoeff, releaseCoeff,
                               lowBin, highBin, smoothing);

                // Shift the time domain buffer
                std::copy (mainProc.timeDomainBuffer.begin() + hopSize,
                          mainProc.timeDomainBuffer.end(),
                          mainProc.timeDomainBuffer.begin());
                std::copy (scProc.timeDomainBuffer.begin() + hopSize,
                          scProc.timeDomainBuffer.end(),
                          scProc.timeDomainBuffer.begin());
            }
        }
    }

    // For now, we'll implement a simpler time-domain version
    // A full spectral implementation would require complex FFT processing with phase preservation
    // This is a simplified version that demonstrates the concept

    // Calculate RMS of sidechain in frequency bands (simplified)
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* mainData = mainBuffer.getWritePointer (ch);
        const auto* scData = sidechainBuffer.getReadPointer (ch);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Simple envelope following based on sidechain level
            float scLevel = std::abs (scData[sample]);
            float targetGain = 1.0f - (scLevel * amount);
            targetGain = juce::jlimit (0.0f, 1.0f, targetGain);

            // Apply attack/release smoothing
            float coeff = (targetGain < envelopeFollowers[0]) ? attackCoeff : releaseCoeff;
            envelopeFollowers[0] += coeff * (targetGain - envelopeFollowers[0]);

            // Apply gain reduction
            mainData[sample] *= envelopeFollowers[0];
        }
    }
}

//==============================================================================
void SpectralEngine::processFFTFrame (ChannelProcessor& mainProc,
                                     ChannelProcessor& scProc,
                                     float amount,
                                     float attackCoeff,
                                     float releaseCoeff,
                                     int lowBin,
                                     int highBin,
                                     float smoothing)
{
    // Compute magnitude spectra
    computeMagnitudeSpectrum (mainProc.fftData, inputSpectrum);
    computeMagnitudeSpectrum (scProc.fftData, sidechainSpectrum);

    // Apply spectral smoothing
    if (smoothing > 0.0f)
    {
        applySmoothingToSpectrum (inputSpectrum, smoothing);
        applySmoothingToSpectrum (sidechainSpectrum, smoothing);
    }

    // Calculate gain reduction for each frequency bin
    for (int bin = 0; bin < fftSize / 2; ++bin)
    {
        float targetGain = 1.0f;

        // Only process bins within the specified frequency range
        if (bin >= lowBin && bin <= highBin)
        {
            targetGain = computeGainReduction (inputSpectrum[bin], sidechainSpectrum[bin], amount);
        }

        // Apply attack/release envelope following
        float coeff = (targetGain < envelopeFollowers[bin]) ? attackCoeff : releaseCoeff;
        envelopeFollowers[bin] += coeff * (targetGain - envelopeFollowers[bin]);

        // Store the gain reduction for visualization
        gainReduction[bin] = envelopeFollowers[bin];

        // Apply gain reduction to the FFT bins
        // Note: In a complete implementation, we would apply this to the complex FFT data
        // and perform inverse FFT. For simplicity, this is a placeholder.
    }
}

//==============================================================================
void SpectralEngine::computeMagnitudeSpectrum (const std::vector<float>& fftData,
                                              std::vector<float>& spectrum)
{
    for (int i = 0; i < fftSize / 2; ++i)
    {
        spectrum[i] = fftData[i];  // JUCE's performFrequencyOnlyForwardTransform gives magnitudes
    }
}

//==============================================================================
float SpectralEngine::computeGainReduction (float inputMag, float sidechainMag, float amount)
{
    if (sidechainMag < 0.0001f)
        return 1.0f;  // No reduction if sidechain is silent

    // Calculate how much to reduce based on sidechain presence
    float ratio = sidechainMag / juce::jmax (inputMag, 0.0001f);
    ratio = juce::jlimit (0.0f, 1.0f, ratio);

    // Apply amount parameter
    float reduction = 1.0f - (ratio * amount);
    return juce::jlimit (0.0f, 1.0f, reduction);
}

//==============================================================================
void SpectralEngine::applySmoothingToSpectrum (std::vector<float>& spectrum, float smoothing)
{
    if (smoothing < 0.01f)
        return;

    // Simple moving average smoothing across frequency bins
    int smoothWidth = juce::roundToInt (smoothing * 10.0f);  // 0-10 bins
    smoothWidth = juce::jlimit (1, 20, smoothWidth);

    std::vector<float> smoothed (spectrum.size());

    for (size_t i = 0; i < spectrum.size(); ++i)
    {
        float sum = 0.0f;
        int count = 0;

        for (int offset = -smoothWidth; offset <= smoothWidth; ++offset)
        {
            int index = (int) i + offset;
            if (index >= 0 && index < (int) spectrum.size())
            {
                sum += spectrum[index];
                count++;
            }
        }

        smoothed[i] = sum / (float) count;
    }

    spectrum = smoothed;
}
