cmake_minimum_required(VERSION 3.22)

# ---------------------------------------------------------------------------
# Machinedrum-inspired drum synthesis engines, each shipped as an individual
# VST3 instrument. Targets macOS (Intel/x86_64) by default; also builds on
# Linux for development/verification.
# ---------------------------------------------------------------------------

project(MachinedrumVSTs VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE AND NOT DEFINED CMAKE_OSX_ARCHITECTURES)
    # Intel-only build, per project requirement.
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for macOS" FORCE)
endif()

if(APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment target" FORCE)
endif()

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

add_subdirectory(shared)
add_subdirectory(machines)
