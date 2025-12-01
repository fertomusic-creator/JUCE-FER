#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

//==============================================================================
/**
    The SpectralEngine class handles all FFT-based spectral processing.
    It analyzes both the input and sidechain signals, compares their spectra,
    and applies dynamic per-band gain reduction to create space in the input
    signal where the sidechain signal has energy.
*/
class SpectralEngine
{
public:
    SpectralEngine();
    ~SpectralEngine();

    //==============================================================================
    void prepare (double sampleRate, int samplesPerBlock);
    void reset();

    //==============================================================================
    void process (juce::AudioBuffer<float>& mainBuffer,
                  juce::AudioBuffer<float>& sidechainBuffer,
                  float amount,
                  float attackMs,
                  float releaseMs,
                  float lowFreq,
                  float highFreq,
                  float smoothing);

    //==============================================================================
    // Access to spectral data for visualization
    const std::vector<float>& getInputSpectrum() const { return inputSpectrum; }
    const std::vector<float>& getSidechainSpectrum() const { return sidechainSpectrum; }
    const std::vector<float>& getGainReduction() const { return gainReduction; }

private:
    //==============================================================================
    static constexpr int fftOrder = 11;  // 2048 samples
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int hopSize = fftSize / 4;  // 75% overlap

    //==============================================================================
    struct ChannelProcessor
    {
        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float>> window;

        std::vector<float> fftData;
        std::vector<float> timeDomainBuffer;
        std::vector<float> overlapBuffer;

        int inputPos = 0;

        ChannelProcessor()
        {
            fft = std::make_unique<juce::dsp::FFT> (fftOrder);
            window = std::make_unique<juce::dsp::WindowingFunction<float>> (fftSize, juce::dsp::WindowingFunction<float>::hann);
            fftData.resize (fftSize * 2, 0.0f);
            timeDomainBuffer.resize (fftSize, 0.0f);
            overlapBuffer.resize (fftSize, 0.0f);
        }

        void reset()
        {
            std::fill (fftData.begin(), fftData.end(), 0.0f);
            std::fill (timeDomainBuffer.begin(), timeDomainBuffer.end(), 0.0f);
            std::fill (overlapBuffer.begin(), overlapBuffer.end(), 0.0f);
            inputPos = 0;
        }
    };

    std::vector<ChannelProcessor> mainProcessors;
    std::vector<ChannelProcessor> sidechainProcessors;

    //==============================================================================
    // Spectral data
    std::vector<float> inputSpectrum;
    std::vector<float> sidechainSpectrum;
    std::vector<float> gainReduction;
    std::vector<float> envelopeFollowers;  // Per-band envelope followers for attack/release

    //==============================================================================
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;

    //==============================================================================
    void processFFTFrame (ChannelProcessor& mainProc,
                         ChannelProcessor& scProc,
                         float amount,
                         float attackCoeff,
                         float releaseCoeff,
                         int lowBin,
                         int highBin,
                         float smoothing);

    void computeMagnitudeSpectrum (const std::vector<float>& fftData,
                                  std::vector<float>& spectrum);

    float computeGainReduction (float inputMag, float sidechainMag, float amount);

    void applySmoothingToSpectrum (std::vector<float>& spectrum, float smoothing);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralEngine)
};
