# MDShared is header-only at the CMake level: the actual .cpp files are
# compiled once per plugin target (see cmake/AddMachine.cmake), because
# JUCE's per-target Linux dependency wiring (GTK for juce_gui_extra, etc.)
# is only applied to targets created via juce_add_plugin -- a separate
# static library built from these sources wouldn't receive it.
add_library(MDShared INTERFACE)

target_include_directories(MDShared INTERFACE include)

target_link_libraries(MDShared INTERFACE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
)

set(MD_SHARED_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MDLookAndFeel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineEditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MachineProcessor.cpp
    PARENT_SCOPE
)
