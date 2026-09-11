# Helper to declare one Machinedrum-style machine as its own VST3 instrument.
function(add_md_machine TARGET_NAME PRODUCT_NAME PLUGIN_CODE)
    juce_add_plugin(${TARGET_NAME}
        COMPANY_NAME "Independent"
        PLUGIN_MANUFACTURER_CODE Mdcl
        PLUGIN_CODE ${PLUGIN_CODE}
        FORMATS VST3
        PRODUCT_NAME "${PRODUCT_NAME}"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        VST3_CATEGORIES Instrument Drum
        COPY_PLUGIN_AFTER_BUILD FALSE
    )

    target_sources(${TARGET_NAME} PRIVATE ${ARGN})

    target_link_libraries(${TARGET_NAME} PRIVATE
        MDShared
        juce::juce_audio_utils
        juce::juce_dsp
    )

    target_compile_definitions(${TARGET_NAME} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
    )
endfunction()
