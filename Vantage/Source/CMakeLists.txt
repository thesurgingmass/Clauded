juce_add_plugin(Vantage
    COMPANY_NAME                "${VANTAGE_COMPANY_NAME}"
    PLUGIN_MANUFACTURER_CODE    "${VANTAGE_COMPANY_CODE}"
    PLUGIN_CODE                 "Vtg1"
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Vantage"
    BUNDLE_ID                   "com.claudedaudio.vantage"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES             "${VANTAGE_VST3_CATEGORY}"
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

target_sources(Vantage PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
    Parameters.cpp
    Parameters.h
    DSP/Constants.cpp
    DSP/Constants.h
    DSP/VoiceParameters.h
    DSP/NoiseGenerator.cpp
    DSP/NoiseGenerator.h
    DSP/PolyBlep.h
    DSP/Wavetable.cpp
    DSP/Wavetable.h
    DSP/OscillatorEngine.cpp
    DSP/OscillatorEngine.h
    DSP/FilterEngine.cpp
    DSP/FilterEngine.h
    DSP/ExponentialRamp.h
    DSP/EnvelopeEngine.cpp
    DSP/EnvelopeEngine.h
    DSP/LFOEngine.cpp
    DSP/LFOEngine.h
    DSP/ModulationMatrix.cpp
    DSP/ModulationMatrix.h
    DSP/SynthSound.h
    DSP/SynthVoice.cpp
    DSP/SynthVoice.h
    DSP/VantageSynthesiser.cpp
    DSP/VantageSynthesiser.h
    DSP/FXEngine.cpp
    DSP/FXEngine.h
)

target_include_directories(Vantage PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(Vantage PUBLIC
    JUCE_ALSA=1
)

target_link_libraries(Vantage
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
