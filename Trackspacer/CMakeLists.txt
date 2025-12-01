cmake_minimum_required(VERSION 3.22)

project(TRACKSPACER VERSION 1.0.0)

# Add JUCE as a subdirectory (it's in the parent directory)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# juce_add_plugin creates a static library target with the name passed as the first argument
juce_add_plugin(Trackspacer
    COMPANY_NAME "FER Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Fera
    PLUGIN_CODE Trsp
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Trackspacer")

target_sources(Trackspacer
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/SpectralEngine.cpp
        Source/SpectrumAnalyzer.cpp)

target_compile_definitions(Trackspacer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_USE_DARK_SPLASH_SCREEN=1)

target_link_libraries(Trackspacer
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
