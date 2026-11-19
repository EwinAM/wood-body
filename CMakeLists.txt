cmake_minimum_required(VERSION 3.22)
project(WoodBodyOne VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(WoodBodyOne
    COMPANY_NAME "Ewin"
    PLUGIN_MANUFACTURER_CODE Ewin
    PLUGIN_CODE WBo1
    FORMATS VST3 AU
    AU_MAIN_TYPE kAudioUnitType_Effect
    BUNDLE_ID com.ewin.woodbodyone
    PRODUCT_NAME "Wood Body ONE"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx EQ)

juce_generate_juce_header(WoodBodyOne)

juce_add_binary_data(WoodBodyIRs SOURCES
    IR/Spruce.wav
    IR/Mahogany.wav
    IR/Rosewood.wav)

target_sources(WoodBodyOne PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(WoodBodyOne PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(WoodBodyOne
    PRIVATE
        WoodBodyIRs
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
