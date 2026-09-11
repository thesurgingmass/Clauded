cmake_minimum_required(VERSION 3.22)

project(Vantage VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# On macOS, build a universal binary (Intel + Apple Silicon) unless the
# caller has already set an explicit architecture list.
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS" FORCE)
endif()
if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment target" FORCE)
endif()

# ---------------------------------------------------------------------------
# JUCE
# ---------------------------------------------------------------------------
# Pulled via FetchContent so the ~1GB JUCE tree never lives in this repo.
# Pin to a specific tagged release for reproducible builds.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.15
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Keep the dependency footprint small: we ship no browser/webview UI and no
# networking, so skip the system libraries (WebKitGTK, libcurl) those pull
# in on Linux.
add_compile_definitions(
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)

set(VANTAGE_COMPANY_NAME "Clauded Audio")
set(VANTAGE_COMPANY_CODE "Cldd")
set(VANTAGE_VST3_CATEGORY "Instrument|Synth")

add_subdirectory(Source)
