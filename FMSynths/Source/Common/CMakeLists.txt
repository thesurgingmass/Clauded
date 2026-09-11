add_library(FMCore STATIC
    FMOperator.h
    FMOperator.cpp
    MultiModeFilter.h
    MultiModeFilter.cpp
    Drive.h
    Drive.cpp
    DspUtils.h
    FMLookAndFeel.h
    FMLookAndFeel.cpp
)

target_include_directories(FMCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(FMCore
    PUBLIC
        juce::juce_audio_basics
        juce::juce_dsp
        juce::juce_gui_basics
    PRIVATE
        juce::juce_recommended_config_flags
)

target_compile_features(FMCore PUBLIC cxx_std_17)
