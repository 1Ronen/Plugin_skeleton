cmake_minimum_required(VERSION 3.22)
project(PluginSkeleton VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# JUCE — fixed location on this machine
add_subdirectory("D:/HISE_Dev/JUCE" JUCE)

# Active plugin — change this line to switch plugins
add_subdirectory(plugins/SimpleGain)
