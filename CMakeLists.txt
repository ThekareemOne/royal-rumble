cmake_minimum_required(VERSION 3.22)
project(RoyalRumble VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "")
# Universal by default so release builds cover Apple Silicon and Intel.
# Override with -DCMAKE_OSX_ARCHITECTURES=arm64 for faster local builds.
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")

# Use a local JUCE checkout when one is present (fast local iteration);
# otherwise fetch it, so a fresh clone and CI both build without extra steps.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    message(STATUS "Royal Rumble: using local JUCE checkout")
    add_subdirectory(JUCE)
else()
    message(STATUS "Royal Rumble: fetching JUCE 8.0.4")
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# CI builds must not write into the user's plug-in folders.
option(RR_INSTALL_AFTER_BUILD "Copy the built plug-ins into the user plug-in folders" ON)

juce_add_plugin(RoyalRumble
    COMPANY_NAME              "Karim Hisham"
    DESCRIPTION               "Kick tail generator"
    BUNDLE_ID                 "com.karimhisham.royalrumble"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   ${RR_INSTALL_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE  Khsm
    PLUGIN_CODE               RyRm
    FORMATS                   AU VST3 Standalone
    PRODUCT_NAME              "Royal Rumble")

juce_add_binary_data(RoyalRumbleAssets SOURCES
    Fonts/ChakraPetch-Bold.ttf
    Fonts/ChakraPetch-SemiBold.ttf
    Fonts/ChakraPetch-Medium.ttf
    Fonts/JetBrainsMono-Medium.ttf
    Fonts/JetBrainsMono-Bold.ttf)

target_sources(RoyalRumble PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/RumbleEngine.cpp
    Source/EnvelopeDisplay.cpp
    Source/RoyalLookAndFeel.cpp
    Source/Fonts.cpp)

# JUCE 8 is dual-licensed AGPLv3 or commercial. This project takes the AGPLv3
# option, which is what permits JUCE_DISPLAY_SPLASH_SCREEN=0. See LICENSE.
target_compile_definitions(RoyalRumble PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(RoyalRumble PRIVATE
    RoyalRumbleAssets
    juce::juce_audio_utils
    juce::juce_dsp
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# Offline harness: renders a kick pattern through the engine and writes wavs.
juce_add_console_app(RoyalRumbleTest PRODUCT_NAME "RoyalRumbleTest")
target_sources(RoyalRumbleTest PRIVATE test/main.cpp Source/RumbleEngine.cpp)
target_compile_definitions(RoyalRumbleTest PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1)
target_link_libraries(RoyalRumbleTest PRIVATE
    juce::juce_dsp juce::juce_audio_formats
  PUBLIC juce::juce_recommended_config_flags)
