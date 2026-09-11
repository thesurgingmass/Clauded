juce_add_plugin(FMDrum
    COMPANY_NAME                "${FM_SYNTHS_COMPANY_NAME}"
    PLUGIN_MANUFACTURER_CODE    "${FM_SYNTHS_COMPANY_CODE}"
    PLUGIN_CODE                 "Fmdr"
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "FM Drum"
    BUNDLE_ID                   "com.claudedaudio.fmdrum"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES             "${FM_SYNTHS_VST3_CATEGORY}"
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

target_sources(FMDrum PRIVATE
    FMDrumVoice.cpp
    FMDrumVoice.h
    FMDrumSound.h
    Parameters.cpp
    Parameters.h
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
)

target_include_directories(FMDrum PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(FMDrum PUBLIC
    JUCE_ALSA=1
)

target_link_libraries(FMDrum
    PRIVATE
        FMCore
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(APPLE)
    # See the matching comment in Source/FMTone/CMakeLists.txt: JUCE signs the
    # bundle and then rewrites moduleinfo.json inside it afterwards, which
    # invalidates that signature. Re-sign ad-hoc one more time, last.
    add_custom_command(TARGET FMDrum_VST3 POST_BUILD
        COMMAND codesign --force --deep -s - "$<TARGET_BUNDLE_DIR:FMDrum_VST3>"
        COMMENT "Re-signing FM Drum.vst3 ad-hoc after moduleinfo.json regeneration"
        VERBATIM)

    add_custom_command(TARGET FMDrum_Standalone POST_BUILD
        COMMAND codesign --force --deep -s - "$<TARGET_BUNDLE_DIR:FMDrum_Standalone>"
        COMMENT "Re-signing FM Drum.app ad-hoc after moduleinfo.json regeneration"
        VERBATIM)
endif()
