add_library(FSCore STATIC
    RateLevelEnvelope.h
    RateLevelEnvelope.cpp
    FMOperator.h
    FMOperator.cpp
    FormantGenerator.h
    FormantGenerator.cpp
    NoiseSource.h
    MultiModeFilter.h
    MultiModeFilter.cpp
    Lfo.h
    Lfo.cpp
    Drive.h
    Drive.cpp
    DspUtils.h
)

target_include_directories(FSCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(FSCore
    PUBLIC
        juce::juce_audio_basics
        juce::juce_dsp
    PRIVATE
        juce::juce_recommended_config_flags
)

target_compile_features(FSCore PUBLIC cxx_std_17)
