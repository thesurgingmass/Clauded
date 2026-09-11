add_library(MDShared STATIC
    src/MDLookAndFeel.cpp
    src/MachineEditor.cpp
    src/MachineProcessor.cpp
)

target_include_directories(MDShared PUBLIC include)

target_link_libraries(MDShared PUBLIC
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
)

target_compile_definitions(MDShared PUBLIC
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
