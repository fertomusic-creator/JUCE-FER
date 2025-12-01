#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TrackspacerAudioProcessorEditor::TrackspacerAudioProcessorEditor (TrackspacerAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Set up title
    titleLabel.setText ("TRACKSPACER", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (32.0f));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);

    // Set up spectrum analyzer
    addAndMakeVisible (spectrumAnalyzer);

    // Set up sliders
    setupSlider (amountSlider, amountLabel, "AMOUNT");
    setupSlider (attackSlider, attackLabel, "ATTACK");
    setupSlider (releaseSlider, releaseLabel, "RELEASE");
    setupSlider (lowFreqSlider, lowFreqLabel, "LOW FREQ");
    setupSlider (highFreqSlider, highFreqLabel, "HIGH FREQ");
    setupSlider (smoothSlider, smoothLabel, "SMOOTH");

    // Set up buttons
    setupButton (soloButton);
    soloButton.setButtonText ("SOLO");
    soloButton.setClickingTogglesState (true);
    addAndMakeVisible (soloButton);

    setupButton (bypassButton);
    bypassButton.setButtonText ("BYPASS");
    bypassButton.setClickingTogglesState (true);
    addAndMakeVisible (bypassButton);

    // Create attachments
    auto& apvts = processorRef.getValueTreeState();

    amountAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
        apvts, "amount", amountSlider));
    attackAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
        apvts, "attack", attackSlider));
    releaseAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
        apvts, "release", releaseSlider));
    lowFreqAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
        apvts, "lowfreq", lowFreqSlider));
    highFreqAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
        apvts, "highfreq", highFreqSlider));
    smoothAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
        apvts, "smooth", smoothSlider));
    soloAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (
        apvts, "solo", soloButton));
    bypassAttachment.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (
        apvts, "bypass", bypassButton));

    // Start timer for updating spectrum display
    startTimerHz (30);

    // Set window size
    setSize (800, 600);
    setResizable (true, true);
    setResizeLimits (600, 400, 1600, 1200);
}

TrackspacerAudioProcessorEditor::~TrackspacerAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TrackspacerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill background with gradient
    g.fillAll (juce::Colour (0xff0a0a0a));

    auto bounds = getLocalBounds();

    // Draw a subtle gradient
    juce::ColourGradient gradient (juce::Colour (0xff1a1a1a), 0, 0,
                                   juce::Colour (0xff0a0a0a), 0, (float) bounds.getHeight(),
                                   false);
    g.setGradientFill (gradient);
    g.fillRect (bounds);
}

void TrackspacerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int margin = 20;
    const int controlHeight = 80;
    const int buttonHeight = 40;

    // Title
    titleLabel.setBounds (bounds.removeFromTop (60).reduced (margin));

    // Buttons at the top right
    auto buttonArea = bounds.removeFromTop (buttonHeight + margin).reduced (margin);
    auto buttonWidth = 100;
    bypassButton.setBounds (buttonArea.removeFromRight (buttonWidth));
    buttonArea.removeFromRight (10);
    soloButton.setBounds (buttonArea.removeFromRight (buttonWidth));

    // Spectrum analyzer
    auto spectrumHeight = bounds.getHeight() - (controlHeight * 2) - margin * 3;
    spectrumAnalyzer.setBounds (bounds.removeFromTop (spectrumHeight).reduced (margin));

    bounds.removeFromTop (margin);

    // First row of controls
    auto controlRow1 = bounds.removeFromTop (controlHeight).reduced (margin);
    int controlWidth = controlRow1.getWidth() / 3 - 10;

    auto amountArea = controlRow1.removeFromLeft (controlWidth);
    amountLabel.setBounds (amountArea.removeFromTop (20));
    amountSlider.setBounds (amountArea);

    controlRow1.removeFromLeft (10);

    auto attackArea = controlRow1.removeFromLeft (controlWidth);
    attackLabel.setBounds (attackArea.removeFromTop (20));
    attackSlider.setBounds (attackArea);

    controlRow1.removeFromLeft (10);

    auto releaseArea = controlRow1.removeFromLeft (controlWidth);
    releaseLabel.setBounds (releaseArea.removeFromTop (20));
    releaseSlider.setBounds (releaseArea);

    bounds.removeFromTop (margin);

    // Second row of controls
    auto controlRow2 = bounds.removeFromTop (controlHeight).reduced (margin);
    controlWidth = controlRow2.getWidth() / 3 - 10;

    auto lowFreqArea = controlRow2.removeFromLeft (controlWidth);
    lowFreqLabel.setBounds (lowFreqArea.removeFromTop (20));
    lowFreqSlider.setBounds (lowFreqArea);

    controlRow2.removeFromLeft (10);

    auto highFreqArea = controlRow2.removeFromLeft (controlWidth);
    highFreqLabel.setBounds (highFreqArea.removeFromTop (20));
    highFreqSlider.setBounds (highFreqArea);

    controlRow2.removeFromLeft (10);

    auto smoothArea = controlRow2.removeFromLeft (controlWidth);
    smoothLabel.setBounds (smoothArea.removeFromTop (20));
    smoothSlider.setBounds (smoothArea);
}

//==============================================================================
void TrackspacerAudioProcessorEditor::timerCallback()
{
    // Update spectrum analyzer with latest data
    spectrumAnalyzer.setInputSpectrum (processorRef.getInputSpectrum());
    spectrumAnalyzer.setSidechainSpectrum (processorRef.getSidechainSpectrum());
    spectrumAnalyzer.setGainReduction (processorRef.getGainReduction());
}

//==============================================================================
void TrackspacerAudioProcessorEditor::setupSlider (juce::Slider& slider,
                                                   juce::Label& label,
                                                   const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    slider.setColour (juce::Slider::thumbColourId, juce::Colour (0xff4a9eff));
    slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff4a9eff));
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff2a2a2a));
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff1a1a1a));
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff3a3a3a));
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colours::white);
    label.setFont (juce::FontOptions (14.0f));
    addAndMakeVisible (label);
}

void TrackspacerAudioProcessorEditor::setupButton (juce::TextButton& button)
{
    button.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2a2a));
    button.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff4a9eff));
    button.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    button.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}
