juce_add_plugin(FormantOne
    COMPANY_NAME                "${FORMANT_ONE_COMPANY_NAME}"
    PLUGIN_MANUFACTURER_CODE    "${FORMANT_ONE_COMPANY_CODE}"
    PLUGIN_CODE                 "Fmn1"
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Formant One"
    BUNDLE_ID                   "com.claudedaudio.formantone"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES             "${FORMANT_ONE_VST3_CATEGORY}"
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

target_sources(FormantOne PRIVATE
    FMAlgorithms.h
    FMAlgorithms.cpp
    FormantOneVoice.h
    FormantOneVoice.cpp
    FormantOneSound.h
    FormantOneLookAndFeel.h
    FormantOneLookAndFeel.cpp
    Parameters.h
    Parameters.cpp
    PluginProcessor.h
    PluginProcessor.cpp
    PluginEditor.h
    PluginEditor.cpp
)

target_include_directories(FormantOne PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(FormantOne PUBLIC
    JUCE_ALSA=1
)

target_link_libraries(FormantOne
    PRIVATE
        FSCore
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
