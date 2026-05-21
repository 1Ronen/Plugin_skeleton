cmake_minimum_required(VERSION 3.22)
project(PluginSkeleton VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# JUCE — configurable path (override with -DJUCE_PATH=... on cmake command line)
if(NOT DEFINED JUCE_PATH)
    set(JUCE_PATH "D:/HISE_Dev/JUCE")
endif()
add_subdirectory(${JUCE_PATH} JUCE)

# Active plugin — change this line to switch plugins
add_subdirectory(plugins/SimpleGain)
