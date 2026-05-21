# Template — replace [PluginName], [PluginCode], [ProductName], [Category] before use.
# [PluginName]  — CamelCase, no spaces (e.g. SimpleGain)
# [PluginCode]  — exactly 4 chars, unique across your plugins (e.g. SgAn)
# [ProductName] — display name with spaces (e.g. "Simple Gain")
# [Category]    — VST3 category string (e.g. "Fx", "Fx|Dynamics", "Instrument|Synth")

juce_add_plugin([PluginName]
    COMPANY_NAME             "YourStudio"
    PLUGIN_MANUFACTURER_CODE "YrSt"
    PLUGIN_CODE              "[PluginCode]"
    FORMATS                  VST3
    PRODUCT_NAME             "[ProductName]"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES          "[Category]"
)

target_sources([PluginName] PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions([PluginName] PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries([PluginName]
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_graphics
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header([PluginName])

add_custom_command(TARGET [PluginName]_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Assets"
    "$<TARGET_FILE_DIR:[PluginName]_VST3>/Assets"
    COMMENT "Copying Assets to VST3 bundle")
