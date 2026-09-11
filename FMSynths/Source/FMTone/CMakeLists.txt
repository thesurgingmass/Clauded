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

if(APPLE)
    # JUCE's own post-build steps ad-hoc sign the .vst3 bundle and *then*
    # regenerate Contents/Resources/moduleinfo.json, which invalidates that
    # signature's sealed resource manifest (the file changed after signing
    # sealed it). Hosts that enforce signature integrity - Ableton Live does,
    # Finder/`file`/`codesign --verify` without --deep don't, which made this
    # easy to miss - reject the result with "a sealed resource is missing or
    # invalid" / "not a plugin". Re-sign ad-hoc one more time, after
    # everything else, so the seal covers the bundle's final contents.
    add_custom_command(TARGET FMTone_VST3 POST_BUILD
        COMMAND codesign --force --deep -s - "$<TARGET_BUNDLE_DIR:FMTone_VST3>"
        COMMENT "Re-signing FM Tone.vst3 ad-hoc after moduleinfo.json regeneration"
        VERBATIM)
endif()
