cmake_minimum_required(VERSION 3.15)

project(GoldenSunVST VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_binary_data(AudioPluginData
    SOURCES
        Source/Resources/gsfont.ttf
    )

set(GS_SOURCES
    Source/GS/GSPresets.cpp
    Source/GS/GSPresets.h
    Source/GS/GSReverb.cpp
    Source/GS/GSReverb.h
    Source/GS/GSSynths.cpp
    Source/GS/GSSynths.h
)

set(PROCESSOR_SOURCES
    Source/Processor/CGBChannel.cpp
    Source/Processor/CGBChannel.h
    Source/Processor/CGBPatterns.cpp
    Source/Processor/CGBPatterns.h
    Source/Processor/ChannelState.cpp
    Source/Processor/ChannelState.h
    Source/Processor/Instrument.cpp
    Source/Processor/Instrument.h
    Source/Processor/Processor.cpp
    Source/Processor/Processor.h
    Source/Processor/Resampler.cpp
    Source/Processor/Resampler.h
    Source/Processor/ReverbEffect.cpp
    Source/Processor/ReverbEffect.h
    Source/Processor/RPNHandler.cpp
    Source/Processor/RPNHandler.h
    Source/Processor/SampleInstrument.cpp
    Source/Processor/SampleInstrument.h
    Source/Processor/Types.h
)

set(PRESETS_SOURCES
    Source/Presets/CGBSynthPresets.cpp
    Source/Presets/CGBSynthPresets.h
    Source/Presets/Presets.cpp
    Source/Presets/Presets.h
    Source/Presets/PresetsHandler.cpp
    Source/Presets/PresetsHandler.h
)

set(GUI_SOURCES
    Source/GUI/AboutWindow.cpp
    Source/GUI/AboutWindow.h
    Source/GUI/ControlsTab.cpp
    Source/GUI/ControlsTab.h
    Source/GUI/CustomLookAndFeel.cpp
    Source/GUI/CustomLookAndFeel.h
    Source/GUI/LevelMeter.cpp
    Source/GUI/LevelMeter.h
    Source/GUI/MainWindow.cpp
    Source/GUI/MainWindow.h
    Source/GUI/OverviewTab.cpp
    Source/GUI/OverviewTab.h
    Source/GUI/PresetCombo.cpp
    Source/GUI/PresetCombo.h
    Source/GUI/PropertyListener.h
    Source/GUI/SettingsWindow.cpp
    Source/GUI/SettingsWindow.h
    Source/GUI/SliderEx.cpp
    Source/GUI/SliderEx.h
)

set(EXTERNAL_SOURCES
    Source/External/tinysoundfont/tsf.h
)

set(JUCE_GENERATED_SOURCES
    Source/JuceGenerated/BinaryData.cpp
    Source/JuceGenerated/BinaryData.h
    Source/JuceGenerated/JuceHeader.h
)

set(ALL_SOURCES
    ${GS_SOURCES}
    ${PROCESSOR_SOURCES}
    ${PRESETS_SOURCES}
    ${GUI_SOURCES}
    ${EXTERNAL_SOURCES}
    ${JUCE_GENERATED_SOURCES}
)

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME "Vincent Dortel"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE vdor
    PLUGIN_CODE GSVS
    FORMATS VST3
    NEEDS_WEB_BROWSER FALSE
    PRODUCT_NAME "${PROJECT_NAME}")

target_sources(${PROJECT_NAME} PRIVATE ${ALL_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        Source
        Source/JuceGenerated
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_basics
        juce::juce_core
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_processors
        juce::juce_audio_plugin_client
        juce::juce_data_structures
        juce::juce_audio_formats
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

source_group("GS" FILES ${GS_SOURCES})
source_group("Processor" FILES ${PROCESSOR_SOURCES})
source_group("Presets" FILES ${PRESETS_SOURCES})
source_group("GUI" FILES ${GUI_SOURCES})
source_group("External" FILES ${EXTERNAL_SOURCES})
source_group("JuceGenerated" FILES ${JUCE_GENERATED_SOURCES})
