juce_add_plugin(FMTone
    COMPANY_NAME                "${FM_SYNTHS_COMPANY_NAME}"
    PLUGIN_MANUFACTURER_CODE    "${FM_SYNTHS_COMPANY_CODE}"
    PLUGIN_CODE                 "Fmtn"
    FORMATS                     VST3
    PRODUCT_NAME                "FM Tone"
    BUNDLE_ID                   "com.claudedaudio.fmtone"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES             "${FM_SYNTHS_VST3_CATEGORY}"
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

target_sources(FMTone PRIVATE
    FMToneVoice.cpp
    FMToneVoice.h
    FMToneSound.h
    FMAlgorithms.h
    Parameters.cpp
    Parameters.h
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
)

target_include_directories(FMTone PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(FMTone PUBLIC
    JUCE_ALSA=1
)

target_link_libraries(FMTone
    PRIVATE
        FMCore
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
